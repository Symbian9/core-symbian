/*
 * GpsIndicatorObserver.h
 *
 *  Created on: 23/mar/2011
 *      Author: Giovanna
 */

#ifndef GPSINDICATOROBSERVER_H_
#define GPSINDICATOROBSERVER_H_

#include <e32base.h>
#include <e32property.h>

// Defines value UID of Positioning Indicator P&S keys category.
const TInt KPosIndicatorCategory = 0x101F7A79;

// Defines UID of Positioning Indicator P&S keys category. 
const TUid KPosIndicatorCategoryUid = { KPosIndicatorCategory };

const TInt KPosIntGpsHwStatus = 0x00000001;

class CGpsIndicatorRemover : public CActive
   {
   
 public:
   static CGpsIndicatorRemover* NewL( const TUid aCategoryUid, const TUint32 aKey);
   static CGpsIndicatorRemover* NewLC( const TUid aCategoryUid, const TUint32 aKey);
   ~CGpsIndicatorRemover();
   void Start();
 
 private:
   CGpsIndicatorRemover( const TUid aCategoryUid, const TUint32 aKey);
   void ConstructL();
   void RunL();
   void DoCancel();
 private:
   RProperty                     iProperty;
   const TUid                    iCategoryUid;
   const TUint32                 iKey;
   };
#endif /* GPSINDICATOROBSERVER_H_ */
