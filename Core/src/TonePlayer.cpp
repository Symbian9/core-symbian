/*
 * TonePlayer.cpp
 *
 *  Created on: 30/set/2011
 *      Author: Giovanna
 */

#include "TonePlayer.h"
#include <MdaAudioTonePlayer.h>
#include <eikmenup.h>
 

CTonePlayer* CTonePlayer::NewL()
	{
    CTonePlayer* self = NewLC();
    CleanupStack::Pop(self);  
    return self;
	}
 
CTonePlayer* CTonePlayer::NewLC()
	{
    CTonePlayer* self = new (ELeave) CTonePlayer();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
	}
 
CTonePlayer::CTonePlayer(): iFrequency(600 /*440*/),iDuration(10000)
	{
	}
 
CTonePlayer::~CTonePlayer()
	{
	delete iTimer;
	Stop();
    delete iToneUtility;
	}
 
void CTonePlayer::ConstructL()
	{
	iTimer = CTimeOutTimer::NewL(*this);
    iToneUtility = CMdaAudioToneUtility::NewL(*this);
    iToneUtility->PrepareToPlayTone(iFrequency,iDuration);
	}
 
void CTonePlayer::Play()
	{	
	iToneUtility->Play();
	}
 
void CTonePlayer::Stop()
	{
	iTimer->Cancel();
	iToneUtility->CancelPlay();
	}
 
void CTonePlayer::MatoPrepareComplete(TInt /*aError*/)
	{
	iToneUtility->SetVolume(iToneUtility->MaxVolume());
	}
 
void CTonePlayer::MatoPlayComplete(TInt /*aError*/)
	{
	TTime time;
	time.HomeTime();
	time += TTimeIntervalSeconds(5);        
	iTimer->RcsAt(time);
	}

void CTonePlayer::TimerExpiredL(TAny* src)
	{
	iToneUtility->CancelPlay();
	iToneUtility->Play();
	}
