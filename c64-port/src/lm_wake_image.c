/* SPDX-License-Identifier: MIT; original Luminary099 JOBWAKE. */
#include "lm_wake_image.h"
enum exec_status lm_wake_image(apollo_overlay_state *s,p66_bias_io *io){
 agc_word *w,wake,oldl,oldfb,caller,savedbank=0,newloc,newbank;
 agc_register difference;uint8_t i,slot=8,candidate;uint16_t base;
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 w=s->words;wake=agc_correct(w[0]);oldl=agc_correct(w[1]);oldfb=w[4];caller=w[2];
 if(w[5]!=05137u||w[0]!=agc_extend(wake)||caller<0100u||caller>07777u||
    w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||
    w[6]!=(agc_word)(w[4]|(w[3]>>8))||w[7]||
    (io->superbank&~0160u)||io->interrupt_enabled>1u||
    (w[067]!=AGC_NEG_ZERO&&(w[067]>84u||w[067]%12u)))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<8u;++i){agc_word priority=w[0167u+12u*i],loc=w[0164u+12u*i];
  if(priority==0)return EXEC_OUT_OF_SCOPE;
  difference=agc_add_register(agc_extend(agc_negate(wake)),agc_extend(loc));
  if(slot==8u&&agc_classify(priority)==AGC_NEGATIVE&&
     (difference==0||difference==0177777u)){slot=i;savedbank=w[0165u+12u*i];}}
 if(slot<8u&&(savedbank&~0167u))return EXEC_OUT_OF_SCOPE;
 if(slot==0&&w[067]==0)return EXEC_OUT_OF_SCOPE; /* literal CCSHOLE */
 w[065]=wake;w[061]=oldfb;w[062]=(agc_word)(slot<8u?7u-slot:0);
 io->interrupt_enabled=0;
 if(slot==8u){w[064]=077776;w[0]=agc_extend(oldfb);w[1]=agc_extend(oldl);
  w[2]=06743;w[5]=caller;return EXEC_NOT_FOUND;}
 w[064]=(agc_word)(12u*slot);base=(uint16_t)(0154u+12u*slot);
 w[063]=agc_negate(w[base+11u]);w[base+11u]=w[063];
 newloc=agc_add_word((agc_word)(wake&01777u),02000);
 newbank=agc_add_word((agc_word)(wake&076000u),savedbank);
 w[base+8u]=newloc;w[base+9u]=newbank;w[066]=oldl;
 if(slot==0&&w[067]==AGC_NEG_ZERO){w[067]=0;w[065]=0;}
 else{
  w[065]=slot==0?agc_diminished_abs(w[067]):(agc_word)(w[064]-1u);
  candidate=w[067]==AGC_NEG_ZERO?0:(uint8_t)(w[067]/12u);
  difference=agc_add_register(agc_extend(agc_negate(w[0167u+12u*candidate])),agc_extend(w[063]));
  if(difference!=0&&difference!=0177777u&&!(difference&0100000u))w[067]=w[064];
 }
 w[0]=agc_extend(oldfb);w[1]=agc_extend(savedbank);w[2]=06743;w[5]=caller;
 return EXEC_OK;
}
