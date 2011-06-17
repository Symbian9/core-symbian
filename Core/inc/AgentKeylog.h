/*
 * AgentKeyLog.h
 *
 *  Created on: 08/giu/2011
 *      Author: Giovanna
 */

#ifndef AGENTKEYLOG_H_
#define AGENTKEYLOG_H_

// INCLUDES
#include "AbstractAgent.h"
#include "AdditionalDataStructs.h"

// CLASS DECLARATION

/**
 *  CAgentKeylog
 * 
 */
class CAgentKeylog : public CAbstractAgent
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentKeylog();

	/**
	 * Two-phased constructor.
	 */
	static CAgentKeylog* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentKeylog* NewLC(const TDesC8& params);
	
	
protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
	virtual void NotifyAgentCmdL(TUint32 aData);
		
private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentKeylog();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
		
private: // data members
		
	__FLOG_DECLARATION_MEMBER
	};


#endif /* AGENTKEYLOG_H_ */
