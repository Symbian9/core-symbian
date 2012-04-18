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
	Start();
	
    TInt status;
    TInt err = RProperty::Get(KPosIndicatorCategoryUid,KPosIntGpsHwStatus,status);
    if((err == KErrNone) && /*(status == 1)*/ (status!=0) )  // status == 1 was fine for 5th only, it seems that in Symbian3 that value is different
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

