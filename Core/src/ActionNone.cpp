/*
 ============================================================================
 Name		: ActionNone.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CActionNone implementation
 ============================================================================
 */

#include "ActionNone.h"

CActionNone::CActionNone(TActionType aId, TQueueType aQueueType) :
	CAbstractAction(aId, aQueueType)
	{
	// No implementation required
	}

CActionNone::~CActionNone()
	{
	}

CActionNone* CActionNone::NewLC(TActionType aId, const TDesC8& params, TQueueType aQueueType)
	{
	CActionNone* self = new (ELeave) CActionNone(aId, aQueueType);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionNone* CActionNone::NewL(TActionType aId, const TDesC8& params, TQueueType aQueueType)
	{
	CActionNone* self = CActionNone::NewLC(aId, params, aQueueType);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionNone::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	}

void CActionNone::DispatchStartCommandL()
	{
	MarkCommandAsDispatchedL();
	SetFinishedJob(ETrue);
	}

