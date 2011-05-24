/*
 * SlimMonitorPhoneCall.h
 *
 *  Created on: 19/apr/2011
 *      Author: Giovanna
 */

#ifndef SLIMMONITORPHONECALL_H_
#define SLIMMONITORPHONECALL_H_

#include <e32base.h>
#include <Etel3rdParty.h>
#include <HT\Logging.h>


class MSlimCallMonCallBack
{
public:	
	virtual void NotifyIdle()=0;
	virtual void NotifyDialling()=0;
	virtual void NotifyRinging()=0;
};

class CSlimPhoneCallMonitor : public CActive
  {

public: // public constructors & destructor
  	
	static CSlimPhoneCallMonitor* NewLC(MSlimCallMonCallBack &aCallBack);
  	static CSlimPhoneCallMonitor* NewL(MSlimCallMonCallBack &aCallBack);
  	~CSlimPhoneCallMonitor();
  	void StartListeningForEvents();
  	/*
  	 * ActiveCall check if there's an active/connected call; only if ETrue, aNumber is meaningfull;; and only 
  	 * in that case, if aNumber.Length() == 0, then aNumber is a private number. 
  	 */
  	TBool ActiveCall(TDes& aNumber);
  	
protected:
  	// from CActive
  	void DoCancel();
  	void RunL();
  	TInt RunError(TInt /*aError*/);

private: 
  	// private constructors
  	void ConstructL();
  	CSlimPhoneCallMonitor(MSlimCallMonCallBack &aCallBack);
	// private internal functions
  	
private:
  	
	MSlimCallMonCallBack&	iCallBack;
  	CTelephony* 		iTelephony;
  	CTelephony::TCallStatusV1 		iCallStatus;
 	CTelephony::TCallStatusV1Pckg 	iCallStatusPckg;
 	
 	__FLOG_DECLARATION_MEMBER
 		
};


#endif /* SLIMMONITORPHONECALL_H_ */
