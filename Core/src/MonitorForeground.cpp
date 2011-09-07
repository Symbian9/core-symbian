/*
 * MonitorForeground.cpp
 *
 *  Created on: 21/lug/2011
 *      Author: Giovanna
 */

#include "MonitorForeground.h"
#include <COEDEF.H>



CForegroundMonitor* CForegroundMonitor::NewL(RWsSession& aWsSession,MForegroundCallBack& aCallBack)
	{
	CForegroundMonitor* self = CForegroundMonitor::NewLC(aWsSession,aCallBack);
	CleanupStack::Pop(self);
	return self;
	}
 
 
CForegroundMonitor* CForegroundMonitor::NewLC(RWsSession& aWsSession,MForegroundCallBack& aCallBack)
	{
	CForegroundMonitor* self = new (ELeave) CForegroundMonitor(aWsSession,aCallBack);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}
 
 
CForegroundMonitor::CForegroundMonitor(RWsSession& aWsSession,MForegroundCallBack& aCallBack)
:CActive(EPriorityNormal),iCallBack(aCallBack), iWsSession(aWsSession), iWg(aWsSession)
{
}
 
CForegroundMonitor::~CForegroundMonitor()
{
	Cancel();
	delete iTaskList;
	iWg.Close();
}
 
void CForegroundMonitor::ConstructL()
{
	CActiveScheduler::Add(this);
 
	iTaskList = new (ELeave) TApaTaskList(iWsSession);
	
	User::LeaveIfError(iWg.Construct((TUint32)&iWg, EFalse));
	iWg.SetOrdinalPosition(-1);
	iWg.EnableReceiptOfFocus(EFalse);
	
	CApaWindowGroupName* wn=CApaWindowGroupName::NewLC(iWsSession);
	wn->SetHidden(ETrue);
	wn->SetWindowGroupName(iWg);
	CleanupStack::PopAndDestroy();
 
	//User::LeaveIfError(iWg.EnableGroupChangeEvents());
	TInt err = iWg.EnableGroupListChangeEvents();

}
 
void CForegroundMonitor::RunL()
{
	if (iStatus == KErrNone) 
	{
		
	TWsEvent e;   
	iWsSession.GetEvent(e);
	
	TInt eventType = e.Type(); 
	
	switch(eventType)
		{
		// see TEventCode
		//case EEventWindowVisibilityChanged:  
		case EEventWindowGroupListChanged:
			{
 
			TInt wgId = iWsSession.GetFocusWindowGroup();
	 
			CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC(iWsSession, wgId);
			//retrieve the task in the foreground
			TApaTask app =  iTaskList->FindByPos(0); 
			//compare the window group ids, call the callback only if the ids are equal
			if (app.WgId() == wgId)
				{
				//wgName->ConstructFromWgIdL(wgId);
				TBuf<50> caption;
				caption.Copy(wgName->Caption());
				iCallBack.ForegroundEventL(wgName->AppUid(),caption);
				}
	 
			CleanupStack::PopAndDestroy(wgName);
			}
			break;
		default:
			break;
		}
	}
	
	if (iStatus != KErrCancel) 
		{
		Listen();
		}
}
 
void CForegroundMonitor::DoCancel()
{
	iWsSession.EventReadyCancel();
}
 
void CForegroundMonitor::Listen()
{
	if(!IsActive())
		{
		iWsSession.EventReady(&iStatus);
		SetActive();
		}
}
