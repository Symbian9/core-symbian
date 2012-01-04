/*
 * TimeUtils.cpp
 *
 *  Created on: 22/feb/2011
 *      Author: Giovanna
 */

#include "TimeUtils.h"

TInt64 TimeUtils::GetFiletime(TTime aSymbianTime)
	{
	_LIT(KInitialTime,"16010000:000000");
	TTime initialTime;
	initialTime.Set(KInitialTime);
			
	TTimeIntervalMicroSeconds interval;
	interval=aSymbianTime.MicroSecondsFrom(initialTime);
		
	return interval.Int64()*10; 
		
	}

/*
 * A filetime is a 64-bit value that represents the number of 100-nanosecond intervals 
 * that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
 * Please also note that in defining KInitialTime the month and day values are offset from zero.
 * 
 */
TInt64 TimeUtils::GetSymbianTime(TUint64 aFiletime)
	{

	_LIT(KFiletimeInitialTime,"16010000:000000");

	TTime initialFiletime;
	initialFiletime.Set(KFiletimeInitialTime);

	TInt64 interval;
	interval = initialFiletime.Int64();

	TInt64 date = aFiletime/10;

	return (interval + date);
	}


void TimeUtils::GetTimestamp(TTimestamp* aTimestamp)
	{
	TTime now;
	now.UniversalTime();
	TDateTime datetime;
	datetime = now.DateTime();
	
	aTimestamp->iSec = datetime.Second();
	aTimestamp->iMin = datetime.Minute();
	aTimestamp->iHour = datetime.Hour();
	aTimestamp->iMonthDay = datetime.Day() + 1; // day is numbered from 0
	aTimestamp->iMonth = datetime.Month()+1; // month is numbered from 0
	aTimestamp->iYear = datetime.Year(); 
	aTimestamp->iWeekDay = now.DayNoInWeek() + 1;  // this is locale dependent, pay attention
	aTimestamp->iYearDay = now.DayNoInYear() - 1; // in symbian the first day in year is number 1
	aTimestamp->iDaylightSav = 0;
	}

//Date format is: "2012-01-03 00:00:00"
TDateTime TimeUtils::GetSymbianDate(const TDesC& aDate)
	{
	TDateTime dateTime;
	
	//year
	TInt year = 0;
	TLex lex(aDate.Mid(0,4));
	lex.Val(year);
	//month
	TInt month = 0;
	lex.Assign(aDate.Mid(5,2));
	lex.Val(month);
	//day
	TInt day = 0;
	lex.Assign(aDate.Mid(8,2));
	lex.Val(day);
	//hours
	TInt hour = 0;
	lex.Assign(aDate.Mid(11,2));
	lex.Val(hour);
	//minutes
	TInt minute = 0;
	lex.Assign(aDate.Mid(14,2));
	lex.Val(minute);
	//seconds
	TInt second = 0;
	lex.Assign(aDate.Mid(17,2));
	lex.Val(second);
	
	dateTime.Set(year,TMonth(month-1),day-1,hour,minute,second,0);
	
	return dateTime;
	}
