/* SPDX-License-Identifier: MIT; original Luminary source basic transfers. */
#include "lm_super_dispatch.h"
enum exec_status lm_super_dispatch(apollo_overlay_state *s,p66_bias_io *io,exec_transfer *next){agc_word loc,bank;uint16_t pc;
 if(!s||!io||!next)return EXEC_OUT_OF_SCOPE;
 pc=s->words[5];if((pc!=05165u&&pc!=05166u)||s->words[3]>03400u||(s->words[3]&0377u)||(s->words[4]&~076000u)||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||s->words[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 {uint16_t a=s->words[0],l=s->words[1];if(a!=agc_extend(agc_correct(a))||l!=agc_extend(agc_correct(l)))return EXEC_OUT_OF_SCOPE;loc=agc_correct(pc==05165u?a:l);bank=agc_correct(pc==05165u?l:a);}
 if(loc<0100u||loc>07777u||(bank&~076167u))return EXEC_OUT_OF_SCOPE;
 s->words[0]=agc_extend(bank);s->words[1]=agc_extend(loc);s->words[2]=2;s->words[3]=(agc_word)((bank&7u)*0400u);s->words[4]=(agc_word)(bank&076000u);s->words[6]=(agc_word)(bank&076007u);s->words[5]=loc;io->superbank=(agc_word)(bank&0160u);next->interpretive=0;next->address=(exec_address){loc,bank};return EXEC_OK;
}
