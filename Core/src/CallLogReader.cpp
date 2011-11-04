/*
 * CallLogReader.cpp
 *
 *  Created on: 20/mag/2011
 *      Author: Giovanna
 */


#include "CallLogReader.h"
 

CCallLogReader* CCallLogReader::NewL(MCallLogCallBack& aCallBack, RFs& aFs)
{
    CCallLogReader* self = CCallLogReader::NewLC(aCallBack, aFs);
    CleanupStack::Pop(self);
    return self;
}
 
CCallLogReader* CCallLogReader::NewLC(MCallLogCallBack& aCallBack, RFs& aFs)
{
    CCallLogReader* self = new (ELeave) CCallLogReader(aCallBack, aFs);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}
 
CCallLogReader::CCallLogReader(MCallLogCallBack& aCallBack, RFs& aFs)
:CActive(CActive::EPriorityStandard),iCallBack(aCallBack),iFs(aFs)
{
}
 
CCallLogReader::~CCallLogReader()
{	
    Cancel();
    delete iLogView;
    delete iLogFilter;	
    delete iLogClient;		
}
 
void CCallLogReader::ConstructL(void)
{
    CActiveScheduler::Add(this);
 
    iLogClient = CLogClient::NewL(iFs);
    iLogView = CLogViewEvent::NewL(*iLogClient);
    iLogFilter = CLogFilter::NewL();
 
    iLogClient->GetString(iDirIn,R_LOG_DIR_IN);
    iLogClient->GetString(iDirOut,R_LOG_DIR_OUT);
    iLogClient->GetString(iDirMissed,R_LOG_DIR_MISSED);
    iLogClient->GetString(iDirFetched,R_LOG_DIR_FETCHED);
}

void CCallLogReader::ReadCallLogL()
	{
	iLogFilter->SetEventType(KLogCallEventTypeUid); //KLogPacketDataEventTypeUid
	
	if(iLogView->SetFilterL(*iLogFilter, iStatus))
	    {		
	    iEngineState = ECreatingView;
	    SetActive();
	    }
	else
		{
	    DoneReadingL(KErrNone);
		}
	}
 
void CCallLogReader::DoCancel()
{
    if(iLogView)
        iLogView->Cancel();
 
    if(iLogClient)
        iLogClient->Cancel();
}
 
void CCallLogReader::RunL()
{
    if(iStatus != KErrNone)
        DoneReadingL(iStatus.Int());
    else
        switch (iEngineState)
        {
            case ECreatingView:
               if(iLogView)
               {
                   // The filtered view has been successfully created
                   // so issue a request to start processing logs backwards	
                   if(iLogView->LastL(iStatus))
                   {	
                       iEngineState = EReadingEntries;
                       SetActive();
                   }
                   else
                       DoneReadingL(KErrNone);
               }
               break;
 
            case EReadingEntries:
               if(iLogView)
               {
            	   TInt callDirection;
            	   
            	   TBuf<KLogMaxDirectionLength> eventDirection;
            	   eventDirection.Copy((iLogView->Event()).Direction());
            	   	
            	   if(eventDirection.Compare(iDirIn)==0)
            		   {
            		   callDirection = EDirIn;
            		   }
            	   else if(eventDirection.Compare(iDirOut)==0)
            		   {
            		   callDirection = EDirOut;
            		   }
            	   else if(eventDirection.Compare(iDirMissed)==0)
            		   {
            		   callDirection = EDirMissed;
            		   }
            	   else if(eventDirection.Compare(iDirFetched)==0)
            		   {
            		   callDirection = EDirFetched;
            		   }
            	   
                   iCallBack.HandleCallLogEventL(callDirection, iLogView->Event());
                   
                   iEngineState = EReadingEntries;
                   if(iLogView->PreviousL(iStatus))
                       SetActive();
                   else
                       DoneReadingL(KErrNone);
               }
               break;
 
           default:
              break;
        }
}
 
 
void CCallLogReader::DoneReadingL(TInt aError)
{
    iCallBack.CallLogProcessed(aError);	
}
