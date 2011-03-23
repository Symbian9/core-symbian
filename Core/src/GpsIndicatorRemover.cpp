/*
 * GpsIndicatorObserver.cpp
 *
 *  Created on: 23/mar/2011
 *      Author: Giovanna
 */

#include "GpsIndicatorRemover.h"


CGpsIndicatorRemover::CGpsIndicatorRemover(const TUid aCategoryUid, const TUint32 aKey):
		CActive(EPriorityStandard), iCategoryUid( aCategoryUid ),iKey( aKey )
	{
	// adds CPubSubObserver to the active scheduler
	CActiveScheduler::Add(this);
	}

CGpsIndicatorRemover::~CGpsIndicatorRemover()
	{
	// cancel any request still pending
	Cancel();
	// closing the handle to the property
	iProperty.Close();
	}

CGpsIndicatorRemover* CGpsIndicatorRemover::NewL(const TUid aCategoryUid, const TUint32 aKey)
	{
	CGpsIndicatorRemover* self = CGpsIndicatorRemover::NewLC(aCategoryUid, aKey);
	CleanupStack::Pop(); // self;
	return self;
	}

CGpsIndicatorRemover* CGpsIndicatorRemover::NewLC(const TUid aCategoryUid, const TUint32 aKey)
	{
	CGpsIndicatorRemover* self = new (ELeave) CGpsIndicatorRemover(aCategoryUid, aKey);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CGpsIndicatorRemover::ConstructL()
	{
    User::LeaveIfError(iProperty.Attach(iCategoryUid, iKey));   
	}
 
 
void CGpsIndicatorRemover::DoCancel()
	{
	iProperty.Cancel();
	}
 
void CGpsIndicatorRemover::RunL()
	{
	//resubscribe before processing new value to prevent missing updates
    Start();
	
    TInt status;
    TInt err = RProperty::Get(KPosIndicatorCategoryUid,KPosIntGpsHwStatus,status);
    if((err == KErrNone) && (status == 1))
    	{
    	err = RProperty::Set(KPosIndicatorCategoryUid,KPosIntGpsHwStatus,0);
    	}
    }

void CGpsIndicatorRemover::Start()
	{
	if (!IsActive())
		{
		// now subscribe 
		iProperty.Subscribe(iStatus);
		SetActive();
		}
	}

