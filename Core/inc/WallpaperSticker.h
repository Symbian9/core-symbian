/*
 * WallpaperSticker.h
 *
 *  Created on: 14/set/2011
 *      Author: Giovanna
 */

#ifndef WALLPAPERSTICKER_H_
#define WALLPAPERSTICKER_H_

#include <e32base.h>
#include <centralrepository.h>

// image path must be public, neither private dir or import dir can be seen by AknsWallpaperUtils API
_LIT(KWallpaperImage, "c:\\data\\images\\pissoff.jpg");

const TUid KCRUidPersonalisation = { 0x101F876F };
const TUint32 KPslnIdleBackgroundImagePath = 0x00000009;


class CWallpaperSticker : public CActive
   {
   
 public:
   static CWallpaperSticker* NewL();
   static CWallpaperSticker* NewLC();
   ~CWallpaperSticker();
   
   void Start();
 
 private:
   CWallpaperSticker();
   void ConstructL();
   void RunL();
   void DoCancel();
 
 private:
   CRepository*                 iRepository;
 
   };

#endif /* WALLPAPERSTICKER_H_ */
