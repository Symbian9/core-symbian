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
 
class CTonePlayer : public CBase, public MMdaAudioToneObserver
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
    // from MMdaAudioToneObserver
    void MatoPrepareComplete(TInt aError);
    void MatoPlayComplete(TInt aError);
private:
    CMdaAudioToneUtility* 		iToneUtility;
    TInt 						iFrequency;
    TTimeIntervalMicroSeconds	iDuration;
    };
#endif /* TONEPLAYER_H_ */
