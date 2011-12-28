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

#include "JsonString.h"
#include "JsonUtil.h"
#include "StringUtil.h"

_LIT(KFalseValue, "false");
_LIT(KTrueValue, "true");

CJsonString::CJsonString() : 
MJsonValue( MJsonValue::EString ),
iDataType( EUndefined )
	{
	// No implementation required
	iData.Create(256);
	}

CJsonString::~CJsonString()
	{
	iData.Close();
	}

void CJsonString::SetStringL(const TDesC& aValue)
	{
	iDataType = EString;
	RBufCopyL(iData, aValue );
	}

void CJsonString::SetIntL(TInt aValue)
	{
	iDataType = EInt;
	iData.Format( _L("%d"), aValue);
	}

void CJsonString::SetInt64L(TInt64 aValue)
	{
	iDataType = EInt64;
	iData.Format( _L("%Ld"), aValue);
	}

void CJsonString::SetReal32L(TReal32 aValue)
	{
	iDataType = EReal;
	TRealFormat format;
	format.iType = KRealFormatFixed | KDoNotUseTriads;//not sure these are necessary
	format.iPoint = '.';//this is required so that when the user is working in a different local (e.g. French) he won't get ',' instead of '.'
	iData.Num(aValue, format);
	}

void CJsonString::SetReal64L(TReal64 aValue)
	{
	iDataType = EReal;
	TRealFormat format;
	format.iType = KRealFormatFixed | KDoNotUseTriads;//not sure these are necessary
	format.iPoint = '.';//this is required so that when the user is working in a different local (e.g. French) he won't get ',' instead of '.'
	iData.Num(aValue, format);
	}

void CJsonString::SetBoolL(TBool aValue)
	{
	iDataType = EBool;
	if( aValue )
		RBufCopyL(iData, KTrueValue);
	else
		RBufCopyL(iData, KFalseValue);
	}

TPtrC CJsonString::String() const
	{
	return iData;
	}

TReal32 CJsonString::Real32L() const
	{
	TLex lex( iData );
	TInt error;
	TReal32 real;
	
	if( (error = lex.Val( real, '.' )) != KErrNone )//the '.' is required so that when the user is working in a different local (e.g. French) he won't get ',' instead of '.'
		User::Leave( error );
	return real;
	}

TReal64 CJsonString::Real64L() const
	{
	TLex lex( iData );
	TInt error;
	TReal64 real;
  
	if( (error = lex.Val( real, '.' )) != KErrNone )//the '.' is required so that when the user is working in a different local (e.g. French) he won't get ',' instead of '.'
		User::Leave( error );
	return real;
	}

TBool CJsonString::BoolL() const
	{
	if( iData.Compare( KFalseValue ) == 0 )
		return EFalse;
	else if( iData.Compare( KTrueValue ) == 0 )
		return ETrue;
	else
		User::Leave( KErrNotSupported );
	return EFalse;
	}

TInt CJsonString::IntL() const
	{
	TLex lex( iData );
	TInt int_, error;
	
	if( (error = lex.Val( int_ )) != KErrNone )
		User::Leave( error );
	return int_;
	}

TInt64 CJsonString::Int64L() const
	{
	TLex lex( iData );
	TInt error;
	TInt64 int_;
	
	if( (error = lex.Val( int_ )) != KErrNone )
		User::Leave( error );
	return int_;
	}

void CJsonString::ToStringL(RBuf& aOutputString) const
	{
	switch( iDataType )
		{
		case EUndefined:
		case EString:
			ConvertUnicodeToJsonFormatL( iData, aOutputString );
			break;
			
		case EInt:
		case EInt64:
		case EReal:
		case EBool:
			RBufAppendL(aOutputString, iData);
			break;
		}
	}
