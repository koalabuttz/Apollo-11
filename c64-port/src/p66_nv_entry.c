/* SPDX-License-Identifier: MIT */
#include "p66_nv_entry.h"
#include "agc_word.h"
enum p66_nv_boundary p66_nv_enter(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words;uint16_t busy=(uint16_t)((w[01021]&020000u)+w[01012]);
 if(w[5]!=04155u||w[0]!=01474u||w[1]!=0||w[2]!=03121u||w[3]!=03400u||w[4]!=020000u||w[6]!=020007u||w[7]!=0||w[01012]>7u||io->superbank>060u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_NV_OUT_OF_SCOPE;
 w[0123]=01474;
 if(busy){w[0]=(uint16_t)(busy-1u);w[2]=3;w[5]=03121;return P66_NV_BUSY;}
 w[01037]=03122;w[1]=agc_extend(w[01022]);w[01022]=0;w[01021]=040000;
 w[01040]=(uint16_t)(020007u|io->superbank);w[3]=00400;w[4]=062000;w[6]=062001;io->superbank=0100;
 w[0136]=04202;w[023]=0;w[0157]=60;w[0160]=6;w[0161]=w[0156];w[0]=6;w[2]=03602;w[5]=02337;
 return P66_NV_VERB;
}
