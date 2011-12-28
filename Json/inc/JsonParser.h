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
#ifndef JSONPARSER_H
#define JSONPARSER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <badesca.h>
#include "Stack.h"
#include "JsonTokener.h"

// CLASS DECLARATION
class MJsonContentHandler
	{
public:
	virtual void OnBeginObject() = 0;
	virtual void OnEndObject() = 0;
	
	virtual void OnBeginArray() = 0;
	virtual void OnEndArray() = 0;
	
	virtual void OnKey(const TDesC& aKey) = 0;
	virtual void OnValue(const TDesC& aValue) = 0;
	virtual void OnError(TInt aError) = 0;
	};

/**
 *  CJsonParser
 * 
 */
class CJsonParser : public CBase
	{
	enum TJsonParserState
		{
		ENone,
		EParseObject,
		EParseArray,
		EParseKey,
		EParseValue,
		EParseComa,
		EParseColon
		};
	
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CJsonParser();

	/**
	 * Two-phased constructor.
	 */
	static CJsonParser* NewL(MJsonContentHandler& aContentHandler);

	/**
	 * Two-phased constructor.
	 */
	static CJsonParser* NewLC(MJsonContentHandler& aContentHandler);

	void ParseL(const TDesC& aString);
	TPtrC LastString();
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CJsonParser(MJsonContentHandler& aContentHandler);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	MJsonContentHandler& iContentHandler;	
	CJsonTokener* iTokener;
	
	RStack<TJsonParserState> iParserStateStack;
	RBuf iLastString;	
	TPtrC iString;
	};

#endif // JSONPARSER_H
