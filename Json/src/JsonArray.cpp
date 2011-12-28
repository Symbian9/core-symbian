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

#include "JsonArray.h"
#include "JsonObject.h"
#include "JsonString.h"
#include "StringUtil.h"

CJsonArray::CJsonArray() 
	: MJsonValue( MJsonValue::EArray )
	{
	iElements.Reserve(8);
	}

CJsonArray::~CJsonArray()
	{
	for(int i = 0; i < iElements.Count(); ++i)
		if( iElements[i] )
			delete iElements[i];
	iElements.Close();
	}

void CJsonArray::AddIntL(TInt aValue)
	{
	CJsonString* string = new (ELeave) CJsonString;	
	string->SetIntL(aValue);
	iElements.AppendL( string );
	}

void CJsonArray::AddReal32L(TReal32 aValue)
	{
	CJsonString* string = new (ELeave) CJsonString;	
	string->SetReal32L(aValue);
	iElements.AppendL( string );
	}

void CJsonArray::AddReal64L(TReal64 aValue)
	{
	CJsonString* string = new (ELeave) CJsonString;	
	string->SetReal64L(aValue);
	iElements.AppendL( string );
	}

void CJsonArray::AddBoolL(TBool aValue)
	{
	CJsonString* string = new (ELeave) CJsonString;	
	string->SetBoolL(aValue);
	iElements.AppendL( string );
	}

void CJsonArray::AddL(const TDesC& aValue)
	{
	CJsonString* string = new (ELeave) CJsonString;	
	string->SetStringL(aValue);
	iElements.AppendL( string );
	}

void CJsonArray::AddL(CJsonObject* aValue)
	{
	iElements.AppendL( aValue );
	}

void CJsonArray::AddL(CJsonArray* aValue)
	{
	iElements.AppendL( aValue );
	}

void CJsonArray::GetValue(TInt aIndex, MJsonValue*& aValue)
	{
	ASSERT( aIndex >= 0 );
	aValue = iElements[aIndex];
	}

void CJsonArray::GetObject(TInt aIndex, CJsonObject*& aObject) const
	{
	ASSERT( aIndex >= 0 );
	aObject = dynamic_cast<CJsonObject*>( iElements[aIndex] );
	}

void CJsonArray::GetArray(TInt aIndex, CJsonArray*& aArray) const
	{
	ASSERT( aIndex >= 0 );
	aArray = dynamic_cast<CJsonArray*>( iElements[aIndex] );
	}

void CJsonArray::GetStringL(TInt aIndex, TDes& aString) const
	{
	ASSERT( aIndex >= 0 );
	CJsonString* string;
	if( string = dynamic_cast<CJsonString*>( iElements[aIndex] ) )
		aString.Copy( string->String() );
	}

TInt CJsonArray::GetStringLength(TInt aIndex) const
	{
	ASSERT( aIndex >= 0 );
	CJsonString* string;
	if( string = dynamic_cast<CJsonString*>( iElements[aIndex] ) )
		return string->String().Length();
	return 0;
	}

void CJsonArray::GetIntL(TInt aIndex, TInt& aInt) const
	{
	ASSERT( aIndex >= 0 );
	CJsonString* string;
	if( string = dynamic_cast<CJsonString*>( iElements[aIndex] ) )
		aInt = string->IntL();
	}

void CJsonArray::GetBoolL(TInt aIndex, TBool& aBool) const
	{
	ASSERT( aIndex >= 0 );
	CJsonString* string;
	if( string = dynamic_cast<CJsonString*>( iElements[aIndex] ) )
		aBool = string->BoolL();
	}

void CJsonArray::GetReal32L(TInt aIndex, TReal32& aReal) const
 	{
 	ASSERT( aIndex >= 0 );
 	CJsonString* string;
 	if( string = dynamic_cast<CJsonString*>( iElements[aIndex] ) )
 		aReal = string->Real32L();
 	}
 
 void CJsonArray::GetReal64L(TInt aIndex, TReal64& aReal) const
 	{
 	ASSERT( aIndex >= 0 );
 	CJsonString* string;
	if( string = dynamic_cast<CJsonString*>( iElements[aIndex] ) )
 		aReal = string->Real64L();
 	}

// [element1,element2,...,elementn]
void CJsonArray::ToStringL(RBuf& aOutputString) const
	{
	RBufAppendL(aOutputString, '[' );	
	if( iElements.Count() )
		{
		for(int i = 0; i < iElements.Count() - 1; ++i)
			{
			if(iElements[i])
				{			
				iElements[i]->ToStringL( aOutputString );
				RBufAppendL(aOutputString, ',' );
				}
				else
				{
				RBufAppendL(aOutputString, _L("null,") );
				}
			}
		
		if(iElements[iElements.Count() - 1])
			{
			iElements[iElements.Count() - 1]->ToStringL( aOutputString );
			}
		else
			{
			RBufAppendL(aOutputString, _L("null") );
			}
		}		
	RBufAppendL(aOutputString, ']' );
	}
