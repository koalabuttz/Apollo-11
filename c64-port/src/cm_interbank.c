/* SPDX-License-Identifier: MIT; original CM interbank communication. */
#include "cm_interbank.h"
#include "fixed_data.h"
static const agc_word entries[14]={04662,04666,04675,04701,04704,04711,04715,04740,04744,04753,04757,04773,04707,04710};
static void fb(apollo_overlay_state *s,agc_word v){s->words[4]=(agc_word)(v&076000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));}
enum exec_status cm_interbank(apollo_overlay_state *s,cm_native_io *io,enum cm_interbank_entry entry,agc_word literal){agc_word oldfb,cadr,q,base,value=0,bank;agc_register olda,oldl;
 if(s==0||io==0||(unsigned)entry>=14u||s->words[5]!=entries[entry]||s->words[7]!=0||s->words[2]>07775u||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||s->words[3]>03400u||(s->words[3]&0377u)||(s->words[4]&~076000u)||(io->superbank&~0160u)||io->interrupt_enabled>1u||agc_overflow(s->words[0]))return EXEC_OUT_OF_SCOPE;
 q=s->words[2];oldfb=s->words[4];olda=s->words[0];oldl=s->words[1];base=(agc_word)(entry>=CM_IB_IBNKCALL&&entry<=CM_IB_ISWRETURN?072:0133);cadr=literal;
 if(entry==CM_IB_SWCALL||entry==CM_IB_BANKJUMP||entry==CM_IB_ISWCALL||entry==CM_IB_SUPDACAL)cadr=agc_correct(olda);
 if(entry==CM_IB_SUPDACAL){bank=(agc_word)(cadr>>10);if(bank>=030u&&(agc_correct(oldl)&0100u))bank=(agc_word)(bank+8u);if(!apollo_fixed_data_read(APOLLO_FIXED_CM,(uint16_t)(bank*1024u+(cadr&01777u)),&value))return EXEC_OUT_OF_SCOPE;}
 if(entry==CM_IB_SWRETURN||entry==CM_IB_ISWRETURN){if(s->words[base]>07777u||(s->words[base+1u]&~076000u))return EXEC_OUT_OF_SCOPE;fb(s,s->words[base+1u]);s->words[base+1u]=oldfb;s->words[2]=(agc_word)(base+1u);s->words[5]=s->words[base];return EXEC_OK;}
 if(entry==CM_IB_BANKCALL||entry==CM_IB_SWCALL||entry==CM_IB_IBNKCALL||entry==CM_IB_ISWCALL){if(literal>AGC_MASK)return EXEC_OUT_OF_SCOPE;if(entry==CM_IB_BANKCALL||entry==CM_IB_IBNKCALL){s->words[base]=agc_correct(olda);s->words[base+1u]=agc_correct(oldl);q=agc_add_word(q,1);}s->words[0]=agc_extend(s->words[base]);s->words[1]=agc_extend(s->words[base+1u]);s->words[base]=q;s->words[base+1u]=oldfb;fb(s,cadr);s->words[2]=entry==CM_IB_BANKCALL||entry==CM_IB_SWCALL?04675:04753;s->words[5]=(agc_word)(02000u+(cadr&01777u));return EXEC_OK;}
 if(entry==CM_IB_POSTJUMP||entry==CM_IB_BANKJUMP){if(literal>AGC_MASK)return EXEC_OUT_OF_SCOPE;fb(s,cadr);s->words[0]=entry==CM_IB_POSTJUMP?olda:agc_extend(q);s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);return EXEC_OK;}
 if(entry==CM_IB_MAKECADR){s->words[0]=agc_add_register(agc_extend((agc_word)(s->words[0133]&01777u)),agc_extend(s->words[0134]));s->words[5]=q;s->words[2]=3;return EXEC_OK;}
 if(entry==CM_IB_SUPDACAL){s->words[0135]=value;s->words[0]=agc_extend(value);s->words[1]=(agc_word)(cadr&01777u);s->words[5]=q;s->words[2]=3;io->interrupt_enabled=1;return EXEC_OK;}
 if(entry==CM_IB_USPRCADR){if(literal>AGC_MASK)return EXEC_OUT_OF_SCOPE;s->words[0164]=q;s->words[0165]=s->words[6];s->words[023]=1;fb(s,cadr);s->words[2]=(agc_word)(cadr&01777u);s->words[5]=(agc_word)(02000u+s->words[2]);return EXEC_OK;}
 if(entry==CM_IB_SUPERSW){io->superbank=(agc_word)(agc_correct(olda)&0160u);s->words[5]=q;s->words[2]=3;return EXEC_OK;}
 if(entry==CM_IB_QJUMP){if(q>01777u)return EXEC_OUT_OF_SCOPE;s->words[5]=(agc_word)(02000u+q);return EXEC_OK;}
 s->words[5]=02000;return EXEC_OK;}
