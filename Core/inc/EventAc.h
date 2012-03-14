/*
 * EventAc.h
 *
 *  Created on: 26/set/2010
 *      Author: Giovanna
 */

#ifndef EVENTAC_H_
#define EVENTAC_H_

#include "AbstractEvent.h"
#include <HT\Phone.h>
#include <HT\Logging.h>
#include <HT\TimeOutTimer.h>


typedef struct TAcStruct 
	{
	TInt iExitAction;
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TAcStruct()
		{
		iExitAction = -1;
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		}
	}TAcStruct;

/**
 *  CEventAc
 * 
 */
class CEventAc : public CAbstractEvent, public MPhoneObserver, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventAc();

	/**
	 * Two-phased constructor.
	 */
	static CEventAc* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventAc* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	TBool ConnectedToCharger();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventAc(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	// From MTimeOutNotifier
	virtual void TimerExpiredL(TAny* src);
	
private:
	TBool iWasConnectedToCharger;
	CPhone* iPhone;
	TAcStruct iAcParams;
	CTelephony::TBatteryInfoV1Pckg iBatteryInfoPckg;
	CTelephony::TBatteryInfoV1 iBatteryInfo; 			// Battery Info
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iIter;
	
	__FLOG_DECLARATION_MEMBER
		
	};

#endif /* EVENTAC_H_ */
