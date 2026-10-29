/* SPDX-License-Identifier: MIT */
#include "lm_idle_start.h"
enum exec_status lm_idle_start(apollo_overlay_state *image,p66_bias_io *io,agc_word *channel11,exec_address *next){
 uint16_t *w,location,bank;agc_word old_l;
 if(!image||!io||!channel11||!next)return EXEC_OUT_OF_SCOPE;
 w=image->words;location=w[0164];bank=w[0165];
 if(w[5]!=03214u||w[4]!=02000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(02000u|(w[3]>>8))||w[7]!=0||w[067]!=0||w[0167]==0||w[0167]>=040000u||location==0||location>07777u||(bank&(agc_word)~076167u)||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled!=1u||*channel11>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 old_l=agc_correct(w[1]);*channel11=(agc_word)(*channel11|2u);
 w[0164]=*channel11;w[0165]=old_l;w[0]=agc_extend(bank);w[1]=location;w[2]=2;w[5]=location;
 w[6]=(agc_word)(bank&076007u);w[3]=(agc_word)((bank&7u)*0400u);w[4]=(agc_word)(bank&076000u);io->superbank=(agc_word)(bank&0160u);
 next->location=location;next->bank=bank;return EXEC_OK;
}
