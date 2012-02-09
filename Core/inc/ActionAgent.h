/*
 * ActionAgent.h
 *
 *  Created on: 09/feb/2012
 *      Author: Giovanna
 */

#ifndef ACTIONAGENT_H_
#define ACTIONAGENT_H_

#include "AbstractAction.h"
#include "Core.h"

enum TAgentCommand
	{
	EAgentStart =1,
	EAgentStop
	};

/**
 *  CActionAgent
 * 
 */
class CActionAgent : public CAbstractAction
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionAgent();

	/**
	 * Two-phased constructor.
	 */
	static CActionAgent* NewL(const TDesC8& params, TQueueType aQueueType);

	/**
	 * Two-phased constructor.
	 */
	static CActionAgent* NewLC(const TDesC8& params, TQueueType aQueueType);

	void SetCorePointer(CCore* aCore);
	
protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionAgent(TQueueType aQueueType);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	TInt GetModuleId(const TDesC& aModuleName);
	
private:
	
	TAgentCommand   iCommand;
	TInt			iModuleId;
	CCore*			iCore;    //do not delete!
	};


#endif /* ACTIONAGENT_H_ */
