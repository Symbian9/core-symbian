/*
Copyright (c) 2009, Piotr Wach, Polidea
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Polidea nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY PIOTR WACH, POLIDEA ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL PIOTR WACH, POLIDEA BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "JsonParser.h"
#include "JsonUtil.h"

CJsonParser::CJsonParser(MJsonContentHandler& aContentHandler)
	: iContentHandler(aContentHandler), iString(0, 0)
	{
	// No implementation required
	}

CJsonParser::~CJsonParser()
	{
	iParserStateStack.Close();
	
	if( iTokener )
		delete iTokener;
	
	iLastString.Close();
	}

CJsonParser* CJsonParser::NewLC(MJsonContentHandler& aContentHandler)
	{
	CJsonParser* self = new (ELeave) CJsonParser(aContentHandler);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CJsonParser* CJsonParser::NewL(MJsonContentHandler& aContentHandler)
	{
	CJsonParser* self = CJsonParser::NewLC(aContentHandler);
	CleanupStack::Pop(); // self;
	return self;
	}

void CJsonParser::ConstructL()
	{
	iLastString.Create(256);
	iTokener = new (ELeave) CJsonTokener;
	}

void CJsonParser::ParseL(const TDesC& aString)
	{	
	iTokener->SetJsonString( aString );
	
	TText c;
	for(;;)
		{
		c = iTokener->NextClean();
		switch( c )
			{
			case 0:
				return;
			
			case '{':				
				iParserStateStack.PushL( EParseObject );
				iContentHandler.OnBeginObject();
				break;
				
			case '}':
				if( iParserStateStack.TopL() == EParseValue )
					iParserStateStack.Pop();
				if( iParserStateStack.TopL() != EParseObject )
					{
					iContentHandler.OnError(KErrNotSupported);
					return;
					}
				iParserStateStack.Pop();
				iContentHandler.OnEndObject();
				break;
				
			case '[':				
				iParserStateStack.PushL( EParseArray );
				iContentHandler.OnBeginArray();
				break;
				
			case ']':
				if( iParserStateStack.TopL() == EParseValue )
					iParserStateStack.Pop();
				if( iParserStateStack.TopL() != EParseArray )
					{
					iContentHandler.OnError(KErrNotSupported);
					return;
					}
				iParserStateStack.Pop();
				iContentHandler.OnEndArray();
				break;
				
			case ':':
				iParserStateStack.PushL( EParseValue );
				break;
				
			case ',':
				if( iParserStateStack.TopL() == EParseValue )
					iParserStateStack.Pop();
				break;
				
			case '\"':
				{
				TPtrC string = iTokener->NextString( '\"' );
				iLastString.Zero();
				ConvertJsonFormatToUnicodeL(string, iLastString);
				
				if( iParserStateStack.TopL() != EParseArray 
						&& iParserStateStack.TopL() != EParseValue)
					{
					iContentHandler.OnKey( iLastString );
					}
				else
					{
					if( iParserStateStack.TopL() == EParseValue )
						iParserStateStack.Pop();
					iContentHandler.OnValue( iLastString );
					}
				}
				break;			

			default:
				{
				iTokener->Back();
				TPtrC string = iTokener->NextTo( _L(",}]") );
				iLastString.Zero();
				ConvertJsonFormatToUnicodeL(string, iLastString);
				
				iContentHandler.OnValue( iLastString );
				if( iParserStateStack.TopL() == EParseValue )
					iParserStateStack.Pop();
				}
				break;
			}
		}
	}

TPtrC CJsonParser::LastString()
	{
	return iLastString;
	}

