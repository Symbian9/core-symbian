/*
 ============================================================================
 Name		: EventTimer.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventTimer implementation
 ============================================================================
 */

#include "EventTimer.h"
#include <HT\TimeUtils.h>
#include "json.h"
	
CEventTimer::CEventTimer(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Timer, aTriggerId)
	{
	// No implementation required
	}

CEventTimer::~CEventTimer()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	delete iEndTimer;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CEventTimer* CEventTimer::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventTimer* self = new (ELeave) CEventTimer(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventTimer* CEventTimer::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventTimer* self = CEventTimer::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}
 
void CEventTimer::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventTimer.txt");
	__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	iTimer = CTimeOutTimer::NewL(*this);
	iEndTimer = CTimeOutTimer::NewL(*this);

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
		//retrieve subtype
		TBuf<32> subtype;
		rootObject->GetStringL(_L("subtype"),subtype);
		if(subtype.Compare(_L("loop")) == 0)
			{
			//loop timer
			iTimerParams.iType = Type_Loop;
			}
		else if(subtype.Compare(_L("daily")) == 0)
			{
			//daily timer
			iTimerParams.iType = Type_Daily;
			}
		else if(subtype.Compare(_L("startup")) == 0)
			{
			//single timer
			iTimerParams.iType = Type_Startup;
			//TODO: this is not translated correctly now by console
			}
		
		//retrieve exit action
		if(rootObject->Find(_L("end")) != KErrNotFound)
			{
			rootObject->GetIntL(_L("end"),iTimerParams.iExitAction);
			}
		else
			iTimerParams.iExitAction = -1;
			
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			rootObject->GetIntL(_L("repeat"),iTimerParams.iRepeatAction);
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				{
				//limited loop
				rootObject->GetIntL(_L("iter"),iTimerParams.iIter);
				}
			else
				{
				//infinite loop
				iTimerParams.iIter = -1;
				}
			rootObject->GetIntL(_L("delay"),iTimerParams.iDelay);
			}
		else
			{
			iTimerParams.iRepeatAction = -1;
			iTimerParams.iIter = -1;
			iTimerParams.iDelay = -1;
			}
		
		//retrieve start time, end time
		TBuf<16> timeBuf;
		rootObject->GetStringL(_L("ts"),timeBuf);
		iTimerParams.iTs.Parse(timeBuf);
		rootObject->GetStringL(_L("te"),timeBuf);
		iTimerParams.iTe.Parse(timeBuf);
		
		//retrieve enable flag
		iEnabled = EFalse;
		TBuf<8> enableBuf;
		rootObject->GetStringL(_L("enabled"),enableBuf);
		if(enableBuf.Compare(_L("true")) == 0)
			{
			iEnabled = ETrue;
			}
		
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	switch(iTimerParams.iType)
		{
		case Type_Loop:
			{
			if(iTimerParams.iIter == -1)
				{
				//infinite loop
				iSecondsInterv = iTimerParams.iDelay;
				}
			else
				{
				//TODO: finite loop
				}
			}
			break;
		case Type_Startup:
			{
			iSecondsInterv = iTimerParams.iDelay;
			}
			break;
		case Type_Daily:
			{
			//start
			TDateTime dateTs = iTimerParams.iTs.DateTime();
			iTimeAt.UniversalTime();
			TDateTime date = iTimeAt.DateTime();
			date.SetHour(dateTs.Hour());
			date.SetMinute(dateTs.Minute());
			date.SetSecond(dateTs.Second());
			iTimeAt = date;
			//stop
			TDateTime dateTe  = iTimerParams.iTe.DateTime();
			iEndTimeAt.UniversalTime();
			date = iEndTimeAt.DateTime();
			date.SetHour(dateTe.Hour());
			date.SetMinute(dateTe.Minute());
			date.SetSecond(dateTe.Second());
			iEndTimeAt = date;			
			}
			break;
		default:
			break;
		}
	}

