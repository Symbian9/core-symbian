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

#include "JsonUtil.h"
#include "StringUtil.h"

void ConvertUnicodeToJsonFormatL(const TDesC& aString, RBuf& aOutput)
	{
	RBufAppendL(aOutput, '\"' );
	for(TInt i = 0; i < aString.Length(); ++i)
		{
		switch( aString[i] )
			{
			case '\\':
				RBufAppendL(aOutput, _L("\\\\") );
				break;
				
			case '/':
				RBufAppendL(aOutput, _L("\\/") );
				break;
						
			case '"':
				RBufAppendL(aOutput, _L("\\\"") );
				break;
			
			case '\t':
				RBufAppendL(aOutput, _L("\\t") );
				break;
			case '\f':
				RBufAppendL(aOutput, _L("\\f") );
				break;
			case '\b':
				RBufAppendL(aOutput, _L("\\b") );
				break;
			case '\n':
				RBufAppendL(aOutput, _L("\\n") );
				break;
			case '\r':
				RBufAppendL(aOutput, _L("\\r") );
				break;
			
			default:
				{
				if( aString[i] < 256 )
					RBufAppendL(aOutput, TChar(aString[i]) );
				else
					{
					TBuf<32> format;
					format.Format(_L("\\u%04x"), aString[i]);
					RBufAppendL(aOutput, format);
					}
				}
			}
		}
	RBufAppendL(aOutput, '\"' );
	}

TUint16 NextCharacter(const TDesC& aString, TInt aIndex)
	{
	return (aIndex + 1 < aString.Length()) ? aString[ aIndex+1 ] : aString[ aIndex ];
	}

TUint ConvertHexToUnicodeCharL(const TDesC& aString, TInt aPos, TInt aLength, TUint &aCharacter)
	{
	if( aPos + aLength > aString.Length() )
		User::Leave(KErrOverflow);
	
	TLex intParser( aString.Mid(aPos, aLength) );
	User::LeaveIfError( intParser.Val( aCharacter, EHex ) );	
	return aLength;
	}

void ConvertJsonFormatToUnicodeL(const TDesC& aString, RBuf& aOutput)
	{
	for(TInt i = 0; i < aString.Length(); ++i)
		{
		switch( aString[i] )
			{
			case '\\':
				{
				switch ( NextCharacter(aString, i) )
					{
					case 'u':
						{
						TUint character, offset;
						offset = ConvertHexToUnicodeCharL(aString, i + 2, 4, character);
						RBufAppendL(aOutput, character );						
						i += offset + 1;
						}
						break;
					
					case '\\':
						RBufAppendL(aOutput, '\\' );
						++i;
						break;	
						
					case '/':
						RBufAppendL(aOutput, '/' );
						++i;
						break;	
						
					case '"':
						RBufAppendL(aOutput, '"' );
						++i;
						break;	
						
					case 't':
						RBufAppendL(aOutput, '\t' );
						++i;
						break;						
					case 'f':
						RBufAppendL(aOutput, '\f' );
						++i;
						break;
					case 'b':
						RBufAppendL(aOutput, '\b' );
						++i;
						break;
					case 'n':
						RBufAppendL(aOutput, '\n' );
						++i;
						break;
					case 'r':
						RBufAppendL(aOutput, '\r' );
						++i;
						break;
						
					default:
						RBufAppendL(aOutput, aString[i] );
						++i;
						break;
					}	
				}
				break;
				
			// its only text formating so dont append it to final string
			case '\n':
			case '\r':
			case '\f':
			case '\t':
			case '\b':
				break;
				
			default:
				RBufAppendL(aOutput, aString[i] );
				break;
			}
		}
	}
