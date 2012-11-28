/*
 * MonitorCrisis.cpp
 *
 *  Created on: 05/giu/2012
 *      Author: Giovanna
 */

#include "MonitorCrisis.h"
#include "Keys.h"


CMonitorCrisis::CMonitorCrisis(const TInt aMask,MCrisisCallBack& aCallBack):
		CActive(EPriorityStandard), iCategoryUid( KPropertyUidCore ),iKey( KPropertyCrisis ),iMask(aMask),iCallBack(aCallBack)
	{
	// adds CPubSubObserver to the active scheduler
	CActiveScheduler::Add(this);
	}

CMonitorCrisis::~CMonitorCrisis()
	{
	// cancel any request still pending
	Cancel();
	// closing the handle to the property
	iProperty.Close();
	}

CMonitorCrisis* CMonitorCrisis::NewL(const TInt aMask,MCrisisCallBack& aCallBack)
	{
	CMonitorCrisis* self = CMonitorCrisis::NewLC(aMask, aCallBack);
	CleanupStack::Pop(); // self;
	return self;
	}

CMonitorCrisis* CMonitorCrisis::NewLC(const TInt aMask,MCrisisCallBack& aCallBack)
	{
	CMonitorCrisis* self = new (ELeave) CMonitorCrisis(aMask,aCallBack);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CMonitorCrisis::ConstructL()
	{
    User::LeaveIfError(iProperty.Attach(iCategoryUid, iKey));   
	}
 
 
void CMonitorCrisis::DoCancel()
	{
	iProperty.Cancel();
	}
 
void CMonitorCrisis::RunL()
	{
	Start();
	
    TInt value = 0;
    TInt err = RProperty::Get(iCategoryUid,iKey,value);
    if(err == KErrNone)
    	{
    	if(value & iMask)
    		iCallBack.CrisisOnL();
    	else
    		iCallBack.CrisisOffL();
    	}
    }

void CMonitorCrisis::Start()
	{
	if (!IsActive())
		{
		// now subscribe 
		iProperty.Subscribe(iStatus);
		SetActive();
		}
	}


