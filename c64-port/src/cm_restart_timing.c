/* SPDX-License-Identifier: MIT */
#include "cm_restart_timing.h"
#include "agc_multiword.h"
static agc_register com(agc_register a){return (agc_register)(a^0177777u);}
static enum agc_ccs_class regclass(agc_register a){
 int ov=agc_overflow(a);
 return ov>0?AGC_POSITIVE:ov<0?AGC_NEGATIVE:agc_classify(agc_correct(a));
}
static agc_register dabs(agc_register a){
 agc_register m=(a&0100000u)?com(a):a;
 return m?(agc_register)(m-1u):0;
}
static agc_register plus(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static void pairadd(apollo_overlay_state *s,agc_register_pair *r){
 agc_dp p={s->words[01137],s->words[01140]};agc_double_add(r,&p);
 s->words[01137]=p.high;s->words[01140]=p.low;
}
static int bankvalid(const apollo_overlay_state *s){
 return s!=0&&s->words[6]==02003&&s->words[3]==01400&&s->words[4]==02000;
}
enum cm_restart_time_exit cm_restart_wait_delay(apollo_overlay_state *s,uint8_t group){
 agc_register a;enum agc_ccs_class kind;
 if(group>=6||!bankvalid(s))return CM_RSTIME_INVALID;
 a=com(s->words[0]);s->words[1]=a;
 a=agc_add_register(agc_extend(agc_negate(s->words[01053+group*2u])),agc_extend(agc_negate(s->words[025])));
 kind=regclass(a);a=dabs(a);
 if(kind==AGC_POSITIVE)a=com(a);
 if(kind==AGC_POSITIVE||kind==AGC_PLUS_ZERO)a=plus(a,037776);
 if(kind!=AGC_MINUS_ZERO)a=plus(a,1);
 a=agc_add_register(a,s->words[1]);kind=regclass(a);a=dabs(a);
 if(kind==AGC_POSITIVE)a=0;
 s->words[0]=plus(a,1);s->words[2]=03614;s->words[5]=0704;
 return CM_RSTIME_WAITLIST;
}
enum cm_restart_time_exit cm_restart_long_delay(apollo_overlay_state *s){
 agc_register_pair r;agc_word high,low;enum agc_ccs_class kind;
 if(!bankvalid(s)||s->words[1]!=agc_extend(agc_correct(s->words[1])))return CM_RSTIME_INVALID;
 s->words[01137]=agc_correct(s->words[0]);s->words[01140]=agc_correct(s->words[1]);
 r.a=agc_extend(agc_negate(s->words[024]));r.l=agc_extend(agc_negate(s->words[025]));pairadd(s,&r);
 r.a=agc_extend(s->words[01135]);r.l=agc_extend(s->words[01136]);pairadd(s,&r);
 high=s->words[01137];low=s->words[01140];kind=agc_classify(high);
 if(kind==AGC_POSITIVE||((kind==AGC_PLUS_ZERO||kind==AGC_MINUS_ZERO)&&agc_classify(low)==AGC_POSITIVE)){
  s->words[0704]=05342;s->words[0]=agc_extend(high);s->words[1]=agc_extend(low);
  s->words[2]=03726;s->words[5]=0704;return CM_RSTIME_LONGCALL;
 }
 /* Negative high/low and either signed zero share source immediate branches.
  * At exactly -0/-0 IMEDIATE adds one to the preceding CCS diminished zero. */
 s->words[0]=1;s->words[1]=0;s->words[2]=03614;s->words[5]=0704;
 return CM_RSTIME_WAITLIST;
}
