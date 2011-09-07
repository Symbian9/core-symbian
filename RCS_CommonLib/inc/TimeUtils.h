/*
 * TimeUtils.h
 *
 *  Created on: 22/feb/2011
 *      Author: Giovanna
 */

#ifndef TIMEUTILS_H_
#define TIMEUTILS_H_

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		
#include <e32cmn.h>
#include <f32file.h>
#include <W32STD.H>

typedef struct TTimestamp
	{
	TInt32 iSec;     // seconds after the minute - [0,59] 
	TInt32 iMin;     // minutes after the hour - [0,59] 
	TInt32 iHour;    // hours since midnight - [0,23] 
	TInt32 iMonthDay;    // day of the month - [1,31] 
	TInt32 iMonth;     // months since January - [0,11] 
	TInt32 iYear;    // years since 1900 
	TInt32 iWeekDay;    // days since Sunday - [0,6] 
	TInt32 iYearDay;    // days since January 1 - [0,365] 
	TInt32 iDaylightSav;   // daylight savings time flag 
	} TTimestamp;

class TimeUtils
	{
public:
	/*
	 * Given a Symbian TTime, a Windows Filetime is returned.
	 */
	static TInt64 GetFiletime(TTime aSymbianTime);
	/*
	 * Given a Windows Filetime, a Symbian time is returned.
	 */
	static TInt64 GetSymbianTime(TUint64 aFiletime);
	/*
	 * Get the TTimestamp struct.
	 */
	static void GetTimestamp(TTimestamp* aTimestamp);

	};


#endif /* TIMEUTILS_H_ */
