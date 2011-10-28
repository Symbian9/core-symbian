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
	Mem::Copy( &iTimerParams, iParams.Ptr(), sizeof(TTimerStruct));
		
	iTimerType = (TTimerType) iTimerParams.iType;

	TUint64 timeMillis;
	
	switch(iTimerType)
		{
		case Type_Date:
		case Type_Repeat:
		case Type_Single:
			{
			timeMillis = iTimerParams.iHiDelay;
			timeMillis <<= 32;
			timeMillis += iTimerParams.iLoDelay;
			iSecondsInterv = (timeMillis / 1000);
			
			if(iTimerType == Type_Date)
				{
				iTimeAt = TimeUtils::GetSymbianTime(timeMillis);
				}
			else
				{
				iTimeAt = (timeMillis*1000);
				}
			}
			break;
		case Type_Daily:
			{
			//start
			timeMillis = 0;
			timeMillis += iTimerParams.iLoDelay;
			iSecondsInterv = (timeMillis/1000);
			//stop
			timeMillis = 0;
			timeMillis += iTimerParams.iHiDelay;
			iEndSecondsInterv = (timeMillis/1000);
			
			//daily: the milliseconds since midnight
			iTimeAt.UniversalTime();
			TDateTime date = iTimeAt.DateTime();
			date.SetHour(0);
			date.SetMinute(0);
			date.SetSecond(0);
			date.SetMicroSecond(0);
			iTimeAt = date;
			iTimeAt += iSecondsInterv;
			
			iEndTimeAt = date;
			iEndTimeAt += iEndSecondsInterv;

			}
			break;
		default:
			break;
		}
	}

void CEventTimer::StartEventL()
	{
	__FLOG(_L("StartEventL"));
	__FLOG_2(_L("%d %d"), iTimerType, iSecondsInterv.Int());
	switch(iTimerType)
		{
		case Type_Repeat:
		case Type_Single:
			{
			iTimeAt.HomeTime();
			iTimeAt += iSecondsInterv;
			iTimer->RcsAt(iTimeAt);		
			}
			break;
		case Type_Date:
			{
			// First we have to check if the date is expired
			TTime now;
			now.UniversalTime();
			if (iTimeAt <= now)
				{
				TTimeIntervalSeconds secondsInterv = 3;
				iTimeAt.HomeTime();
				iTimeAt += secondsInterv;
				iTimer->RcsAt(iTimeAt);
				} 
			else 
				{
				iTimer->RcsAtUTC( iTimeAt );
				}		
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
	switch(iTimerType)
		{
		case Type_Repeat:
			{
			__FLOG(_L("After"));
			iTimeAt.HomeTime();
			iTimeAt += iSecondsInterv;
			iTimer->RcsAt( iTimeAt );
			SendActionTriggerToCoreL();
			}
			break;
		case Type_Date:
		case Type_Single:
			{
			SendActionTriggerToCoreL();
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
				if (iTimerParams.iExitAction != 0xFFFFFFFF)						
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

