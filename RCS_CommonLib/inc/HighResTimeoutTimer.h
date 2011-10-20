/*
 * HighResTimeoutTimer.h
 *
 *  Created on: 30/ago/2011
 *      Author: Giovanna
 */

#ifndef HIGHRESTIMEOUTTIMER_H_
#define HIGHRESTIMEOUTTIMER_H_

// INCLUDES
#include <e32base.h>

class MHighResTimeoutNotifier
    {
    public: // New functions

        /**
        * HighResTimerExpiredL.
        * The function to be called when a timeout occurs.
        */
        virtual void HighResTimerExpiredL(TAny* src) = 0;
    };

// CLASS DECLARATION
/**
* CHighResTimeoutTimer
*  This class will notify an object after a specified high resolution timeout.
*/
class CHighResTimeoutTimer : public CTimer
    {
    public: // Constructors and destructors

        /** 
        * NewL.
        * Two-phased constructor.
        * Creates a CHighResTimeOutTimer object using two phase construction,
        * and returns a pointer to the created object.
        * @param aPriority Priority to use for this timer.
        * @param aTimeOutNotify Object to notify of timeout event.
        * @return A pointer to the created instance of CTimeOutTimer.
        */
        static CHighResTimeoutTimer* NewL(MHighResTimeoutNotifier& aHRTimeOutNotify, const TInt aPriority=EPriorityStandard );

        /**
        * NewLC.
        * Two-phased constructor.
        * Creates a CHighResTimeOutTimer object using two phase construction,
        * and returns a pointer to the created object.
        * @param aPriority Priority to use for this timer.
        * @param aTimeOutNotify Object to notify of timeout event.
        * @return A pointer to the created instance of CTimeOutTimer.
        */
        static CHighResTimeoutTimer* NewLC(MHighResTimeoutNotifier& aTimeOutNotify, const TInt aPriority=EPriorityStandard );

        /**
        * ~CHighResTimeoutTimer.
        * Destructor.
        * Destroys the object and release all memory objects.
        */
        virtual ~CHighResTimeoutTimer();
        
        
        void RcsHighRes(TTimeIntervalMicroSeconds32 aInterval);

    protected: // Functions from base classes

        /**
        * From CActive, RunL.
        * Called when operation completes.
        */
        void RunL();

    private: // Constructors and destructors

        /**
        * CHighResTimeoutTimer.
        * C++ default constructor.
        * Performs the first phase of two phase construction.
        * @param aPriority Priority to use for this timer.
        * @param aTimeOutNotify An observer to notify.
        */
        CHighResTimeoutTimer( MHighResTimeoutNotifier& aTimeOutNotify, const TInt aPriority );

        /**
        * ConstructL.
        * 2nd phase constructor.
        */
        void ConstructL();

    private: // Data

        /**
        * iNotify, the observer for this objects events.
        */
        MHighResTimeoutNotifier& iNotify;
        TTimeIntervalMicroSeconds32 iInterval;
        
    };


#endif /* HIGHRESTIMEOUTTIMER_H_ */
