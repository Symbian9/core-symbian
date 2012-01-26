#ifndef __T_CLIENT_H__
#define __T_CLIENT_H__

#include <e32std.h>
#include <e32base.h>
#include <HT\SharedQueueCliSrv.h>

class RSharedQueue : public RSessionBase
	{
public:
	IMPORT_C TInt Connect();

	IMPORT_C TCmdStruct Top(TInt aQueueId);
	IMPORT_C HBufC8* TopParamL(TInt aQueueId);
	IMPORT_C TBool LockTop(TInt aQueueId);
	IMPORT_C TBool IsEmpty(TInt aQueueId);
	IMPORT_C void Enqueue(TInt aQueueId, TCmdStruct aCmd, const TDesC8& params = KNullDesC8());
	IMPORT_C void Enqueue(TInt aQueueId, TCmdType aType, TInt aSrc, TInt aDest, const TDesC8& params = KNullDesC8());
	IMPORT_C TCmdStruct Dequeue(TInt aQueueId);
	IMPORT_C void DoEmpty();
	};

#endif

