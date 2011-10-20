/*
 * HighResTimeoutTimer.cpp
 *
 *  Created on: 30/ago/2011
 *      Author: Giovanna
 */

#include "HighResTimeoutTimer.h"


// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CHighResTimeoutTimer::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CHighResTimeoutTimer* CHighResTimeoutTimer::NewL(MHighResTimeoutNotifier& aTimeOutNotify, const TInt aPriority )
    {
    CHighResTimeoutTimer* self = CHighResTimeoutTimer::NewLC(aTimeOutNotify, aPriority );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CHighResTimeoutTimer::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CHighResTimeoutTimer* CHighResTimeoutTimer::NewLC( MHighResTimeoutNotifier& aTimeOutNotify, const TInt aPriority )
    {
    CHighResTimeoutTimer* self = new ( ELeave ) CHighResTimeoutTimer( aTimeOutNotify, aPriority );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CHighResTimeoutTimer::CHighResTimeoutTimer()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CHighResTimeoutTimer::CHighResTimeoutTimer( MHighResTimeoutNotifier& aTimeOutNotify, const TInt aPriority )
: CTimer( aPriority ), iNotify( aTimeOutNotify )
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CHighResTimeoutTimer::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CHighResTimeoutTimer::ConstructL()
    {
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CHighResTimeoutTimer::~CHighResTimeoutTimer()
// Destructor.
// -----------------------------------------------------------------------------
//
CHighResTimeoutTimer::~CHighResTimeoutTimer()
    {
	Cancel();
    }

void CHighResTimeoutTimer::RcsHighRes(TTimeIntervalMicroSeconds32 aInterval)
	{
	iInterval = aInterval;
	this->HighRes(iInterval);
	}

// -----------------------------------------------------------------------------
// CHighResTimeoutTimer::RunL()
// Called when operation completes.
// -----------------------------------------------------------------------------
//
void CHighResTimeoutTimer::RunL()
    {
    // Timer request has completed, so notify the timer's owner
	TInt status = iStatus.Int();
	if(status == KErrAbort)
		{
		//this is moslty due to a network time sync, it's ok to consider it an expiration anyway
		iNotify.HighResTimerExpiredL( this);
		}
	else
		{
		iNotify.HighResTimerExpiredL( this );
		}
    }
 
