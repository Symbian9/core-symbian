/*
 * ActionEvent.h
 *
 *  Created on: 08/feb/2012
 *      Author: Giovanna
 */

#ifndef ACTIONEVENT_H_
#define ACTIONEVENT_H_

#include "AbstractAction.h"
#include "AbstractEvent.h"
#include "Core.h"

/**
 *  CActionEvent
 * 
 */
class CActionEvent : public CAbstractAction
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionEvent();

	/**
	 * Two-phased constructor.
	 */
	static CActionEvent* NewL(const TDesC8& params, TQueueType aQueueType);

	/**
	 * Two-phased constructor.
	 */
	static CActionEvent* NewLC(const TDesC8& params, TQueueType aQueueType);

	void SetCorePointer(CCore* aCore);
		
	
protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionEvent(TQueueType aQueueType);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	
	CCore* iCore; // do not delete this!!
	TBool iEnable;
	TInt iEventIdx;
	};


#endif /* ACTIONEVENT_H_ */
