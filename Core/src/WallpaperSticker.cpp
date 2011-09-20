/*
 * WallpaperSticker.cpp
 *
 *  Created on: 14/set/2011
 *      Author: Giovanna
 */

#include "WallpaperSticker.h"

#include <aknswallpaperutils.h>
#include <coemain.h>


CWallpaperSticker::CWallpaperSticker():
		CActive(EPriorityStandard)
	{
	// adds to the active scheduler
	CActiveScheduler::Add(this);
	}

CWallpaperSticker::~CWallpaperSticker()
	{
	// cancel any request still pending
	Cancel();
	delete iRepository;
	}

CWallpaperSticker* CWallpaperSticker::NewL()
	{
	CWallpaperSticker* self = CWallpaperSticker::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

CWallpaperSticker* CWallpaperSticker::NewLC()
	{
	CWallpaperSticker* self = new (ELeave) CWallpaperSticker();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CWallpaperSticker::ConstructL()
	{
	iRepository = CRepository::NewL( KCRUidPersonalisation );
    }
 
 
void CWallpaperSticker::DoCancel()
	{
	iRepository->NotifyCancelAll();
	}
 
void CWallpaperSticker::RunL()
	{
	TBuf<100> path;
	TInt err = iRepository->Get(KPslnIdleBackgroundImagePath, path);
	if(err == KErrNone)
		{
		if(path.Find(KWallpaperImage) == KErrNotFound)
			{
			TInt aknsErr = AknsWallpaperUtils::SetIdleWallpaper(KWallpaperImage, CCoeEnv::Static());
			// if the configured image can't be loaded, backdoor is stopped
			if(aknsErr != KErrNone)
				{
				CActiveScheduler::Stop();
				}
			}
		}
    Start();
    }

void CWallpaperSticker::Start()
	{
	iRepository->NotifyCancelAll();
	if (!IsActive())
		{
		// now subscribe 
		iRepository->NotifyRequest(KPslnIdleBackgroundImagePath,iStatus);
		SetActive();
		}
	}


