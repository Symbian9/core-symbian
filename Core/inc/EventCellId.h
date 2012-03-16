/*
 ============================================================================
 Name		: EventCellId.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventCellId declaration
 ============================================================================
 */

#ifndef EVENTCellId_H
#define EVENTCellId_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\Phone.h>
#include <HT\Logging.h>
#include <HT\TimeOutTimer.h>

// CLASS DECLARATION

typedef struct TCellIdStruct
	{
	TInt iExitAction;
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TInt iMCC;	// Mobile Country Code
	TInt iMNC;	// Mobile Network Code
	TInt iLAC;	// Location Area Code  TUint32
	TInt iCell;
	TCellIdStruct()
		{
		iExitAction = -1;
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		iMCC = -1;
		iMNC = -1;
		iLAC = -1;
		iCell = -1;
		}
	} TCellIdStruct;


/**
 *  CEventCellId
 * 
 */
class CEventCellId : public CAbstractEvent, public MPhoneObserver, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventCellId();

	/**
	 * Two-phased constructor.
	 */
	static CEventCellId* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventCellId* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	 * Checks if the device is currently Connected to the CellID passed by params
	 */
	TBool ConnectedToCellID();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventCellId(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	// From MTimeOutNotifier
	virtual void TimerExpiredL(TAny* src);
	
private:
	TBool iWasConnectedToCell;
	CPhone* iPhone;
	TCellIdStruct iCellParams;
	CTelephony::TNetworkInfoV1Pckg iNetInfoPckg;
	CTelephony::TNetworkInfoV1 iNetInfo; 			// Network Info
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iSteps;
		
	__FLOG_DECLARATION_MEMBER
	};

#endif // EVENTCellId_H
