
#ifndef __CLIENTSERVER_H
#define __CLIENTSERVER_H

#include <e32std.h> 

#include "Keys.h"


const TUid KSharedQueueSrvUid3=GetUid(KUidSQSrv); 

/**
KPropertyUidSharedQueue
Property reserved for the Shared Queue
*/
const TUid KPropertyUidSharedQueue = KSharedQueueSrvUid3;


/**
KPropertyKeyPrimarySharedQueueTopAddedOrRemoved
The key to observe changes in the top element of the Primary Shared Queues
Will be updated when there's a change on the top of the primary queue
*/
const TUint KPropertyKeyPrimarySharedQueueTopAddedOrRemoved = 2;

/**
KPropertyKeySecondarySharedQueueTopAddedOrRemoved
The key to observe changes in the top element of the Secondary Shared Queues
Will be updated when there's a change on the top of the secondary queue
*/
const TUint KPropertyKeySecondarySharedQueueTopAddedOrRemoved = 3;

enum TAgentStatus
	{
	EAgent_Disabled = 1,
	EAgent_Enabled,
	EAgent_Running,
	EAgent_Stopped
	};
		
		
enum TAgentType
	{
	EAgent			= 0x1000,
	EAgent_Messages = 0x1001,				// ***
	//EAgent_Tasks_TODO = 0x1002,				// ***
	EAgent_CallList = 0x1003,				// ***
	EAgent_Device = 0x1004,					// ***
	EAgent_Position = 0x1005,				// ***
	EAgent_Call_TODO,						// ***
	EAgent_CallLocal,						// ***
	EAgent_Keylog = 0x1008,
	EAgent_Screenshot = 0x1009,				// ***
	EAgent_URL_TODO,
	EAgent_IM_TODO = 0x100b,
	EAgent_Mic = 0x100D,					// ***
	EAgent_Cam,								// ***
	EAgent_Clipboard_TODO,
	EAgent_Crisis_TODO,
	EAgent_Application = 0x1011,     		// ***
	EAgent_PDA_TODO,	
	EAgent_Addressbook,						// ***
	EAgent_Calendar,						// ***
	EAgent_Password,
	EAgent_LAST_ID
	};

enum TEventType
	{
	EEvent			= 0x2000,
	EEvent_Timer,					// ***
	EEvent_Sms,						// ***
	EEvent_Call,					// ***
	EEvent_Connection,				// ***
	EEvent_Process,					// ***
	EEvent_CellID,					// ***
	EEvent_Quota_TODO,
	EEvent_Sim_Change,				// ***
	EEvent_Location,				// ***
	EEvent_AC,						// ***
	EEvent_Battery,					// ***
	EEvent_Standby,					// ***
	EEvent_Date,					// 8.0
	EEvent_AfterInstall,            // 8.0
	EEvent_LAST_ID
	};

enum TActionType
	{
	EAction			= 0x4000,
	EAction_Sync,					// ***	
	EAction_Uninstall,				// ***
	EAction_Reload,					// ***	
	EAction_Sms,					// ***	
	EAction_Toothing_TODO,
	EAction_StartAgent,				// *** 
	EAction_StopAgent,				// *** 
	EAction_SyncPDA_TODO,
	EAction_Execute_TODO,
	EAction_SyncApn,				// ***
	EAction_Log = 0x400B,			// ***
	EAction_Event,
	EAction_Agent,
	EAction_LAST_ID	
	};

enum TOptionType
	{
	EOption_TODO	= 0x8000,
	EOption_BTGUID_TODO,
	EOption_WiFiMAC_TODO,
	EOption_LAST_ID	
	};


#define ECore 0x9000


enum TQueueType
	{
	EPrimaryQueue = 1,
	ESecondaryQueue
	};

enum TCmdType 
	{
	EStart = 1,
	ERestart,
	ECmdStop, 
	ENotify,
	ECycle
	};


typedef struct TCmdStruct	
	{
public:
	TCmdType iType;
	TInt iSrc;
	TInt iDest;
	TInt iTag;
	
public:
	inline TCmdStruct() { TCmdStruct(EStart, ECore, ECore); }
	
	inline TCmdStruct(TCmdType aType, TInt aSrc, TInt aDest) 
		{
		iType = aType;
		iSrc = aSrc;
		iDest = aDest;
		iTag = 0;
		}
	inline TCmdStruct(TCmdType aType, TInt aSrc, TInt aDest, TInt aTag) 
		{
		iType = aType;
		iSrc = aSrc;
		iDest = aDest;
		iTag = aTag;
		}
	} TCommand;


#endif


