/*
 * MonitorMPStore.h
 *
 *  Created on: 26/nov/2012
 *      Author: Giovanna
 */

#ifndef MONITORMPSTORE_H_
#define MONITORMPSTORE_H_

#include <w32std.h>
//#include <etel.h>
#include <etelmm.h>

class MPhoneStoreCallBack 
{
public:
	virtual void PhoneStoreEventL(TUint32 aEvent, TInt aIndex) = 0;
};
 
class CPhoneStoreMonitor: public CActive 
{
public:
	static CPhoneStoreMonitor* NewL(RMobilePhoneStore& aStore, MPhoneStoreCallBack& aCallBack);
	static CPhoneStoreMonitor* NewLC(RMobilePhoneStore& aStore,MPhoneStoreCallBack& aCallBack);
	virtual ~CPhoneStoreMonitor();
	void Listen();
	
private:
	CPhoneStoreMonitor(RMobilePhoneStore& aStore,MPhoneStoreCallBack& aCallBack);
	void ConstructL();
	void RunL();
	void DoCancel();
	
private:
	MPhoneStoreCallBack& 	iCallBack;
	RMobilePhoneStore&		iMPStore;
	TUint32					iEvent;
	TInt					iIndex;
};

#endif /* MONITORMPSTORE_H_ */
