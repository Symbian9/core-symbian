
#ifndef __T_SERVER_H__
#define __T_SERVER_H__

#include <e32base.h>
#include <e32def.h>

#include <f32file.h>
#include "SharedQueueCliSrv.h"
#include <HT\logging.h>


_LIT(KSharedQueueSrvName,	"ConnectionManager.exe"); // obfuscated name
_LIT(KSharedQueueSrvImg,	"SharedQueueSrv_20023634");		// EXE name


const TUint KServMajorVersionNumber=1;
const TUint KServMinorVersionNumber=0;
const TUint KServBuildVersionNumber=0;


enum TQueueMessages
	{
	ELockTop,
	EIsEmpty,
	EEnqueue,
	EDequeue,
	ETop,
	ETopParam,
	EDoEmpty     // added j
	};


class CSharedQueueSrv : public CServer2
	{
public:
	static CSharedQueueSrv* NewLC();
	virtual ~CSharedQueueSrv();
	static TInt EntryPoint(TAny* aStarted);
	void AddSession();
	void DropSession();
	
public:	
	TBool IsEmpty(TInt aQueueId);
	TBool LockTop(TInt aQueueId);
	void UnlockTop(TInt aQueueId);
	TCmdStruct TopL(TInt aQueueId);
	HBufC8* TopParamL(TInt aQueueId);
	TCmdStruct DequeueL(TInt aQueueId);
	void EnqueueL(TInt aQueueId, TCmdStruct aCmd, const TDesC8& params);
	void SetAccomplishedOnTopL(TInt aQueueId);
	void DoEmptyL();                  // added j
private:
	CSharedQueueSrv(TInt aPriority);
	void ConstructL();
	CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;
	static void ThreadMainL();

public:
	TInt iSessionCount;

private:
	RArray<TCmdStruct> 		iPrimaryArray;  //this is the the queue for actions using sync or uninstall
	RArray<TCmdStruct>		iSecondaryArray;  //this is the queue for normal actions 
	RPointerArray<HBufC8> iPrimaryParams;   // parameters for primary queue
	RPointerArray<HBufC8> iSecondaryParams; //parameters for secondary queue
	TBool iPrimaryTopIsLocked;    // lock on primary queue
	TBool iSecondaryTopIsLocked;  // lock on secondary queue
	__FLOG_DECLARATION_MEMBER
	};

#endif

