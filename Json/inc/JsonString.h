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

#ifndef JSONSTRING_H
#define JSONSTRING_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "JsonValue.h"

// CLASS DECLARATION

/**
 *  CJsonString
 */
class CJsonString : public CBase, public MJsonValue
	{
	enum TJsonStringDataType
		{
		EUndefined,
		EString,
		EBool,
		EInt,
		EInt64,
		EReal
		};
	
public:
	CJsonString();
	virtual ~CJsonString();
	
	void SetStringL(const TDesC& aValue);
	void SetIntL(TInt aValue);
	void SetInt64L(TInt64 aValue);
	void SetBoolL(TBool aValue);	
	void SetReal32L(TReal32 aValue);
	void SetReal64L(TReal64 aValue);
	
	TPtrC String() const;
	TReal32 RealL() const;
	TBool BoolL() const;	
	TInt IntL() const;
	TInt64 Int64L() const;
	TReal32 Real32L() const;
	TReal64 Real64L() const;
	
	TJsonStringDataType DataType() const { return iDataType; }	
	
	// from MJsonValue
	virtual void ToStringL(RBuf& aOutputString) const;
	
private:
	TJsonStringDataType iDataType;
	RBuf iData;
	};

#endif // JSONSTRING_H
