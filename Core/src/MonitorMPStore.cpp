/*
 * MonitorMPStore.cpp
 *
 *  Created on: 26/nov/2012
 *      Author: Giovanna
 */

#include "MonitorMPStore.h"


CPhoneStoreMonitor* CPhoneStoreMonitor::NewL(RMobilePhoneStore& aStore,MPhoneStoreCallBack& aCallBack)
	{
	CPhoneStoreMonitor* self = CPhoneStoreMonitor::NewLC(aStore, aCallBack);
	CleanupStack::Pop(self);
	return self;
	}
 
 
CPhoneStoreMonitor* CPhoneStoreMonitor::NewLC(RMobilePhoneStore& aStore,MPhoneStoreCallBack& aCallBack)
	{
	CPhoneStoreMonitor* self = new (ELeave) CPhoneStoreMonitor(aStore,aCallBack);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}
 
 
CPhoneStoreMonitor::CPhoneStoreMonitor(RMobilePhoneStore& aStore,MPhoneStoreCallBack& aCallBack)
:CActive(EPriorityNormal),iCallBack(aCallBack),iMPStore(aStore)
{
}
 
CPhoneStoreMonitor::~CPhoneStoreMonitor()
{
	Cancel();
}
 
void CPhoneStoreMonitor::ConstructL()
{
	CActiveScheduler::Add(this);
}
 
void CPhoneStoreMonitor::RunL()
{
	if (iStatus == KErrNone) 
		{
		iCallBack.PhoneStoreEventL(iEvent,iIndex);
		Listen();
		}
	if (iStatus != KErrCancel) 
		{
		Listen();
		}
}
 
void CPhoneStoreMonitor::DoCancel()
{
	if (IsActive())
		iMPStore.CancelAsyncRequest(EMobilePhoneStoreNotifyStoreEvent);
}
 
void CPhoneStoreMonitor::Listen()
{
	if (IsActive())
		return;

	iMPStore.NotifyStoreEvent(iStatus, iEvent, iIndex);
	SetActive();
}
