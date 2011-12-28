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

#include "JsonPair.h"
#include "JsonUtil.h"
#include "JsonString.h"
#include "JsonObject.h"
#include "JsonArray.h"
#include "StringUtil.h"

_LIT(KNullValue, "null");

CJsonPair::CJsonPair(const TDesC& aKey, MJsonValue::TJsonValueType aType)
	: iKey(aKey)
	{
	switch( aType )
		{
		case MJsonValue::EString:
			iValue = new (ELeave) CJsonString();
			break;
		case MJsonValue::EObject:
			iValue = new (ELeave) CJsonObject();
			break;
		case MJsonValue::EArray:
			iValue = new (ELeave) CJsonArray();
			break;
		}	
	}

CJsonPair::CJsonPair(const TDesC& aKey, MJsonValue* aValue)
	: iKey(aKey), iValue(aValue)
	{
	
	}

CJsonPair::~CJsonPair() 	
	{
	if( iValue )
		delete iValue;
	}

void CJsonPair::SetKeyL(const TDesC& aKey)
	{
	if(iKey.MaxLength() < aKey.Length())
		User::Leave(KErrOverflow);
	iKey.Copy(aKey);
	}

void CJsonPair::ToStringL(RBuf& aOutputString) const
	{
	ConvertUnicodeToJsonFormatL(iKey, aOutputString);
	RBufAppendL(aOutputString, ':' );
	if( iValue )
		iValue->ToStringL( aOutputString );
	else
		RBufAppendL(aOutputString, KNullValue );
	}
