/*
 * EventProcess.h
 *
 *  Created on: 29/set/2010
 *      Author: Giovanna
 */

#ifndef EVENTPROCESS_H_
#define EVENTPROCESS_H_

#include "AbstractEvent.h"
#include "Monitor.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>
#include <w32std.h>


typedef struct TProcessStruct 
	{
	TInt iExitAction;
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TInt iType;           // 0=process name; 1=window name
	TBuf<64> iName;
	TProcessStruct()
		{
		iExitAction = -1;
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		iType = 0;
		}
	}TProcessStruct;
	
/**
 *  CEventProcess
 * 
 */
class CEventProcess : public CAbstractEvent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventProcess();

	/**
	 * Two-phased constructor.
	 */
	static CEventProcess* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventProcess* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StopEventL();
	

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventProcess(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    
private:
    
    TProcessStruct iProcessParams;
    	
    // timer for process check
	CTimeOutTimer* iTimer;
	TTime iTimeAt;
	TTimeIntervalSeconds iSecondsInterv;
	
	// vars for process check
	TInt	iOldCount;
	TInt	iNewCount;
	RWsSession iWsSession;
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iSteps;
		
	
	__FLOG_DECLARATION_MEMBER
	};

#endif /* EVENTPROCESS_H_ */
