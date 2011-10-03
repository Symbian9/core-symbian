/*
 * TonePlayer.h
 *
 *  Created on: 30/set/2011
 *      Author: Giovanna
 */

#ifndef TONEPLAYER_H_
#define TONEPLAYER_H_

#include <e32std.h>
#include <MdaAudioTonePlayer.h>
#include <HT\TimeOutTimer.h> 
 
class CTonePlayer : public CBase, public MMdaAudioToneObserver, public MTimeOutNotifier
    {
public:
    static CTonePlayer* NewL();
    static CTonePlayer* NewLC();
    ~CTonePlayer();
public:
	void Play();
    void Stop();
private:
    CTonePlayer();
    void ConstructL();
    // From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    // from MMdaAudioToneObserver
    void MatoPrepareComplete(TInt aError);
    void MatoPlayComplete(TInt aError);
private:
    CMdaAudioToneUtility* 		iToneUtility;
    TInt 						iFrequency;
    TTimeIntervalMicroSeconds	iDuration;
    CTimeOutTimer*				iTimer;
    };
#endif /* TONEPLAYER_H_ */
