/*
 ============================================================================
 Name		: EventTimer.h
 Author	  : (Marco Bellino) Jo'
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventTimer declaration
 ============================================================================
 */

#ifndef EVENTTIMER_H
#define EVENTTIMER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>

// CLASS DECLARATION


enum TTimerType
	{
	Type_Startup = 0,
	Type_Loop,
	Type_Daily,
	Type_Unknown
	};

typedef struct TTimerStruct
	{
	TInt iExitAction;
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TInt iType;
	TTime iTs;
	TTime iTe;
	TTimerStruct()
		{
		iExitAction = -1;
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		iType = Type_Unknown;
		}
	} TTimerStruct;

/**
 *  CEventTimer
 * 
 */
class CEventTimer : public CAbstractEvent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventTimer();

	/**
	 * Two-phased constructor.
	 */
	static CEventTimer* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventTimer* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	CEventTimer(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    
private:
    TTimerStruct	iTimerParams;
    
	CTimeOutTimer* iStartTimer;
	CTimeOutTimer* iEndTimer;
	TTime iStartTimeAt;
	TTime iEndTimeAt;
	//TTimeIntervalSeconds iSecondsInterv;
	//TInt iSteps;
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iSteps;
	__FLOG_DECLARATION_MEMBER
	};

#endif // EVENTTIMER_H
