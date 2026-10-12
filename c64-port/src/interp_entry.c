/* SPDX-License-Identifier: MIT */
#include "interp_entry.h"
bool interp_entry(apollo_overlay_state *s,unsigned cm,enum interp_entry_point e,agc_word *ie){agc_word *w,x,base;
 if(!s||!ie||*ie>1u||cm>1u||(unsigned)e>2u)return false;
 w=s->words;base=(agc_word)(cm?INTERP_ENTRY_CM_INTPRET:INTERP_ENTRY_LM_INTPRET);
 if(w[7]!=0||(w[3]&~03400u)!=0||(w[4]&~076000u)!=0||w[6]!=(agc_word)(w[4]|(w[3]>>8)))return false;
 if(w[5]!=(agc_word)(base+(e==INTERP_INTPRET?0u:e==INTERP_INTPRET2?2u:011u)))return false;
 /* An interpretive branch's LOC is in its bank (or erasable): no producer branches to fixed-fixed. */
 if(e==INTERP_INTPRET2&&agc_correct(w[1])>03777u)return false;
 if(e==INTERP_INTPRET){*ie=1;x=w[2];w[2]=agc_extend(w[0164]);w[0164]=agc_correct(x);w[1]=agc_extend(agc_correct(w[1]));} /* RELINT; QXCH LOC */
 else if(e==INTERP_INTPRET2){x=w[1];w[1]=agc_extend(w[0164]);w[0164]=agc_correct(x);}         /* LXCH LOC */
 else{x=agc_correct(w[1]);w[1]=agc_extend(w[6]);w[6]=(agc_word)(x&076007u);w[3]=(agc_word)((x&7u)<<8);w[4]=(agc_word)(x&076000u);} /* LXCH BBANK */
 w[0165]=w[6];w[0115]=(agc_word)(w[6]&040000u);w[023]=(agc_word)((w[0115]>>7)&0177u);w[0]=agc_extend(w[0115]); /* INTPRET +3; EDOP keeps bits 14-8 */
 w[5]=(agc_word)(base+031u);                                                                    /* TCF NEWOPS */
 return true;
}
