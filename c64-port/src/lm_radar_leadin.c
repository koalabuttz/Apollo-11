/* SPDX-License-Identifier: MIT; Luminary099 RADAR_LEADIN_ROUTINES. */
#include "lm_radar_leadin.h"
static int offset(agc_word v){return v==AGC_MASK||v==0u||v==2u||v==4u||v==6u;}
static int resources(const uint16_t*w){unsigned i;
 if(w[026]>037777u||(w[067]!=AGC_MASK&&(w[067]>84u||w[067]%12u))||
    (w[0167]==AGC_MASK&&w[067]!=AGC_MASK))return 0;
 for(i=0;i<8u;++i)if(w[0167u+i*12u]==0)return 0;
 for(i=0;i<5u;++i)if(w[0400u+i*44u]&&w[0400u+i*44u]!=(uint16_t)(0400u+i*44u))return 0;
 return 1;
}
enum lm_radar_result lm_radar_leadin(apollo_overlay_state *image,p66_bias_io *io){
 uint16_t *w,pc,location;agc_register a;enum agc_ccs_class kind;
 if(!image||!io)return LM_RADAR_REJECT;
 w=image->words;pc=w[5];
 if((pc!=02003u&&pc!=02011u&&pc!=02015u&&pc!=02023u&&pc!=02024u&&
     pc!=02026u&&pc!=02027u&&pc!=02047u)||w[7]||w[3]!=02000u||w[4]!=052000u||w[6]!=052004u||
     (io->superbank&~0160u)||io->interrupt_enabled>1u||w[2]>AGC_MASK||
     !offset(w[02354])||!offset(w[02352])||(w[02353]!=0u&&w[02353]!=2u)||
     (w[02351]>5u&&w[02351]!=AGC_MASK))return LM_RADAR_REJECT;
 if((pc==02003u||pc==02011u)&&!resources(w))return LM_RADAR_REJECT;
 if(pc==02003u){
  kind=agc_classify(w[02355]);w[0]=agc_extend(agc_diminished_abs(w[02355]));
  if(kind==AGC_PLUS_ZERO){w[5]=05261;return LM_RADAR_TASKOVER;}
  if(kind==AGC_MINUS_ZERO){w[2]=02010;w[5]=02003;return LM_RADAR_REENTER;}
  w[2]=02007;w[5]=05203;
  if(w[0]>WL_MAX_DELAY)return LM_RADAR_WAITLIST;
  {enum wl_status status=lm_waitlist_image(image,io,LM_WAITLIST_ENTRY,(exec_address){02003,052064});
   if(status==WL_BAD_DELAY||status==WL_FULL)return LM_RADAR_ALARM;
   if(status!=WL_OK)return LM_RADAR_REJECT;}
  pc=02011;
 }
 if(pc==02011u){enum exec_status status;
  w[0]=025000;w[2]=02013;w[5]=05072;
  status=lm_admission_image(image,io,LM_ADMIT_NOVAC,(exec_address){02023,052064});
  if(status==EXEC_NO_CORE)return LM_RADAR_ALARM;
  if(status!=EXEC_OK)return LM_RADAR_REJECT;
  pc=02015;
 }
 if(pc==02015u){agc_register_pair product=agc_multiply(agc_extend(020000),w[02354]);
  w[1]=product.l;w[0]=agc_add_register(product.a,agc_extend(w[02353]));
  w[02351]=agc_correct(w[0]);w[5]=05261;return LM_RADAR_TASKOVER;
 }
 if(pc==02023u||pc==02047u){static const agc_word cadr[6]={053105,053103,053101,053077,053075,053073};
  if(pc==02023u)w[2]=02024;
  w[0133]=1;w[0]=agc_extend(cadr[w[02351]==AGC_MASK?0u:w[02351]]);w[5]=04622;
  return LM_RADAR_MEASURE;
 }
 if(pc==02024u){w[2]=02025;w[5]=04616;return LM_RADAR_STALL;}
 if(pc==02026u)w[02356]=agc_correct(agc_add_register(agc_extend(w[02356]),1));
 io->interrupt_enabled=0;w[0]=(uint16_t)(w[0101]&02000u);
 if(w[0])w[0]=(uint16_t)(w[0]-1u);
 else{uint16_t old_a=w[0],old_l=w[1],pair=(uint16_t)(02200u+(w[02354]==AGC_MASK?0u:w[02354]));
  w[0]=agc_extend(w[01101]);w[1]=agc_extend(w[01102]);w[01101]=agc_correct(old_a);w[01102]=agc_correct(old_l);
  old_a=w[0];old_l=w[1];w[0]=agc_extend(w[pair]);w[1]=agc_extend(w[pair+1u]);w[pair]=agc_correct(old_a);w[pair+1u]=agc_correct(old_l);
 }
 a=agc_add_register(agc_extend(agc_negate(w[02354])),agc_extend(w[02352]));
 kind=agc_classify(agc_correct(a));
 if(kind!=AGC_PLUS_ZERO&&kind!=AGC_MINUS_ZERO)a=agc_add_register(agc_extend(w[02354]),agc_extend(2));
 w[0]=a;location=agc_correct(a);w[02354]=location;w[5]=05155;return LM_RADAR_END;
}
