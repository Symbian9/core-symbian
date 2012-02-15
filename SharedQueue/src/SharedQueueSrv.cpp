#include "SharedQueueErr.h"
#include "SharedQueue.pan"
#include "SharedQueueSrv.h"
#include "SharedQueueSession.h"
#include <e32des8.h>
#include <f32file.h>
#include <COEMAIN.H>
#include <e32property.h>
#include <HT\Processes.h>
//#include <HT\FileUtils.h>

CSharedQueueSrv* CSharedQueueSrv::NewLC()
	{
	CSharedQueueSrv* server = new (ELeave) CSharedQueueSrv(EPriorityNormal);
	CleanupStack::PushL(server);
	server->ConstructL();
	return server;
	}

CSharedQueueSrv::CSharedQueueSrv(TInt aPriority) :
	CServer2(aPriority)
	{
	}

void CSharedQueueSrv::ConstructL()
	{
	__FLOG_OPEN("HT", "Server.txt");
	__FLOG(_L("----CONSTR----"));
	TRAPD(err, StartL(KSharedQueueSrvName));

	if (err != KErrNone)
		{
		__FLOG(_L("ERRORE STARTL SERVER"));
		User::Leave(err);
		}

	static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy);
	TInt ris = 0;
	ris = RProperty::Define(KPropertyUidSharedQueue, KPropertyKeyPrimarySharedQueueTopAddedOrRemoved, RProperty::EInt,
				KAllowAllPolicy, KAllowAllPolicy);
	ris = RProperty::Define(KPropertyUidSharedQueue, KPropertyKeySecondarySharedQueueTopAddedOrRemoved, RProperty::EInt,
					KAllowAllPolicy, KAllowAllPolicy);
	iPrimaryTopIsLocked = EFalse;
	iSecondaryTopIsLocked = EFalse;
	__FLOG_1(_L("PS_Define:%d"), ris);
	}

TBool CSharedQueueSrv::LockTop(TInt aQueueId)
	{
	if(aQueueId == EPrimaryQueue)
		{
		if (iPrimaryTopIsLocked)
			return EFalse;
		__FLOG(_L("LOCK PRIMARY TOP"));	
		iPrimaryTopIsLocked = ETrue;
			return ETrue;	
		}
	else
		{
		//secondary queue
		if (iSecondaryTopIsLocked)
			return EFalse;
		__FLOG(_L("LOCK SECONDARY TOP"));	
		iSecondaryTopIsLocked = ETrue;
		return ETrue;
		}
	}

void CSharedQueueSrv::UnlockTop(TInt aQueueId)
	{
	if(aQueueId == EPrimaryQueue)
		{
		__FLOG(_L("UNLOCK PRIMARY TOP"));
		iPrimaryTopIsLocked = EFalse;
		}
	else  //secondary queue
		{
		__FLOG(_L("UNLOCK SECONDARY TOP"));
		iSecondaryTopIsLocked = EFalse;
		}
	}


TBool CSharedQueueSrv::IsEmpty(TInt aQueueId)
	{
	if(aQueueId == EPrimaryQueue)
		return (iPrimaryArray.Count() <= 0);
	else //secondary queue
		{
		return (iSecondaryArray.Count() <=0);
		}
	}

TCmdStruct CSharedQueueSrv::TopL(TInt aQueueId)
	{
	if (IsEmpty(aQueueId))
		User::Leave(KErrQueueIsEmpty);
	TCmdStruct top;
	if(aQueueId == EPrimaryQueue)
		top = iPrimaryArray[0];
	else  //secondary queue
		top = iSecondaryArray[0];
//	__FLOG_2(_L("Top Dest: %x  Type: %x"), top.iDest, top.iType);
	return top;
	}

HBufC8* CSharedQueueSrv::TopParamL(TInt aQueueId)
	{
	if (IsEmpty(aQueueId))
		User::Leave(KErrQueueIsEmpty);
	if(aQueueId == EPrimaryQueue)
		return iPrimaryParams[0];
	else //secondary queue
		return iSecondaryParams[0];
	}

TCmdStruct CSharedQueueSrv::DequeueL(TInt aQueueId)
	{
	if (IsEmpty(aQueueId))
		User::Leave(KErrQueueIsEmpty);
	TCmdStruct res = TopL(aQueueId);
	__FLOG_2(_L("Remove Dest: %x  Type: %x"), res.iDest, res.iType);
	if(aQueueId == EPrimaryQueue)
		{
		iPrimaryArray.Remove(0);
		HBufC8* buf = iPrimaryParams[0];
		iPrimaryParams.Remove(0);
		delete buf;
		RProperty::Set(KPropertyUidSharedQueue, KPropertyKeyPrimarySharedQueueTopAddedOrRemoved, 1);
		}
	else //secondary queue
		{
		iSecondaryArray.Remove(0);
		HBufC8* buf = iSecondaryParams[0];
		iSecondaryParams.Remove(0);
		delete buf;
		RProperty::Set(KPropertyUidSharedQueue, KPropertyKeySecondarySharedQueueTopAddedOrRemoved, 1);
		}
	UnlockTop(aQueueId);
	
	/*
	if (!IsEmpty(aQueueId))  //comment this out, useful only during debug/development
		{
		TCmdStruct newTop = TopL(aQueueId);
		__FLOG_2(_L(" NewTop Dest: %x  Type: %x"), newTop.iDest, newTop.iType);	
		} else
		{
		__FLOG(_L(" Queue Empty!"));				
		}
	*/
	return res;
	}

