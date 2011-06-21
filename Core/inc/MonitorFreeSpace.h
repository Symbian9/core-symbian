/*
 * MonitorFreeSpace.h
 *
 *  Created on: 21/apr/2011
 *      Author: Giovanna
 */

#ifndef MONITORFREESPACE_H_
#define MONITORFREESPACE_H_

#include <e32base.h>
#include <e32def.h>
#include <f32file.h>


class MFreeSpaceCallBack
{
public:	
	virtual void NotifyAboveThreshold()=0;
	virtual void NotifyBelowThreshold()=0;
};

class CFreeSpaceMonitor : public CActive
  {

public: // public constructors & destructor
  	
	static CFreeSpaceMonitor* NewLC(MFreeSpaceCallBack& aCallBack,RFs& aFs);
	static CFreeSpaceMonitor* NewL(MFreeSpaceCallBack& aCallBack,RFs& aFs);
	  	
	~CFreeSpaceMonitor();
  	void StartListeningForEvents();
  	TBool IsBelowThreshold();
  	
  	
protected:
  	// from CActive
  	void DoCancel();
  	void RunL();
  	TInt RunError(TInt /*aError*/);

private: 
  	// private constructors
  	void ConstructL();
  	CFreeSpaceMonitor(MFreeSpaceCallBack& aCallBack,RFs& aFs);
	
private:
  	
	MFreeSpaceCallBack&	iCallBack;
	RFs&     	iFs;
	TInt64		iThreshold;
  		
};


#endif /* MONITORFREESPACE_H_ */
