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

#ifndef JSONARRAY_H
#define JSONARRAY_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "JsonValue.h"

// CLASS DECLARATION
class CJsonObject;

/**
 *  CJsonArray
 * 
 */
class CJsonArray : public CBase, public MJsonValue
	{
public:
	CJsonArray();
	virtual ~CJsonArray();
	
	void AddIntL(TInt aValue);
	
	void AddReal32L(TReal32 aValue);
	void AddReal64L(TReal64 aValue);
	
	void AddBoolL(TBool aValue);
	
	void AddL(const TDesC& aValue);
	
	// transfer ownership of aValue to this
	void AddL(CJsonObject* aValue);
	void AddL(CJsonArray* aValue);
	
	void GetValue(TInt aIndex, MJsonValue*& aValue);
	
	void GetObject(TInt aIndex, CJsonObject*& aObject) const;
	void GetArray(TInt aIndex, CJsonArray*& aArray) const;
	void GetStringL(TInt aIndex, TDes& aString) const;
	void GetIntL(TInt aIndex, TInt& aInt) const;
	void GetBoolL(TInt aIndex, TBool& aBool) const;
	
	void GetReal32L(TInt aIndex, TReal32& aReal) const;
	void GetReal64L(TInt aIndex, TReal64& aReal) const;
	
	TInt GetStringLength(TInt aIndex) const;
	
	inline TInt Count() const { return iElements.Count(); }

	// from MJsonValue
	virtual void ToStringL(RBuf& aOutputString) const;
	
private:
	RArray<MJsonValue*> iElements;
	};

#endif // JSONARRAY_H
