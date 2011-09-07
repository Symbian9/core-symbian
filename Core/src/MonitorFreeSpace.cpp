/*
 * MonitorFreeSpace.cpp
 *
 *  Created on: 21/apr/2011
 *      Author: Giovanna
 */

#include "MonitorFreeSpace.h"


CFreeSpaceMonitor* CFreeSpaceMonitor::NewLC(MFreeSpaceCallBack& aCallBack/*,const TInt64& aThreshold*/,RFs& aFs)
	{
 	CFreeSpaceMonitor* self = new (ELeave) CFreeSpaceMonitor(aCallBack/*,aThreshold*/,aFs);
  	CleanupStack::PushL(self);
  	self->ConstructL();
  	return self;
	}


CFreeSpaceMonitor* CFreeSpaceMonitor::NewL(MFreeSpaceCallBack& aCallBack/*,const TInt64& aThreshold*/,RFs& aFs)
	{
 	CFreeSpaceMonitor* self = CFreeSpaceMonitor::NewLC(aCallBack/*,aThreshold*/,aFs);
  	CleanupStack::Pop(self);
  	return self;
	}


CFreeSpaceMonitor::CFreeSpaceMonitor(MFreeSpaceCallBack& aCallBack/*,const TInt64& aThreshold*/,RFs& aFs)
:CActive(EPriorityStandard),iCallBack(aCallBack),/*iThreshold(aThreshold),*/iFs(aFs)
	{
	
	}


CFreeSpaceMonitor::~CFreeSpaceMonitor()
	{
	// always cancel any pending request before deleting the objects
  	Cancel();
  	}


void CFreeSpaceMonitor::ConstructL()
	{
	// Active objects needs to be added to active scheduler
 	CActiveScheduler::Add(this);

 	// set threshold
 	TVolumeInfo volInfo;
 	TInt err = iFs.Volume(volInfo,EDriveC);
 	if(err == KErrNone)
 		{
 		TInt64 size = volInfo.iSize;
 		iThreshold = (size/10);
 		if(size <= 20971520 )  //<=20 MB
 			{
 			iThreshold = 2097152; // =2MB
 			}
 		else if(size>=104857600)  //>=100MB
 			{
 			iThreshold = 10485760; // =10MB
 			}
 		}
 	else
 		iThreshold = 2097152; //1024*1024*2  = 2MB
  	}

void CFreeSpaceMonitor::StartListeningForEvents()
	{
	if(IsActive())
		{
		Cancel();
		}
	iFs.NotifyDiskSpace(iThreshold,EDriveC,iStatus);
	SetActive();
	}
/*
-----------------------------------------------------------------------------
RunL is called by Active scheduler when the request status variable iStatus 
is set to something else than pending, function Int() can be then used
to determine if the request failed or succeeded
-----------------------------------------------------------------------------
*/ 

void CFreeSpaceMonitor::RunL()
	{
  	if(iStatus.Int() == KErrNone)
  		{
  		TVolumeInfo volumeInfo;
  		TInt err;
  		err = iFs.Volume(volumeInfo,EDriveC);
  		if(err == KErrNone)
  			{
  			TInt64 free = volumeInfo.iFree;  //in bytes
  			if(free <= iThreshold)
  				{
  				iCallBack.NotifyBelowThreshold();
  				}
  			else if(free > iThreshold)
  				{
  				iCallBack.NotifyAboveThreshold();
  				}
  			}
  		StartListeningForEvents();
  		}
  	//KErrCancel if request cancelled
  	//KErrArgument if threshold is outside limits
	}
/*
-----------------------------------------------------------------------------
newer call DoCancel in your code, just call Cancel() and let the
active scheduler handle calling this functions, if the AO is active
-----------------------------------------------------------------------------
*/ 
void CFreeSpaceMonitor::DoCancel()
	{
	iFs.NotifyDiskSpaceCancel();
	}


TInt CFreeSpaceMonitor::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}

TBool CFreeSpaceMonitor::IsBelowThreshold()
	{
	TVolumeInfo volumeInfo;
	TInt err;
	err = iFs.Volume(volumeInfo,EDriveC);
	if(err == KErrNone)
	  	{
	  	TInt64 free = volumeInfo.iFree;  //in bytes
	  	if(free <= iThreshold)
	  		{
	  		return ETrue;
	  		}
	  	else if(free > iThreshold)
	  		{
	  		return EFalse;
	  		}
	  	}
	return EFalse;
	}