void CEventTimer::StartEventL()
	{
	__FLOG(_L("StartEventL"));
	switch(iTimerParams.iType)
		{
		case Type_Loop:
			{
			if(iTimerParams.iIter == -1)
				{
				//infinite loop
				iTimeAt.HomeTime();
				iTimeAt += iSecondsInterv;
				iTimer->RcsAt(iTimeAt);		
				}
			else
				{
				//TODO: finite loop
				}
			}
			break;
		case Type_Startup:
			{
			iTimeAt.HomeTime();
			iTimeAt += iSecondsInterv;
			iTimer->RcsAt(iTimeAt);		
			}
			break;
		case Type_Daily:
			{
			// what happens if timer daily already expired? at the moment is missed
			TTime now;
			now.UniversalTime();
			if(iTimeAt <= now)
				{
				//already expired.... let's add 24 hours
				TTimeIntervalHours hours = 24;
				iTimeAt += hours;
				iTimer->RcsAtUTC(iTimeAt);
				}
			else
				{
				iTimer->RcsAtUTC(iTimeAt);
				}
			if(iEndTimeAt <=now)
				{
				TTimeIntervalHours hours = 24;
				iEndTimeAt += hours;
				iEndTimer->RcsAtUTC(iEndTimeAt);
				}
			else
				{
				iEndTimer->RcsAtUTC(iEndTimeAt);
				}
			}
			break;
		default:
			break;
		}
	}


void CEventTimer::TimerExpiredL(TAny* src)
	{
	__FLOG(_L("TimerExpiredL"));

	switch(iTimerParams.iType)
		{
		case Type_Loop:
			{
			if(iTimerParams.iIter == -1)
				{
				//infinite loop
				iTimeAt.HomeTime();
				iTimeAt += iSecondsInterv;
				iTimer->RcsAt( iTimeAt );
				if (iTimerParams.iRepeatAction != -1)
					{
					SendActionTriggerToCoreL(iTimerParams.iRepeatAction); 
					}
				}
			else
				{
				//TODO: finite loop
				}
			}
			break;
		case Type_Startup:
			{
			SendActionTriggerToCoreL(iTimerParams.iRepeatAction); 
			}
			break;
		case Type_Daily:
			{
			TTimeIntervalHours hours(24);
			if(src == iTimer)
				{
				//start action
				iTimeAt += hours;
				iTimer->RcsAtUTC(iTimeAt);
				SendActionTriggerToCoreL();
				}
			else if(src == iEndTimer)
				{
				//end action
				if (iTimerParams.iExitAction != -1)						
					SendActionTriggerToCoreL(iTimerParams.iExitAction); 
				}
			}
			break;
		default:
			break;
		}
	}

/*
 * A filetime is a 64-bit value that represents the number of 100-nanosecond intervals 
 * that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
 * Please also note that in defining KInitialTime the month and day values are offset from zero.
 * 
 */
/*
 L'EventTimer, definito nel file di configurazione dal relativo EventId, triggera l'azione ad esso associata ad intervalli di tempo prestabiliti.

 Sono disponibili tre tipi di timer:

 1. Single: il timer triggera la relativa azione una volta, dopo l'intervallo di tempo stabilito.
 2. Loop: il timer triggera la relativa azione ogni volta che si raggiunge l'intervallo di tempo stabilito.
 3. Date: il timer triggera la relativa azione alla data stabilita. 

 Parametri

 L'evento riceve, tramite la propria EventStruct, una seconda struttura cosi' definita:

 typedef struct _TimerStruct {
 UINT uType;     // Tipo di timer
 UINT Lo_Delay;  // Low Delay Part in ms
 UINT Hi_Delay;  // High Delay Part in ms
 } TimerStruct, *pTimerStruct;

 Timer Type

 I tipi di timer sono cosi' definiti:

 * CONF_TIMER_SINGLE = 0: triggera dopo n millisecondi.
 * CONF_TIMER_REPEAT = 1: triggera ogni n millisecondi.
 * CONF_TIMER_DATE = 2: triggera una volta alla data x identificata come timestamp (ovvero: Lo_Delay e Hi_Delay della TimerStruct). 
 */

