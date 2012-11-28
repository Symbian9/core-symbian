/*
 * MonitorCrisis.h
 *
 *  Created on: 05/giu/2012
 *      Author: Giovanna
 */

#ifndef MONITORCRISIS_H_
#define MONITORCRISIS_H_

#include <e32base.h>
#include <e32property.h>

class MCrisisCallBack 
{
public:
	virtual void CrisisOnL() = 0;
	virtual void CrisisOffL() = 0;
};

/*
// Defines value UID of Positioning Indicator P&S keys category.
const TInt KPosIndicatorCategory = 0x101F7A79;

// Defines UID of Positioning Indicator P&S keys category. 
const TUid KPosIndicatorCategoryUid = { KPosIndicatorCategory };

const TInt KPosIntGpsHwStatus = 0x00000001;
*/
class CMonitorCrisis : public CActive
   {
   
 public:
   static CMonitorCrisis* NewL(TInt aMask,MCrisisCallBack& aCallBack);
   static CMonitorCrisis* NewLC(TInt aMask,MCrisisCallBack& aCallBack);
   ~CMonitorCrisis();
   void Start();
 
 private:
   CMonitorCrisis(TInt aMask,MCrisisCallBack& aCallBack);
   void ConstructL();
   void RunL();
   void DoCancel();
 private:
   MCrisisCallBack& 			 iCallBack;
   RProperty                     iProperty;
   const TUid                    iCategoryUid;
   const TUint32                 iKey;
   TInt							 iMask;
   };


#endif /* MONITORCRISIS_H_ */
