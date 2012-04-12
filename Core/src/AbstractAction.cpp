/*
 ============================================================================
 Name		: AbstractAction.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractAction implementation
 ============================================================================
 */

#include "AbstractAction.h"

EXPORT_C CAbstractAction::CAbstractAction(TActionType aType, TQueueType aCreationQueueId) :
	CAbstractQueueEndPoint(aType, aCreationQueueId), iConditioned (EFalse)
	{
	// No implementation required
	}

EXPORT_C CAbstractAction::~CAbstractAction()
	{
	}

EXPORT_C void CAbstractAction::BaseConstructL(const TDesC8& params)
	{
	CAbstractQueueEndPoint::BaseConstructL(params);
	}


TBool CAbstractAction::ShouldReceiveThisCommandL(TCmdStruct aCommand)
	{
	if((aCommand.iDest == iType) && (aCommand.iTag == iTag))
		return ETrue;
	else
		return EFalse;
	}

void CAbstractAction::DispatchCommandL(TCmdStruct aCommand)
	{
	// Actions will receive only the Start command
	switch (aCommand.iType)
		{
		case EStart:
			// Actions will not receive any more commands after the "Start"
			SetReceiveCmd(EFalse);
			DispatchStartCommandL();
			break;
		case ECmdStop:
			ASSERT(false);
			break;
		default:
			ASSERT(false);
			break;
		}
	}
