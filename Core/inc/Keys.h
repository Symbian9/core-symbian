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
_LIT8(KVERSION,"\x27\xea\xdd\x77");		//2011032103

_LIT8(KIV, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

// Backdoor Id
#ifdef _DEBUG
//_LIT8(KBACKDOORID, "RCS_0000000317\x00\x00");    //backdoor su prod N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000005\x00\x00");    //backdoor su preprod N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000012\x00\x00");    //backdoor su preprod ExternalPartner
_LIT8(KBACKDOORID, "RCS_0000000076\x00\x00");    //backdoor su preprod E71
//_LIT8(KBACKDOORID, "RCS_0000000475\x00\x00");    //backdoor su prod E61
//_LIT8(KBACKDOORID, "RCS_0000000063\x00\x00");    //backdoor su preprod E61
//_LIT8(KBACKDOORID, "RCS_0000000453\x00\x00");    //backdoor su prod N97Mini UNINSTALL
//_LIT8(KBACKDOORID, "RCS_0000000135\x00\x00");    //backdoor su prod E72
//_LIT8(KBACKDOORID, "RCS_0000000455\x00\x00");    //backdoor su prod E71
#else
_LIT8(KBACKDOORID, "av3pVck1gb4eR2\x00\x00");
#endif

// ConfKey
#ifdef _DEBUG
//_LIT8(KAES_CONFIG_KEY,"2aea9bec25ee2bfc8b4ab86d24a4d7b8"); //RCS_0000000317  //N97mini prod
//_LIT8(KAES_CONFIG_KEY,"b22d3cd68cf1f376c86855823d0852a0"); //RCS_0000000005  //N97mini preprod
//_LIT8(KAES_CONFIG_KEY,"abce2a78b54420e9a33d70c212fe3d83"); //RCS_0000000012  //ExternalPartner preprod
_LIT8(KAES_CONFIG_KEY,"bffb2b51e298730bc6806de8de4869b9"); //RCS_0000000076  //E71 preprod
//_LIT8(KAES_CONFIG_KEY,"34e5e13c98d14d2317c4f52c513af286"); //RCS_0000000475  //E61 prod
//_LIT8(KAES_CONFIG_KEY,"71d6ccbcab12055099149ce3f5d752d9"); //RCS_0000000063  //E61 preprod
//_LIT8(KAES_CONFIG_KEY,"28745df4927a0fcca3c40abae83c3881"); //RCS_0000000135  //E72 prod
//_LIT8(KAES_CONFIG_KEY,"7cddcb1b75b6f702a21c4f917ea7c201"); //RCS_0000000455  //E71 prod
//_LIT8(KAES_CONFIG_KEY,"13f49991883eae5ee4c68424e7e5660e"); //RCS_0000000453  //N97mini prod UNINSTALL
#else
_LIT8(KAES_CONFIG_KEY, "Adf5V57gQtyi90wUhpb8Neg56756j87R");
#endif

//Challenge key or Signature, the same for all bkdoors on the same server
#ifdef _DEBUG
//_LIT8(KAES_CHALLENGE_KEY, "572ebc94391281ccf53a851330bb0d99"); //RCS_0000000317 //N97Mini prod
_LIT8(KAES_CHALLENGE_KEY, "96778af9756a30489f6435bb0655eec2"); //RCS_0000000012 //ExternalPartner preprod
//_LIT8(KAES_CHALLENGE_KEY, "96778af9756a30489f6435bb0655eec2"); //RCS_0000000063 //E61 preprod
#else
_LIT8(KAES_CHALLENGE_KEY, "f7Hk0f5usd04apdvqw13F5ed25soV5eD");
#endif

//Log key
#ifdef _DEBUG
//_LIT8(KAES_LOGS_KEY, "95dac559b0edfffc543b06b46c43e610");  //RCS_0000000317  //N97Mini su prod
//_LIT8(KAES_LOGS_KEY, "a875373040a90f4a8d8dfc00ef4cc645");  //RCS_0000000005  //N97Mini su preprod
//_LIT8(KAES_LOGS_KEY, "0640685495f69a6d250951f9e707ed1b");  //RCS_0000000012  //ExternalPartner su preprod
_LIT8(KAES_LOGS_KEY, "656b3dff1c1efa25e8348639e76a7973");  //RCS_0000000076  //E71 su preprod
//_LIT8(KAES_LOGS_KEY, "1b50df977cef743ea28f019bde34c727");  //RCS_0000000012  //E61 su prod
//_LIT8(KAES_LOGS_KEY, "afb16f3085c03382929057fd804ee9c9");  //RCS_0000000063  //E61 su preprod
//_LIT8(KAES_LOGS_KEY, "d4078fdf640118f158792a39cf70bab4");  //RCS_0000000135  //E72 su prod
//_LIT8(KAES_LOGS_KEY, "597cd89c7e39d395438113da1c879940");  //RCS_0000000455  //E71 su prod
//_LIT8(KAES_LOGS_KEY, "36cf605bf75fd11edde58b6ec98997e6");  //RCS_0000000453  //N97Mini su prod UNINSTALL
//_LIT8(KAES_LOGS_KEY, "96db502bbee4cd75f09a7aa9d3c6d659");  //RCS_0000000246  //Montalbano
//_LIT8(KAES_LOGS_KEY, "02ab00be97a461fedf9ceeacb0ef73f3");  //RCS_0000000125  //N96
//_LIT8(KAES_LOGS_KEY, "d4078fdf640118f158792a39cf70bab4");  //RCS_0000000135  //E72
//_LIT8(KAES_LOGS_KEY, "87f8398188d816f74657188b7e2a1370");  //RCS_0000000205  //N81
#else
_LIT8(KAES_LOGS_KEY, "3j9WmmDgBqyU270FTid3719g64bP4s52");
#endif


#endif /* KEYS_H_ */
