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

#ifndef JSONOBJECT_H
#define JSONOBJECT_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "JsonPair.h"

// CLASS DECLARATION
class CJsonArray;

/**
 *  CJsonObject
 * 
 */
class CJsonObject : public CBase, public MJsonValue
	{
public:
	CJsonObject();
	virtual ~CJsonObject();

	void AddIntL(const TDesC& aKey, TInt aValue);
	void AddInt64L(const TDesC& aKey, TInt64 aValue);
	void AddReal32L(const TDesC& aKey, TReal32 aValue);
	void AddReal64L(const TDesC& aKey, TReal64 aValue);
	void AddBoolL(const TDesC& aKey, TBool aValue);
		
	void AddL(const TDesC& aKey, const TDesC& aValue);
	// transfer ownership of aValue to object
	void AddL(const TDesC& aKey, CJsonObject* aValue);
	void AddL(const TDesC& aKey, CJsonArray* aValue);
	
	TInt Find(const TDesC& aKey) const;
	void GetValue(TInt aIndex, MJsonValue*& aValue) const;	
	
	void GetObjectL(const TDesC& aKey, CJsonObject*& aObject) const;
	void GetArrayL(const TDesC& aKey, CJsonArray*& aArray) const;
	void GetStringL(const TDesC& aKey, TDes& aString) const;
	void GetIntL(const TDesC& aKey, TInt& aInt) const;
	void GetInt64L(const TDesC& aKey, TInt64& aInt) const;
	void GetBoolL(const TDesC& aKey, TBool& aBool) const;
	void GetReal32L(const TDesC& aKey, TReal32& aReal) const;
	void GetReal64L(const TDesC& aKey, TReal64& aReal) const;
	
	inline TInt Count() const { return iMembers.Count(); }	
	
	// from MJsonValue
	virtual void ToStringL(RBuf& aOutputString) const;
	
private:
	template <typename T> T* FindEntryAndCast(const TDesC& aKey) const
		{
		TInt index = Find( aKey );
		MJsonValue* value = NULL;
		if( index >= 0 )		
			GetValue(index, value);
		return dynamic_cast<T*>( value );
		}
	
private:	
	RArray<CJsonPair*> iMembers;
	};

#endif // JSONOBJECT_H
