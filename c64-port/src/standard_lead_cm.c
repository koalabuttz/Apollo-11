/* SPDX-License-Identifier: MIT */
#include "standard_lead_cm.h"
enum exec_status standard_lead_cm_test(apollo_overlay_state *s,cm_native_io *io,enum standard_lead_cm_entry entry){
 agc_word *w,oldbank,oldl,entryl;cm_executive e;uint8_t slot;enum exec_status status;
 if(!s||!io||(unsigned)entry>2u)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if(w[5]!=(agc_word)(02000u+(unsigned)entry)||w[4]!=066000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||w[7]!=0||(io->superbank&~060u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(entry!=STANDARD_LEAD_REDO){w[1]=agc_extend(agc_correct(w[1]));w[2]=(agc_word)(02001u+(unsigned)entry);w[5]=entry==STANDARD_LEAD_COMPVER?03436:03131;return EXEC_OK;}
 if(io->executive_alarm||((w[067]!=AGC_NEG_ZERO)&&(w[067]>72u||w[067]%12u))||((w[0167]==AGC_NEG_ZERO)&&(w[067]!=AGC_NEG_ZERO)))return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);
 for(slot=0;slot<7u;++slot)if(e.core[slot][CM_EXEC_PRIORITY]==0||e.core[slot][CM_EXEC_PRIORITY]>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 for(slot=0;slot<7u;++slot)if(e.core[slot][CM_EXEC_PRIORITY]==AGC_NEG_ZERO)break;
 oldbank=slot<7u?e.core[slot][CM_EXEC_BANK]:0;oldl=w[066];entryl=agc_correct(w[1]);
 w[01011]=7;w[070]=02004;w[071]=w[6];
 status=cm_exec_admit(&e,030000,03441,060102,0);
 if(status!=EXEC_OK&&status!=EXEC_NO_CORE)return status;
 cm_native_commit_executive(s,io,&e);
 if(status==EXEC_OK){w[061]=020000;w[062]=(agc_word)(6u-slot);w[070]=020000;w[071]=oldbank;w[0]=02451;w[1]=(agc_word)(020000u|(entryl&7u));w[2]=06712;w[5]=02004;io->interrupt_enabled=1;}
 else{w[061]=020000;w[062]=0;w[0]=0;w[1]=agc_extend(oldl);w[2]=02724;w[3]=(agc_word)((entryl&7u)<<8);w[4]=02000;w[5]=07755;w[6]=(agc_word)(02000u|(entryl&7u));io->interrupt_enabled=0;}
 return status;
}
