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

#ifndef JSONTOKENER_H
#define JSONTOKENER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

// CLASS DECLARATION

/**
 *  CJsonTokener
 * 
 */
class CJsonTokener : public CBase
	{
public:
	// Constructors and destructor
	/**
	 * Constructor for performing 1st stage construction
	 */
	CJsonTokener();

	/**
	 * Destructor.
	 */
	~CJsonTokener();

	/**
	 * Sets string to process tokenization on.
	 */
	void SetJsonString( const TDesC& aJsonString );
	
	
	 /**
	 * Back up one character. This provides a sort of lookahead capability,
	 * so that you can test for a digit or letter before attempting to parse
	 * the next number or identifier.
	 */
	void Back();
	
	/**
	 * Determine if the source string still contains characters that next()
	 * can consume.
	 * @return true if not yet at the end of the source.
	 */
	TBool More() const;
	
	/**
	 * Get the next character in the source string.
	 *
	 * @return The next character, or 0 if past the end of the source string.
	 */
	TText Next();
	
	/**
	 * Consume the next character, and check that it matches a specified
	 * character.
	 * @param aChar The character to match.
	 * @return 		The character.
	 */
	TText Next(TText aChar);
	
	 /**
	 * Get the next aLength characters.
	 *
	 * @param aLength     The number of characters to take.
	 * @return      	  A string of n characters.
	 *   Substring bounds error if there are not
	 *   n characters remaining in the source string.
	 */
	TPtrC Next(TInt aLength);
	
	/**
	 * Get the next char in the string, skipping whitespace
	 * and comments (slashslash, slashstar, and hash).
	 * @return  A character, or 0 if there are no more characters.
	 */
	TChar NextClean();
	
	/**
	 * Return the characters up to the next close quote character.
	 * Backslash processing is done. The formal JSON format does not
	 * allow strings in single quotes, but an implementation is allowed to
	 * accept them.
	 * @param aQuote The quoting character, either
	 *      <code>"</code>&nbsp;<small>(double quote)</small> or
	 *      <code>'</code>&nbsp;<small>(single quote)</small>.
	 * @return      A String.
	 */
	TPtrC NextString(TText aQuote);
	
	/**
	 * Get the text up but not including the specified character or the
	 * end of line, whichever comes first.
	 * @param  aChar A delimiter character.
	 * @return  	 A string.
	 */
	TPtrC NextTo(TText aChar);
	
	/**
	 * Get the text up but not including one of the specified delimeter
	 * characters or the end of line, whichever comes first.
	 * @param aDelimiters A set of delimiter characters.
	 * @return A string.
	 */
	TPtrC NextTo(const TDesC& aDelimiters);
	
private:
	TPtrC iJsonString;
	TInt iPosition;
	};

#endif // JSONTOKENER_H
