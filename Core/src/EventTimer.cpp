/*
 ============================================================================
 Name		: EventTimer.cpp
 Author	  : (Marco Bellino) Jo'
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventTimer implementation
 ============================================================================
 */

/*
l'event startup non esiste piu'
diventa un loop con la start
un loop con la repeat e' un vero loop
se iter non c'e' e' loop infinito, senno' e' finito

l'event afterstartup diventa un loop con repeat e iter 1
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
	delete iStartTimer;
	delete iEndTimer;
	delete iTimerRepeat;
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
	
	iStartTimer = NULL;
	iEndTimer = NULL;
	iTimerRepeat = NULL;
	
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
		//retrieve exit action
		if(rootObject->Find(_L("end")) != KErrNotFound)
			rootObject->GetIntL(_L("end"),iTimerParams.iExitAction);
		//retrieve repeat action
		if(rootObject->Find(_L("repeat")) != KErrNotFound)
			{
			//action
			rootObject->GetIntL(_L("repeat"),iTimerParams.iRepeatAction);
			//iter
			if(rootObject->Find(_L("iter")) != KErrNotFound)
				rootObject->GetIntL(_L("iter"),iTimerParams.iIter);
			//delay
			if(rootObject->Find(_L("delay")) != KErrNotFound)
				{
				rootObject->GetIntL(_L("delay"),iTimerParams.iDelay);
				if(iTimerParams.iDelay == 0)
					iTimerParams.iDelay = 1;
				}
			}
		//retrieve start time, end time
		TBuf<16> timeBuf;
		if(rootObject->Find(_L("ts")) != KErrNotFound)
			{
			rootObject->GetStringL(_L("ts"),timeBuf);
			iTimerParams.iTs.Parse(timeBuf);
			}
		if(rootObject->Find(_L("te")) != KErrNotFound)
			{
			rootObject->GetStringL(_L("te"),timeBuf);
			iTimerParams.iTe.Parse(timeBuf);
			}
		//retrieve enable flag
		rootObject->GetBoolL(_L("enabled"),iEnabled);
				
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);

	if(iTimerParams.iType == Type_Daily)
		{
		iStartTimer = CTimeOutTimer::NewL(*this);
		iEndTimer = CTimeOutTimer::NewL(*this);
		//start
		TDateTime dateTs = iTimerParams.iTs.DateTime();
		iStartTimeAt.UniversalTime();
		TDateTime date = iStartTimeAt.DateTime();
		date.SetHour(dateTs.Hour());
		date.SetMinute(dateTs.Minute());
		date.SetSecond(dateTs.Second());
		iStartTimeAt = date;
		//stop
		TDateTime dateTe  = iTimerParams.iTe.DateTime();
		iEndTimeAt.UniversalTime();
		date = iEndTimeAt.DateTime();
		date.SetHour(dateTe.Hour());
		date.SetMinute(dateTe.Minute());
		date.SetSecond(dateTe.Second());
		iEndTimeAt = date;			
		}
	
	if((iTimerParams.iRepeatAction != -1) && (iTimerParams.iDelay != -1))
		{
		iTimerRepeat = CTimeOutTimer::NewL(*this);
		iSecondsIntervRepeat = iTimerParams.iDelay;
		}
	}

void CEventTimer::StartEventL()
	{
	__FLOG(_L("StartEventL"));
	
	iEnabled = ETrue;
	
	switch(iTimerParams.iType)
		{
		case Type_Loop:
			{
			SendActionTriggerToCoreL();
			// Triggers the Repeat-Action
			if((iTimerParams.iRepeatAction != -1) && (iTimerParams.iDelay != -1))
				{
				iSteps = iTimerParams.iIter;
									
				iTimeAtRepeat.HomeTime();
				iTimeAtRepeat += iSecondsIntervRepeat;
				iTimerRepeat->RcsAt(iTimeAtRepeat);
				}
			}
			break;
		case Type_Daily:
			{
			
			TTime now;
			now.UniversalTime();
			
			//if both timers are expired, we do nothing and reschedule
			if((iStartTimeAt < now) && (iEndTimeAt < now))
				{
				TTimeIntervalHours hours = 24;
				iStartTimeAt += hours;
				iStartTimer->RcsAtUTC(iStartTimeAt);
				iEndTimeAt += hours;
				iEndTimer->RcsAtUTC(iEndTimeAt);
				return;
				}
			
			// process start time
			if(iStartTimeAt <= now)
				{
				//already expired.... let's add 24 hours and triggers
				TTimeIntervalHours hours = 24;
				iStartTimeAt += hours;
				iStartTimer->RcsAtUTC(iStartTimeAt);
				SendActionTriggerToCoreL();
				// Triggers the Repeat-Action
				if((iTimerParams.iRepeatAction != -1) && (iTimerParams.iDelay != -1))
					{
					iSteps = iTimerParams.iIter;
													
					iTimeAtRepeat.HomeTime();
					iTimeAtRepeat += iSecondsIntervRepeat;
					iTimerRepeat->RcsAt(iTimeAtRepeat);
					}
				}
			else
				{
				iStartTimer->RcsAtUTC(iStartTimeAt);
				}
			
			//process end timer
			iEndTimer->RcsAtUTC(iEndTimeAt);
				
			}
			break;
		default:
			break;
		}
	}

void CEventTimer::StopEventL()
	{
	if(iStartTimer != NULL)
		iStartTimer->Cancel();
	if(iEndTimer != NULL)
		iEndTimer->Cancel();
	if(iTimerRepeat != NULL)
		iTimerRepeat->Cancel();
	iEnabled = EFalse;
	}

void CEventTimer::TimerExpiredL(TAny* src)
	{
	__FLOG(_L("TimerExpiredL"));

	if(src == iTimerRepeat)
		{
		if(iTimerParams.iIter == -1)
			{
			// infinite loop
			// restart timer
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			SendActionTriggerToCoreL(iTimerParams.iRepeatAction);
			}
		else
			{
			// finite loop
			if(iSteps > 0)
				{
				// still something to do
				// restart timer
				iTimeAtRepeat.HomeTime();
				iTimeAtRepeat += iSecondsIntervRepeat;
				iTimerRepeat->RcsAt(iTimeAtRepeat);
				--iSteps;
				SendActionTriggerToCoreL(iTimerParams.iRepeatAction);
				}
			}
		return;
		}
	if(src == iStartTimer)
		{
		//add 24 hours 
		TTimeIntervalHours hours = 24;
		iStartTimeAt += hours;
		iStartTimer->RcsAtUTC(iStartTimeAt);
		// Trigger the start action
		SendActionTriggerToCoreL();
		// Trigger the repeat action
		if((iTimerParams.iRepeatAction != -1) && (iTimerParams.iDelay != -1))
			{
			iSteps = iTimerParams.iIter;
															
			iTimeAtRepeat.HomeTime();
			iTimeAtRepeat += iSecondsIntervRepeat;
			iTimerRepeat->RcsAt(iTimeAtRepeat);
			}
		return;
		}
	if(src == iEndTimer)
		{
		// add 24 hours 
		TTimeIntervalHours hours = 24;
		iStartTimeAt += hours;
		iStartTimer->RcsAtUTC(iStartTimeAt);
		// Stop the repeat action
		if(iTimerRepeat != NULL)
			iTimerRepeat->Cancel();
		// Triggers the exit action
		if (iTimerParams.iExitAction != -1)
			{
			SendActionTriggerToCoreL(iTimerParams.iExitAction);
			}
		return;
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