void CSharedQueueSrv::DoEmptyL()  // added jo
	{
		UnlockTop(EPrimaryQueue);
		while(!IsEmpty(EPrimaryQueue) )
			{
			iPrimaryArray.Remove(0);
			HBufC8* buf = iPrimaryParams[0];
			iPrimaryParams.Remove(0);
			delete buf;
			}
		UnlockTop(ESecondaryQueue);
		while(!IsEmpty(ESecondaryQueue) )
			{
			iSecondaryArray.Remove(0);
			HBufC8* buf = iSecondaryParams[0];
			iSecondaryParams.Remove(0);
			delete buf;
			}
	}

void CSharedQueueSrv::EnqueueL(TInt aQueueId,TCmdStruct aCmd, const TDesC8& params)
	{
	if(aQueueId == EPrimaryQueue)
		{
		iPrimaryArray.Append(aCmd);
		iPrimaryParams.Append(params.AllocL());
		RProperty::Set(KPropertyUidSharedQueue, KPropertyKeyPrimarySharedQueueTopAddedOrRemoved, 1);
		}
	else //secondary queue
		{
		iSecondaryArray.Append(aCmd);
		iSecondaryParams.Append(params.AllocL());
		RProperty::Set(KPropertyUidSharedQueue, KPropertyKeySecondarySharedQueueTopAddedOrRemoved, 1);
		}
	}

CSession2* CSharedQueueSrv::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const

//
// Cretae a new client session. This should really check the version number.
//
	{
	//	__FLOG(_L("NewSessionL"));
	// Check we're the right version
	if (!User::QueryVersionSupported(
			TVersion(KServMajorVersionNumber, KServMinorVersionNumber, KServBuildVersionNumber), aVersion))
		{
		//	__FLOG(_L("Version NOT Supported"));
		User::Leave(KErrNotSupported);
		}

	return new (ELeave) CSharedQueueSession();
	}

void CSharedQueueSrv::AddSession()
	{
	iSessionCount++;
	__FLOG_1(_L("AddSession: %d"), iSessionCount);
	}

void CSharedQueueSrv::DropSession()
	{
	iSessionCount--;
	__FLOG_1(_L("DropSession: %d"), iSessionCount);
	if (iSessionCount <= 0)
		{
		CActiveScheduler::Stop();
		}
	}

CSharedQueueSrv::~CSharedQueueSrv()
	{
	__FLOG_1(_L("Destructor:%d"), iSessionCount );

	// This array has the ownership of the HBufC elements, so we must cleanup them using ResetAndDestroy();
	iPrimaryParams.ResetAndDestroy();
	iPrimaryParams.Close();
	iSecondaryParams.ResetAndDestroy();
	iSecondaryParams.ResetAndDestroy();
	iPrimaryArray.Close();
	iSecondaryArray.Close();
	TInt ris = 0;
	ris = RProperty::Delete(KPropertyKeyPrimarySharedQueueTopAddedOrRemoved); // Quando fa il delete viene segnalato il cambio ai Subscriber... Ai quali viene riportato il valore 0
	ris = RProperty::Delete(KPropertyKeySecondarySharedQueueTopAddedOrRemoved); // Quando fa il delete viene segnalato il cambio ai Subscriber... Ai quali viene riportato il valore 0
	__FLOG_1(_L("PS_Delete:%d"), ris);
	ris = 0;
	__FLOG_1(_L("EndDestructor:%d"), iSessionCount );
	__FLOG_CLOSE;
	}

TInt E32Main()
	{
	return CSharedQueueSrv::EntryPoint(NULL);
	}

TInt CSharedQueueSrv::EntryPoint(TAny* /*aNone*/)
	{
	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	if (!cleanupStack)
		{
		User::Panic(KSharedQueueSrvName, ESrvCreateTrapCleanup);
		}
	TRAPD(leave, ThreadMainL());
	if (leave)
		{
		User::Panic(KSharedQueueSrvName, ESrvCreateServer);
		}

	delete cleanupStack;
	cleanupStack = NULL;

	return KErrNone;
	}

void CSharedQueueSrv::ThreadMainL()
	{
	if (!Processes::RenameIfNotRunning(KSharedQueueSrvName))
		{
		// non dovrebbe mai accadere perche' il client non dovrebbe lanciarlo se c'e' gia' un server in esecuzione...
		RSemaphore semaphore;
		User::LeaveIfError(semaphore.OpenGlobal(KSharedQueueSrvImg));
		semaphore.Signal();
		semaphore.Close();
		return;
		}
	CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(activeScheduler);

	CActiveScheduler::Install(activeScheduler);

	CSharedQueueSrv::NewLC();
	//
	// Initialisation complete, now signal the client
	RSemaphore semaphore;
	User::LeaveIfError(semaphore.OpenGlobal(KSharedQueueSrvImg));

	semaphore.Signal();
	semaphore.Close();

	CActiveScheduler::Start();

	CleanupStack::PopAndDestroy(2);
	}
