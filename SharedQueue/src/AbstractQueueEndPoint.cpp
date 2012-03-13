/*
 ============================================================================
 Name		: AbstractQueueEndPoint.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractQueueEndPoint implementation
 ============================================================================
 */

#include "AbstractQueueEndPoint.h"

EXPORT_C CAbstractQueueEndPoint::CAbstractQueueEndPoint(TInt aType, TInt aCreationQueueId) : 
	iType(aType), iAtCreationQueueId(aCreationQueueId)
	{
	// No implementation required
	}

EXPORT_C CAbstractQueueEndPoint::~CAbstractQueueEndPoint()
	{
	__FLOG(_L("Destructor"));
	delete iPS_PrimaryTopAddedOrRemoved;
	delete iPS_SecondaryTopAddedOrRemoved;
	//iParams.Close();
	iQueue.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

EXPORT_C TInt CAbstractQueueEndPoint::CAbstractQueueEndPoint::Type()
	{
	return iType;
	}

EXPORT_C void CAbstractQueueEndPoint::SetReceiveCmd(TBool canReceive)
	{
	if (canReceive == iCanReceive)
		return;
	iCanReceive = canReceive;
	if (canReceive)
		{
		__FLOG(_L("SetReceiveCmd: True"));
		iPS_PrimaryTopAddedOrRemoved->StartMonitorProperty();
		iPS_SecondaryTopAddedOrRemoved->StartMonitorProperty();
		}
	else
		{
		__FLOG(_L("SetReceiveCmd: False"));
		iPS_PrimaryTopAddedOrRemoved->Cancel();
		iPS_SecondaryTopAddedOrRemoved->Cancel();
		}
	__FLOG(_L("SetReceiveCmd: End"));
	}

EXPORT_C void CAbstractQueueEndPoint::SetFinishedJob(TBool aValue)
	{
	iFinishedJob = aValue;
	}

EXPORT_C TBool CAbstractQueueEndPoint::CanReceiveCmd()
	{
	return iCanReceive;
	}

EXPORT_C TBool CAbstractQueueEndPoint::FinishedJob()
	{
	return iFinishedJob;
	}

EXPORT_C void CAbstractQueueEndPoint::BaseConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "QueueEndPoint.txt");
	__FLOG(_L("-------------"));
	__FLOG_1(_L("Type: %x"), iType);
		
	//iParams.Create(params);
	
	User::LeaveIfError(iQueue.Connect());
	iPS_PrimaryTopAddedOrRemoved = CPubSubObserver::NewL(*this, KPropertyUidSharedQueue,
				KPropertyKeyPrimarySharedQueueTopAddedOrRemoved);
	iPS_SecondaryTopAddedOrRemoved = CPubSubObserver::NewL(*this, KPropertyUidSharedQueue,
					KPropertyKeySecondarySharedQueueTopAddedOrRemoved);
	SetReceiveCmd(ETrue);
	} 

EXPORT_C void CAbstractQueueEndPoint::PropertyChangedL(TUid category, TUint key, TInt value)
	{
	
		ASSERT_PANIC( category == KSharedQueueSrvUid3, 11 );
		//ASSERT_PANIC( key == KPropertyKeySharedQueueTopAddedOrRemoved, 12 );
		TInt queueId;
		if(key == KPropertyKeyPrimarySharedQueueTopAddedOrRemoved)
			queueId = EPrimaryQueue; //iQueueId = EPrimaryQueue;
		else  //secondary queue
			queueId = ESecondaryQueue; //iQueueId = ESecondaryQueue;

		//TODO: verify this before 8.0
		if(iAtCreationQueueId != 0)
			{
			// we are an action
			//if(iAtCreationQueueId != iQueueId)
			if(iAtCreationQueueId != queueId)
				return;  //this is not the queue we have to check as an action
			}
		// TODO: end verify this before 8.0
		
		//if (iQueue.IsEmpty(iQueueId))
		if(iQueue.IsEmpty(queueId))
			{
			return;
			} 
		
		// A new Command is available on the Queue
		//TCmdStruct command = iQueue.Top(iQueueId);
		TCmdStruct command = iQueue.Top(queueId);


		//if (ShouldReceiveThisCommandL(command) && iQueue.LockTop(iQueueId))  
		if (ShouldReceiveThisCommandL(command) && iQueue.LockTop(queueId))  
			{
			iQueueId = queueId;
			__FLOG_3(_L("Dispatch Src: %x  Dest: %x  Type: %x"), command.iSrc, command.iDest, command.iType);
			DispatchCommandL(command); 
			}
	
	}

EXPORT_C TBool CAbstractQueueEndPoint::ShouldReceiveThisCommandL(TCmdStruct aCommand)
	{
	return (aCommand.iDest == iType);
	}

EXPORT_C void CAbstractQueueEndPoint::MarkCommandAsDispatchedL()
	{
	__FLOG(_L("MarkCommandAsDispatchedL Start"));
	TCmdStruct removed = iQueue.Dequeue(iQueueId);
	__FLOG_3(_L("Removed Src: %x  Dest: %x  Type: %x"), removed.iSrc, removed.iDest, removed.iType);
	if (removed.iDest != iType)
		{
		User::Panic(_L("Errors in Queue-2"), KErrGeneral);
		}
	__FLOG(_L("MarkCommandAsDispatchedL End"));
			
	}

EXPORT_C void CAbstractQueueEndPoint::SubmitNewCommandL(TInt aQueueId, TCmdStruct aCommand)
	{
	__FLOG(_L("SubmitNewCommandL Start"));
	iQueue.Enqueue(aQueueId, aCommand);
	__FLOG(_L("SubmitNewCommandL End"));
	}

EXPORT_C void CAbstractQueueEndPoint::DoEmptyQueueL()
	{
	__FLOG(_L("EmptyQueueL"));
	iQueue.DoEmpty();
	}

