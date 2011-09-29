/*
 * StateNewConfFeedback.h
 *
 *  Created on: 23/set/2011
 *      Author: Giovanna
 */

#ifndef STATENEWCONFFEEDBACK_H_
#define STATENEWCONFFEEDBACK_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateNewConfFeedback
 * 
 */
class CStateNewConfFeedback : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateNewConfFeedback();

	/**
	 * Two-phased constructor.
	 */
	static CStateNewConfFeedback* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateNewConfFeedback* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);   
	virtual void ProcessDataL(const TDesC8& aData);
	
	/**
	 * Sets the error coming from previous newconf state
	 */
	void SetError(TInt aError);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateNewConfFeedback(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	
	TBuf8<16>	iSignKey;
	HBufC8*		iRequestData;
	HBufC8*		iResponseData; //response data
		
	TInt		iError;
	};


#endif /* STATENEWCONFFEEDBACK_H_ */
