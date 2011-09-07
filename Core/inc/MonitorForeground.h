/*
 * MonitorForeground.h
 *
 *  Created on: 21/lug/2011
 *      Author: Giovanna
 */

#ifndef MONITORFOREGROUND_H_
#define MONITORFOREGROUND_H_

#include <w32std.h>
#include <APGWGNAM.H>
#include <APGTASK.H>

class MForegroundCallBack 
{
public:
	virtual void ForegroundEventL(TUid aAppUid, const TDesC& aCaption) = 0;
};
 
class CForegroundMonitor: public CActive 
{
public:
	static CForegroundMonitor* NewL(RWsSession& aWsSession,MForegroundCallBack& aCallBack);
	static CForegroundMonitor* NewLC(RWsSession& aWsSession,MForegroundCallBack& aCallBack);
	virtual ~CForegroundMonitor();
	void Listen();
	
private:
	CForegroundMonitor(RWsSession& aWsSession,MForegroundCallBack& aCallBack);
	void ConstructL();
	void RunL();
	void DoCancel();
	
private:
	MForegroundCallBack& 	iCallBack;
	RWsSession&     iWsSession;
	RWindowGroup    iWg; 
	TApaTaskList*	iTaskList;
};

#endif /* MONITORFOREGROUND_H_ */
