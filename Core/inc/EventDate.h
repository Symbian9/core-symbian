/*
 * EventDate.h
 *
 *  Created on: 21/dic/2011
 *      Author: Giovanna
 */

#ifndef EVENTDATE_H_
#define EVENTDATE_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>

// CLASS DECLARATION

typedef struct TDateStruct
	{
	TInt iExitAction;
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	} TDateStruct;

/**
 *  CEventDate
 * 
 */
class CEventDate : public CAbstractEvent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventDate();

	/**
	 * Two-phased constructor.
	 */
	static CEventDate* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventDate* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	CEventDate(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    
private:
    TDateStruct	iDateParams;
	CTimeOutTimer* iTimer;
	TTime iTimeAt;
	CTimeOutTimer* iTimerTo;
	TTime iTimeAtTo;
	TBool iDateTo;
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iIter;

	__FLOG_DECLARATION_MEMBER
	};
	
	
	
	
#endif /* EVENTDATE_H_ */
