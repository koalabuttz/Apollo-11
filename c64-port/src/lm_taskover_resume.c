/* SPDX-License-Identifier: MIT */
#include "lm_taskover_resume.h"
enum exec_status lm_taskover_resume(apollo_overlay_state *s,p66_bias_io *io,uint16_t *in_interrupt){
 uint16_t *w,bank,old_bank;agc_word arupt,lr,qr;
 if(!s||!io||!in_interrupt)return EXEC_OUT_OF_SCOPE;
 w=s->words;bank=w[016];
 if(w[5]!=05261u||w[0734]!=AGC_NEG_ZERO||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||(bank&(agc_word)~076167u)||w[015]==0||w[015]>010000u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||*in_interrupt!=1)return EXEC_OUT_OF_SCOPE;
 old_bank=w[6];arupt=w[010];lr=w[011];qr=w[012];
 io->superbank=(agc_word)(bank&0160u);w[012]=agc_correct(w[2]);w[2]=agc_extend(qr);
 w[010]=old_bank;w[011]=agc_correct(w[1]);w[0]=agc_extend(arupt);w[1]=agc_extend(lr);
 w[6]=(agc_word)(bank&076007u);w[3]=(agc_word)((bank&7u)*0400u);w[4]=(agc_word)(bank&076000u);w[5]=(agc_word)(w[015]-1u);w[7]=0;
 io->interrupt_enabled=1;*in_interrupt=0;return EXEC_OK;
}
