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
	Type_Daily = 0x4
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
	CTimeOutTimer* iTimer;
	CTimeOutTimer* iEndTimer;
	TTime iTimeAt;
	TTime iEndTimeAt;
	TTimeIntervalSeconds iSecondsInterv;
	//TTimeIntervalSeconds iEndSecondsInterv;
	__FLOG_DECLARATION_MEMBER
	};

#endif // EVENTTIMER_H
