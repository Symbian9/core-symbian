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

#include "JsonObject.h"
#include "JsonString.h"
#include "JsonArray.h"
#include "StringUtil.h"

TBool IsEqual(const CJsonPair &pair1, const CJsonPair &pair2)
	{
	return (&pair1 == &pair2);
	}

TUint32 HashFunction(const CJsonPair &pair)
	{
	TUint32 hash = 0;	
	for(TInt i = 0; i < pair.Key().Length(); ++i)
		hash = pair.Key()[i] + (hash << 6) + (hash << 16) - hash;
	return hash;
	}

CJsonObject::CJsonObject() 
	: MJsonValue( MJsonValue::EObject )
	{
	// No implementation required	
	}

CJsonObject::~CJsonObject()
	{
	for(int i = 0; i < iMembers.Count(); ++i)
		if( iMembers[i] )
			delete iMembers[i];
	iMembers.Close();
	}

void CJsonObject::AddIntL(const TDesC& aKey, TInt aValue)
	{
	CJsonPair* pair = new (ELeave) CJsonPair(aKey, MJsonValue::EString);
	((CJsonString*)pair->Value())->SetIntL(aValue);
	iMembers.AppendL( pair );
	}

void CJsonObject::AddInt64L(const TDesC& aKey, TInt64 aValue)
	{
	CJsonPair* pair = new (ELeave) CJsonPair(aKey, MJsonValue::EString);
	((CJsonString*)pair->Value())->SetInt64L(aValue);
	iMembers.AppendL( pair );
	}

void CJsonObject::AddReal32L(const TDesC& aKey, TReal32 aValue)
	{
	CJsonPair* pair = new (ELeave) CJsonPair(aKey, MJsonValue::EString);
	((CJsonString*)pair->Value())->SetReal32L(aValue);
	iMembers.AppendL( pair );
	}

void CJsonObject::AddReal64L(const TDesC& aKey, TReal64 aValue)
 	{
 	CJsonPair* pair = new (ELeave) CJsonPair(aKey, MJsonValue::EString);
 	((CJsonString*)pair->Value())->SetReal64L(aValue);
 	iMembers.AppendL( pair );
 	}

void CJsonObject::AddBoolL(const TDesC& aKey, TBool aValue)
	{
	CJsonPair* pair = new (ELeave) CJsonPair(aKey, MJsonValue::EString);
	((CJsonString*)pair->Value())->SetBoolL(aValue);
	iMembers.AppendL( pair );
	}

void CJsonObject::AddL(const TDesC& aKey, const TDesC& aValue)
	{
	CJsonPair* pair = new (ELeave) CJsonPair(aKey, MJsonValue::EString);
	((CJsonString*)pair->Value())->SetStringL(aValue);
	iMembers.AppendL( pair );
	}

void CJsonObject::AddL(const TDesC& aKey, CJsonObject* aValue)
	{
	CJsonPair* pair = new (ELeave) CJsonPair(aKey, aValue);
	iMembers.AppendL( pair );
	}

void CJsonObject::AddL(const TDesC& aKey, CJsonArray* aValue)
	{
	CJsonPair* pair = new (ELeave) CJsonPair(aKey, aValue);
	iMembers.AppendL( pair );
	}

TInt CJsonObject::Find(const TDesC& aKey) const
	{
	for( TInt i = 0; i < iMembers.Count(); i++ )
		{
		// object found
		if( iMembers[i]->Key().Compare( aKey ) == 0 )
			return i;
		}
	return KErrNotFound;
	}

void CJsonObject::GetValue(TInt aIndex, MJsonValue*& aValue) const
	{
	if( 0 <= aIndex && aIndex < iMembers.Count() )
		{
		aValue = iMembers[aIndex]->Value();
		}
	else aValue = NULL;
	}

void CJsonObject::GetObjectL(const TDesC& aKey, CJsonObject*& aObject) const
	{
	aObject = FindEntryAndCast<CJsonObject>( aKey );
	}

void CJsonObject::GetArrayL(const TDesC& aKey, CJsonArray*& aArray) const
	{
	aArray = FindEntryAndCast<CJsonArray>( aKey );
	}

void CJsonObject::GetStringL(const TDesC& aKey, TDes& aString) const
	{
	CJsonString* jsonString;	
	if( jsonString = FindEntryAndCast<CJsonString>( aKey ) )
		aString.Copy( jsonString->String() );
	}

void CJsonObject::GetIntL(const TDesC& aKey, TInt& aInt) const
	{
	CJsonString* jsonString;	
	if( jsonString = FindEntryAndCast<CJsonString>( aKey ) )
		aInt = jsonString->IntL();
	else 
		aInt = 0;		
	}

void CJsonObject::GetInt64L(const TDesC& aKey, TInt64& aInt) const
	{
	CJsonString* jsonString;	
	if( jsonString = FindEntryAndCast<CJsonString>( aKey ) )
		aInt = jsonString->Int64L();
	else 
		aInt = 0;
	}

void CJsonObject::GetBoolL(const TDesC& aKey, TBool& aBool) const
	{
	CJsonString* jsonString;	
	if( jsonString = FindEntryAndCast<CJsonString>( aKey ) )
		aBool = jsonString->BoolL();
	else 
		aBool = 0;		
	}

void CJsonObject::GetReal32L(const TDesC& aKey, TReal32& aReal) const
 	{
 	CJsonString* jsonString;  
 	if( jsonString = FindEntryAndCast<CJsonString>( aKey ) )
 		aReal = jsonString->Real32L();
 	else 
 		aReal = 0;   
 	}
 
void CJsonObject::GetReal64L(const TDesC& aKey, TReal64& aReal) const
 	{
 	CJsonString* jsonString;  
 	if( jsonString = FindEntryAndCast<CJsonString>( aKey ) )
 		aReal = jsonString->Real64L();
 	else 
 		aReal = 0;
 	}

void CJsonObject::ToStringL(RBuf& aOutputString) const
	{
	RBufAppendL(aOutputString, '{' );

	for(int i = 0; i < iMembers.Count() - 1; ++i)
		{
		if( iMembers[i] )
			iMembers[i]->ToStringL( aOutputString );
		RBufAppendL(aOutputString, ',' );
		}
	if(iMembers[iMembers.Count() - 1])
		iMembers[iMembers.Count() - 1]->ToStringL( aOutputString );
	
	RBufAppendL(aOutputString, '}' );
	}
