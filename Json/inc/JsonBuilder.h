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

#ifndef JSONOBJECTBUILDER_H
#define JSONOBJECTBUILDER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "Stack.h"
#include "JsonParser.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonArray.h"
#include "JsonString.h"

// CLASS DECLARATION

/**
 *  CJsonObjectBuilder
 * 
 */
class CJsonBuilder : public CBase, public MJsonContentHandler
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CJsonBuilder();

	/**
	 * Two-phased constructor.
	 */
	static CJsonBuilder* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CJsonBuilder* NewLC();

	void BuildFromJsonStringL(const TDesC& aResponseMessage);
	
	// transfer ownership to the caller
	void GetDocumentObject(CJsonObject*& aObject);
	void GetDocumentArray(CJsonArray*& aArray);

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CJsonBuilder();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
	template <typename T> void AddToParentL( const TDesC& aKey, T& aObject )
		{
		if( !iObjectStack.Count() )
			return;
		MJsonValue* value = iObjectStack.TopL();
	
		switch( value->Type() )
			{
			case MJsonValue::EArray:
				((CJsonArray*)value)->AddL( aObject );
				break;
			case MJsonValue::EObject:
				((CJsonObject*)value)->AddL( aKey, aObject );
				break;
			default:
				break;
			}
		}
	
	// from MJsonContentHandler
	void OnBeginObject();
	void OnEndObject();
	
	void OnBeginArray();
	void OnEndArray();
	
	void OnKey(const TDesC& aKey);
	void OnValue(const TDesC& aValue);
	void OnError(TInt aError);
	
private:
	CJsonParser* iJsonParser;
	MJsonValue* iDocumentObject;
	
	RStack< MJsonValue* > iObjectStack;
	RBuf iLastKey;
	};

#endif // JSONOBJECTBUILDER_H
