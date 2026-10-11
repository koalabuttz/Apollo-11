/* SPDX-License-Identifier: MIT */
#include "cm_mysubs.h"
static uint8_t entry(const apollo_overlay_state *m,agc_word pc){return (uint8_t)(m->words[5]==pc&&m->words[4]==042000u&&m->words[3]==03000u&&m->words[6]==042006u&&m->words[2]<=07777u&&m->words[7]==0);}
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static void commit(apollo_overlay_state *m,agc_register_pair r,agc_dp k,agc_word pc){m->words[0]=r.a;m->words[1]=r.l;m->words[03110]=k.high;m->words[03111]=k.low;m->words[5]=pc;}
enum cm_mysubs_status cm_smallmp(apollo_overlay_state *m){agc_register_pair r;agc_dp k;agc_word saved,low;int ov;if(!entry(m,02042))return CM_MYSUBS_OUT_OF_SCOPE;saved=agc_correct(m->words[0]);ov=agc_overflow(m->words[0]);m->words[03112]=saved;
 /* An initially overflowed TS skips EXTEND. The compiled source alternate
  * path at2044 therefore performs its original unextended MASK operand. */
 if(ov){agc_register indicator=ov>0?1u:0177776u;low=agc_correct((agc_register)(indicator&agc_extend(m->words[03111])));}else{r=agc_multiply(m->words[0],m->words[03111]);low=agc_correct(r.a);}
 r=agc_multiply(agc_extend(m->words[03110]),saved);k.high=0;k.low=low;agc_double_add(&r,&k);commit(m,r,k,02053);return CM_MYSUBS_OK;}
/* DPADD begins immediately with DAS. A deliberately raw overflowed L has
 * not yet undergone an earlier source instruction's end-of-cycle correction.
 * Compute that first pair sum from its raw registers before normalizing. */
static void angle_sum(agc_register_pair *r,agc_dp *k)
{
    agc_register low=add(r->l,k->low),high=add(r->a,k->high);
    int carry=agc_overflow(low),overflow;
    if(carry>0)high=agc_add_register(high,1);
    else if(carry<0)high=agc_add_register(high,0177776u);
    overflow=agc_overflow(high);
    k->high=agc_correct(high);k->low=agc_correct(low);
    r->a=overflow>0?1u:overflow<0?0177776u:0;
    r->l=0;
}
enum cm_mysubs_status cm_dpadd(apollo_overlay_state *m){agc_register_pair r;agc_dp k;agc_register a;enum agc_ccs_class hi,lo;if(!entry(m,02054))return CM_MYSUBS_OUT_OF_SCOPE;r.a=m->words[0];r.l=m->words[1];k.high=m->words[03110];k.low=m->words[03111];angle_sum(&r,&k);
 if(r.a){hi=agc_classify(k.high);a=agc_extend(agc_diminished_abs(k.high));if(hi==AGC_POSITIVE)a=add(a,040000);else if(hi==AGC_NEGATIVE)a=add((agc_register)(a^0177777u),037777);else{lo=agc_classify(k.low);a=agc_extend(agc_diminished_abs(k.low));if(lo==AGC_POSITIVE){a=add(a,040000);k.low=agc_correct(a);a=agc_extend(040000);}else{if(lo==AGC_NEGATIVE)a=(agc_register)(a^0177777u);a=add(a,037777);k.low=agc_correct(a);a=agc_extend(037777);}}k.high=agc_correct(a);r.a=a;}
 commit(m,r,k,02073);return CM_MYSUBS_OK;}
enum cm_mysubs_status cm_mysubs_return(apollo_overlay_state *m){agc_word returned;if((!entry(m,02053)&&!entry(m,02073))||m->words[2]<02000u)return CM_MYSUBS_OUT_OF_SCOPE;returned=m->words[2];m->words[1]=agc_extend(agc_correct(m->words[1]));m->words[5]=returned;m->words[2]=3;return CM_MYSUBS_OK;}
