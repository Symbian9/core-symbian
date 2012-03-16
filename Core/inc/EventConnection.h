/*
 * EventConnection.h
 *
 *  Created on: 08/ott/2010
 *      Author: Giovanna
 */

#ifndef EVENTCONNECTION_H_
#define EVENTCONNECTION_H_

#include "AbstractEvent.h"
#include <rconnmon.h>
#include <HT\TimeOutTimer.h>

typedef struct TConnectionStruct 
	{
	TInt iExitAction;	// action triggered when connection closed
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TConnectionStruct()
		{
		iExitAction = -1;
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		}
	}TConnectionStruct;

/**
 *  CEventConnection
 * 
 */
class CEventConnection : public CAbstractEvent, public MConnectionMonitorObserver, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventConnection();

	/**
	 * Two-phased constructor.
	 */
	static CEventConnection* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventConnection* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();
	/**
	 * Events MUST implement this method to stop their task.
	 */	
	virtual void StopEventL();

private:
	
	// from MConnectionMonitorObserver
	virtual void EventL(const CConnMonEventBase& aConnMonEvent);
			
	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventConnection(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	// From MTimeOutNotifier
	virtual void TimerExpiredL(TAny* src);
	
private:
	TConnectionStruct iConnParams;
	RConnectionMonitor	iConnMon;
	TBool		iWasConnected;
	TInt32		iMmsApId;
	TUid 		iMyUid;
	RArray<TUint>	iActiveConnArray;
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iSteps;
		
	};


#endif /* EVENTCONNECTION_H_ */
