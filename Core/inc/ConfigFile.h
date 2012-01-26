#ifndef CONFIGDATA_H
#define CONFIGDATA_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <s32strm.h>
#include <f32file.h>

#include <HT\logging.h>
#include <HT\SharedQueueCliSrv.h>

#include "Json.h"



/**
 * This class is only used to contain Agent's informations retrieved from the Config
 */
class CDataAgent : public CBase
	{
public:
	static CDataAgent* NewL(TAgentType aId, TAgentStatus aStatus, const TDesC8& aParams);
	static CDataAgent* NewLC(TAgentType aId, TAgentStatus aStatus, const TDesC8& aParams);

	virtual ~CDataAgent();

private:
	CDataAgent(TAgentType aId, TAgentStatus aStatus);
	void ConstructL(const TDesC8& aParams);

public:
	TAgentType iId;
	TAgentStatus iStatus;
	RBuf8 iParams;
	};

/**
 * This class is only used to contain Action's informations retrieved from the Config
 */
class CDataAction : public CBase
	{
public:
	static CDataAction* NewL(TActionType aId, const TDesC8& aParams);
	static CDataAction* NewLC(TActionType aId, const TDesC8& aParams);

	virtual ~CDataAction();

private:
	CDataAction(TActionType aId);
	void ConstructL(const TDesC8& aParams);

public:
	TActionType iId;
	RBuf8 iParams;
	};

/**
 * This class is only used to contain Action's informations retrieved from the Config
 */
class CDataMacroAction : public CBase
	{
public:
	static CDataMacroAction* NewL();
	static CDataMacroAction* NewLC();

	virtual ~CDataMacroAction();

	// Ownership is Transferred
	void AppendAction(CDataAction* aAction);
	
private:
	void ConstructL();

public:
	RPointerArray<CDataAction> iActionsList;
	TInt iQueueId;   // primary queue if macro contains sync or uninstall actions, secondary otherwise
	};

/**
 * This class is only used to contain Action's informations retrieved from the Config
 */
class CDataOption : public CBase
	{
public:
	static CDataOption* NewL(TOptionType aId, const TDesC8& aParams);
	static CDataOption* NewLC(TOptionType aId, const TDesC8& aParams);

	virtual ~CDataOption();

private:
	CDataOption(TOptionType aId);
	void ConstructL(const TDesC8& aParams);

public:
	TOptionType iId;
	RBuf8 iParams;
	};

/**
 * This class is only used to contain Action's informations retrieved from the Config
 */
class CDataEvent : public CBase
	{
public:
	static CDataEvent* NewL(TEventType aId, TUint32 aMacroActionIdx, const TDesC8& aParams);
	static CDataEvent* NewLC(TEventType aId, TUint32 aMacroActionIdx, const TDesC8& aParams);

	virtual ~CDataEvent();

private:
	CDataEvent(TEventType aId, TUint32 aMacroIdx);
	void ConstructL(const TDesC8& aParams);

public:
	TEventType iId;
	RBuf8 iParams;
	TUint32 iMacroActionIdx;
	};

class CConfigFile : public CBase
	{
public:
	// Constructors and destructor

	~CConfigFile();
	static CConfigFile* NewL();
	static CConfigFile* NewLC();

	TBool LoadL(RFs& fs, const TDesC& filename);

	CDataAgent* FindDataAgent(TAgentType aAgentId);
	TInt GetModuleId(CJsonObject* aObject);   
		

private:
	void ReadModulesSectionL(CJsonArray* aModulesArray);  
	void ReadEventsSectionL(const CJsonArray* aEventsArray);  
	void ReadActionsSectionL(CJsonArray* aActionsArray); 
	TInt GetEventId(const CJsonObject* aObject);    
	TInt GetActionId(CJsonObject* aObject); 

	HBufC8* DecryptConfigFileL(RFs& fs, const TDesC& filename);
	
	void LogInfoInvalidL(RFs& fs,const TDesC8& aLogMsg);

	CConfigFile();
	void ConstructL();
	void Clear();

public:
	RPointerArray<CDataAgent> iAgentsList;
	RPointerArray<CDataEvent> iEventsList;
	RPointerArray<CDataOption> iOptionsList;
	RPointerArray<CDataMacroAction> iMacroActionsList;

	__FLOG_DECLARATION_MEMBER
	};

#endif // Data_H
