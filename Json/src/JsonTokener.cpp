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

#include "JsonTokener.h"

CJsonTokener::CJsonTokener() :
iJsonString(KNullDesC),
iPosition(0)
	{
	// No implementation required
	}

CJsonTokener::~CJsonTokener()
	{
	}

void CJsonTokener::SetJsonString( const TDesC& aJsonString )
	{
	iJsonString.Set(aJsonString);
	iPosition = 0;
	}
	
void CJsonTokener::Back()
	{
	if( iPosition > 0 )
		--iPosition;
	}

TBool CJsonTokener::More() const
	{
	return (iPosition < iJsonString.Length());
	}

TText CJsonTokener::Next()
	{
	if( More() )
		{
		return iJsonString[ iPosition++ ]; 
		}
	return 0;
	}

TText CJsonTokener::Next(TText aChar)
	{
	TText n = Next();
	if(n != aChar)
		{
		User::Leave( KErrNotFound );
		}
	return n;
	}

TPtrC CJsonTokener::Next(TInt aLength)
	{
	if( iPosition + aLength >= iJsonString.Length() )
		User::Leave(KErrOverflow);
	
	return iJsonString.Mid( iPosition, aLength );
	}

TChar CJsonTokener::NextClean()
	{
	for (;;) 
		{
		TText c = Next();		
		if (c == '/') 
			{
			switch( Next() ) 
				{
				case '/':
					do 	{
						c = Next();
						} while (c != '\n' && c != '\r' && c != 0);
					break;
					
				case '*':
					for (;;) 
						{
						c = Next();
						if (c == 0)
							User::Leave( KErrNotSupported );
						if (c == '*') 
							{
							if (Next() == '/')
								break;
							Back();
							}					
						}
					break;
					
				default:
					Back();
					break;
				}
		}
		else if (c == '#') 
			{
			do {
				c = Next();
				} while (c != '\n' && c != '\r' && c != 0);
			} 
		else if (c == 0 || c > ' ')
			{
			return c;
			}
		}
	return 0;
	}

TPtrC CJsonTokener::NextString(TText aQuote)
	{
	TText c;
	TInt pos = iPosition, length = 0;
	
	for (;;) 
		{
		c = Next();		
		switch (c) 
			{
			case 0:
			case '\n':
			case '\r':
				User::Leave(KErrNotSupported);
				
			case '\\':
				Next();
				++length;
				break;
				
			default:
				if (c == aQuote)
					return iJsonString.Mid(pos, length);
			}
		++length;
		}
	return TPtrC( KNullDesC );
	}

TPtrC CJsonTokener::NextTo(TText aChar)
	{
	TInt pos = iPosition, length = 0;
	for (;;) 
		{
		TText c = Next();		
		
		if (c == aChar || c == 0 || c == '\n' || c == '\r') 
			{
			if (c != 0)
				Back();
			return iJsonString.Mid(pos, length);
			}
		++length;
		}
	}

TPtrC CJsonTokener::NextTo(const TDesC& aDelimiters)
	{
	TText c;
	TInt pos = iPosition, length = 0;
	for (;;) 
		{
		c = Next();		
		for(int i = 0 ; i < aDelimiters.Length(); ++i)
			{
			if (c == aDelimiters[i] || c == 0 ||
					c == '\n' || c == '\r') 
				{
				if (c != 0)
					Back();
				return iJsonString.Mid(pos, length);
				}
			}
		++length;
		}
	}
