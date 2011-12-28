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

#include "JsonBuilder.h"
#include <utf.h> 
#include "StringUtil.h"

CJsonBuilder::CJsonBuilder() :
iDocumentObject( NULL )
	{
	// No implementation required
	}

CJsonBuilder::~CJsonBuilder()
	{
	if(iJsonParser)
		delete iJsonParser;
	
	iObjectStack.Close();
	iLastKey.Close();
	}

CJsonBuilder* CJsonBuilder::NewLC()
	{
	CJsonBuilder* self = new (ELeave) CJsonBuilder();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CJsonBuilder* CJsonBuilder::NewL()
	{
	CJsonBuilder* self = CJsonBuilder::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

void CJsonBuilder::ConstructL()
	{
	iLastKey.Create(256);
	iJsonParser = CJsonParser::NewL( *this );
	}

void CJsonBuilder::BuildFromJsonStringL(const TDesC& aResponseMessage)
	{
	iJsonParser->ParseL( aResponseMessage );
	}

void CJsonBuilder::GetDocumentObject(CJsonObject*& aObject)
	{
	aObject = dynamic_cast<CJsonObject*>( iDocumentObject );
	iDocumentObject = NULL;
	}

void CJsonBuilder::GetDocumentArray(CJsonArray*& aArray)
	{
	aArray = dynamic_cast<CJsonArray*>( iDocumentObject );
	iDocumentObject = NULL;
	}

void CJsonBuilder::OnBeginObject()
	{
	CJsonObject* object = new (ELeave) CJsonObject();
	
	AddToParentL( iJsonParser->LastString(), object );
	iObjectStack.PushL( object );
	}

void CJsonBuilder::OnEndObject()
	{
	if( iObjectStack.Count() == 1 )
		iDocumentObject = iObjectStack.TopL();
	
	iObjectStack.Pop();
	}

void CJsonBuilder::OnBeginArray()
	{
	CJsonArray* object = new (ELeave) CJsonArray();
		
	AddToParentL( iJsonParser->LastString(), object );
	iObjectStack.PushL( object );
	}

void CJsonBuilder::OnEndArray()
	{
	if( iObjectStack.Count() == 1 )
		iDocumentObject = iObjectStack.TopL();
	
	iObjectStack.Pop();
	}

void CJsonBuilder::OnKey(const TDesC& aKey)
	{
	// no implementation
	RBufCopyL(iLastKey, aKey);
	}

void CJsonBuilder::OnValue(const TDesC& aValue)
	{
	AddToParentL( iLastKey, aValue );
	iLastKey.Zero();
	}

void CJsonBuilder::OnError(TInt aError)
	{
	
	}
