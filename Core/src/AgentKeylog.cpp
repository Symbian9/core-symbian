/*
 * AgentKeyLog.cpp
 *
 *  Created on: 08/giu/2011
 *      Author: Giovanna
 */

#include "AgentKeylog.h"

CAgentKeylog::CAgentKeylog() :
	CAbstractAgent(EAgent_Keylog)
	{
	// No implementation required
	}

CAgentKeylog::~CAgentKeylog()
	{
	__FLOG(_L("Destructor"));
	
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentKeylog* CAgentKeylog::NewLC(const TDesC8& params)
	{
	CAgentKeylog* self = new (ELeave) CAgentKeylog();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentKeylog* CAgentKeylog::NewL(const TDesC8& params)
	{
	CAgentKeylog* self = CAgentKeylog::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentKeylog::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN("HT", "Agent_Keylog.txt");
	__FLOG(_L("-------------"));
		
	BaseConstructL(params);
		
	}

void CAgentKeylog::StartAgentCmdL()
	{
	//__FLOG(_L("StartAgentCmdL()"));
		    
	}

void CAgentKeylog::StopAgentCmdL()
	{
	//__FLOG(_L("StopAgentCmdL()"));
	
	}
