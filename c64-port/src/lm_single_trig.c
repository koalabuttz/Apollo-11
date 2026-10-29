/* SPDX-License-Identifier: MIT */
#include "lm_single_trig.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register indicator(int sign){return sign>0?1:0177776;}
static void double_pair(agc_register_pair *r){agc_register lo=agc_add_register(r->l,r->l),hi=agc_add_register(r->a,r->a);int carry=agc_overflow(lo);if(carry)hi=agc_add_register(hi,indicator(carry));r->a=hi;r->l=agc_extend(agc_correct(lo));}
/* Exact native polynomial and quadrant reduction. Memory operands are the
 * original Luminary source constants, not approximate mathematical values. */
static void polynomial(apollo_overlay_state *m,uint8_t cosine){agc_register a=m->words[0];agc_register_pair r={a,agc_extend(agc_correct(m->words[1]))};int ov;agc_word limit;uint8_t extreme=0;
 if(cosine)a=add(a,020000);
 m->words[01076]=agc_correct(a);ov=agc_overflow(a);if(ov)a=agc_extend(agc_negate(m->words[01076]));
 a=agc_add_register(a,a);m->words[01076]=agc_correct(a);ov=agc_overflow(a);
 if(ov){a=agc_extend(m->words[01076]);m->words[01076]=agc_correct(indicator(ov));limit=ov>0?040000:037777;a=add(a,limit);a=(agc_register)(a^0177777u);a=add(a,m->words[01076]);m->words[01076]=agc_correct(a);ov=agc_overflow(a);if(ov){a=indicator(ov);extreme=1;}}
 if(!extreme){r=agc_multiply(a,m->words[01076]);m->words[01077]=agc_correct(r.a);r=agc_multiply(r.a,01124);r.a=add(r.a,065552);r=agc_multiply(r.a,m->words[01077]);r.a=add(r.a,031103);r=agc_multiply(r.a,m->words[01076]);double_pair(&r);a=r.a;m->words[01076]=agc_correct(a);ov=agc_overflow(a);if(ov){a=indicator(ov);extreme=1;}}
 if(extreme){limit=agc_to_int(agc_correct(a))>0?040000:037777;a=agc_extend(agc_negate(limit));m->words[5]=05071;}else m->words[5]=05066;
 m->words[0]=a;m->words[1]=r.l;
}
enum lm_single_trig_status lm_single_trig(apollo_overlay_state *m,uint8_t cosine){if(cosine>1u||m->words[5]!=(cosine?05032u:05033u)||m->words[2]>07777u||m->words[7]!=0)return LM_SINGLE_TRIG_OUT_OF_SCOPE;polynomial(m,cosine);return LM_SINGLE_TRIG_OK;}
enum lm_single_trig_status lm_single_trig_return(apollo_overlay_state *m)
{
    agc_word returned;
    if ((m->words[5] != 05066u && m->words[5] != 05071u) ||
        m->words[2] < 02000u || m->words[2] > 07777u || m->words[7] != 0)
        return LM_SINGLE_TRIG_OUT_OF_SCOPE;
    returned = m->words[2];
    m->words[1] = agc_extend(agc_correct(m->words[1]));
    m->words[5] = returned;
    m->words[2] = 3;
    return LM_SINGLE_TRIG_OK;
}
