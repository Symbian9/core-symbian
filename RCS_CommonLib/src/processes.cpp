#include "processes.h"
#include <e32cmn.h>
#include <APGTASK.H>
#include <APGWGNAM.H>

TBool Processes::RenameIfNotRunning(const TDesC& newName)
	{
	RMutex mutex;
	TInt result = mutex.CreateGlobal(newName);
	// This check already grants the mutual exclusion from the critical section
	// However, we call the mutex.Wait() just in case...
	if (result != KErrNone)
		{
		return EFalse;
		}
	mutex.Wait();

	// Critical Section
	TBool renamed = EFalse;
	if (!Processes::IsRunning(newName))
		{
		User::RenameProcess(newName);
		renamed = ETrue;
		}
	// Critical Section

	mutex.Signal();
	mutex.Close();
	return renamed;
	}

TBool Processes::IsRunning(const TDesC& exeName)
	{
	TFileName procName;
	procName.Copy(exeName);
	procName.Trim();
	procName.Append(_L("*"));

	TFindProcess processFinder(procName);
	TFullName fullName;
	if (processFinder.Next(fullName) == KErrNone)
		{
		return ETrue;
		}
	return EFalse;
	}

TBool Processes::IsNativeRecRunning()
	{
	//const TUid KScreensaverUidIn3rdEd = {0x100056cf};
	const TUid KNativeRecUidIn5thEd = {0x100058ca};
	TBool running(EFalse);
	
	RWsSession windowSession;
	CleanupClosePushL(windowSession);
	TInt err = windowSession.Connect();
	if(err == KErrNone)
		{
		TApaTaskList apataskList( windowSession);
		TApaTask apatask = apataskList.FindByPos(0);  //retrieve the foremost app
			 
		TInt wgId = apatask.WgId();
		CApaWindowGroupName* wgName=CApaWindowGroupName::NewL(windowSession);
		CleanupStack::PushL(wgName);
		wgName->ConstructFromWgIdL(wgId);
		TUid fgUid = wgName->AppUid();
		if(fgUid == KNativeRecUidIn5thEd )
			{
			running = ETrue;
			}
		CleanupStack::PopAndDestroy(wgName);
		windowSession.Close();
		}
	CleanupStack::PopAndDestroy(&windowSession);
		
	return running;
	}

