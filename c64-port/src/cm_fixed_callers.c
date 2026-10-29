/* SPDX-License-Identifier: MIT */
#include "cm_fixed_callers.h"
static int banks(const apollo_overlay_state *s,const cm_native_io *io){return s&&io&&s->words[7]==0&&s->words[3]<=03400u&&(s->words[3]&0377u)==0&&(s->words[4]&~076000u)==0&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&(io->superbank&~0160u)==0&&io->interrupt_enabled<=1u;}
enum exec_status cm_fixed_caller_enter(apollo_overlay_state *s,cm_native_io *io,uint8_t selfcheck){agc_word bank;
 if(!banks(s,io)||selfcheck>1u||(io->superbank&0100u)==0)return EXEC_OUT_OF_SCOPE;
 bank=selfcheck?066000:062000;
 if(s->words[4]!=bank||s->words[5]!=(selfcheck?03535u:03353u))return EXEC_OUT_OF_SCOPE;
 if(selfcheck){s->words[1]=agc_extend(s->words[01374]);s->words[0]=agc_add_register(agc_extend((agc_word)(s->words[01374]&076000u)),agc_extend(s->words[01373]));s->words[2]=03542;}
 else{s->words[0777]=016;s->words[1]=agc_extend(s->words[01047]);s->words[0]=agc_extend(s->words[01017]);s->words[2]=03361;}
 s->words[5]=04715;return EXEC_OK;
}
enum exec_status cm_supdacal_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){agc_word cadr,bank;
 if(!banks(s,io)||!out||s->words[5]!=04715u||agc_overflow(s->words[0])||s->words[2]>07775u)return EXEC_OUT_OF_SCOPE;
 cadr=agc_correct(s->words[0]);bank=(agc_word)(cadr>>10);
 if(bank>=030u&&(s->words[1]&0100u))bank=(agc_word)(bank+8u);
 *out=(uint16_t)(bank*1024u+(cadr&01777u));return EXEC_OK;
}
enum exec_status cm_supdacal_data(apollo_overlay_state *s,cm_native_io *io,agc_word word){uint16_t address;agc_word q;
 if(word>AGC_MASK||cm_supdacal_address(s,io,&address)!=EXEC_OK)return EXEC_OUT_OF_SCOPE;
 (void)address;q=s->words[2];s->words[0135]=word;s->words[1]=(agc_word)(agc_correct(s->words[0])&01777u);s->words[0]=agc_extend(word);s->words[5]=q;s->words[2]=3;io->interrupt_enabled=1;return EXEC_OK;
}
