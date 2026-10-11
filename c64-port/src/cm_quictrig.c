/* SPDX-License-Identifier: MIT */
#include "cm_quictrig.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register indicator(int sign){return sign>0?1:0177776;}
static void double_pair(agc_register_pair *r){agc_register lo=agc_add_register(r->l,r->l),hi=agc_add_register(r->a,r->a);int carry=agc_overflow(lo);if(carry)hi=agc_add_register(hi,indicator(carry));r->a=hi;r->l=agc_extend(agc_correct(lo));}
/* Exact native polynomial and quadrant reduction. Memory operands are the
 * original Comanche source constants, not approximate mathematical values. */
static void polynomial(apollo_overlay_state *m,uint8_t cosine){agc_register a=m->words[0];agc_register_pair r={a,agc_extend(agc_correct(m->words[1]))};int ov;agc_word limit;uint8_t extreme=0;
 if(cosine)a=add(a,020000);
 m->words[01075]=agc_correct(a);ov=agc_overflow(a);if(ov)a=agc_extend(agc_negate(m->words[01075]));
 a=agc_add_register(a,a);m->words[01075]=agc_correct(a);ov=agc_overflow(a);
 if(ov){a=agc_extend(m->words[01075]);m->words[01075]=agc_correct(indicator(ov));limit=ov>0?040000:037777;a=add(a,limit);a=(agc_register)(a^0177777u);a=add(a,m->words[01075]);m->words[01075]=agc_correct(a);ov=agc_overflow(a);if(ov){a=indicator(ov);extreme=1;}}
 if(!extreme){r=agc_multiply(a,m->words[01075]);m->words[01076]=agc_correct(r.a);r=agc_multiply(r.a,01124);r.a=add(r.a,065552);r=agc_multiply(r.a,m->words[01076]);r.a=add(r.a,031103);r=agc_multiply(r.a,m->words[01075]);double_pair(&r);a=r.a;m->words[01075]=agc_correct(a);ov=agc_overflow(a);if(ov){a=indicator(ov);extreme=1;}}
 if(extreme){limit=agc_to_int(agc_correct(a))>0?040000:037777;a=agc_extend(agc_negate(limit));m->words[5]=05133;}else m->words[5]=05130;
 m->words[0]=a;m->words[1]=r.l;
}
enum cm_trig_status cm_single_trig(apollo_overlay_state *m,uint8_t cosine){if(cosine>1u||m->words[5]!=(cosine?05074u:05075u)||m->words[2]>07777u||m->words[7]!=0)return CM_TRIG_OUT_OF_SCOPE;polynomial(m,cosine);return CM_TRIG_OK;}
enum cm_trig_status cm_quictrig(apollo_overlay_state *m,agc_word *irq){uint8_t offset;agc_register_pair r;agc_word old_hi,old_lo;if(*irq>1u||m->words[5]!=03555||m->words[4]!=046000||m->words[3]>03400u||(m->words[3]&0377u)!=0||m->words[6]!=(046000u|(m->words[3]>>8))||m->words[2]>07777u||m->words[7]!=0)return CM_TRIG_OUT_OF_SCOPE;
 *irq=0;m->words[061]=m->words[2];
 for(offset=4;;offset=(uint8_t)(offset-2u)){m->words[062]=offset;m->words[0]=agc_extend(m->words[0766u+offset]);polynomial(m,0);r=agc_multiply(m->words[0],020000);old_hi=m->words[0736u+offset];old_lo=m->words[0737u+offset];m->words[0736u+offset]=agc_correct(r.a);m->words[0737u+offset]=agc_correct(r.l);m->words[0]=agc_extend(old_hi);m->words[1]=agc_extend(old_lo);
  m->words[0]=agc_extend(m->words[0766u+offset]);polynomial(m,1);r=agc_multiply(m->words[0],020000);old_hi=m->words[0744u+offset];old_lo=m->words[0745u+offset];m->words[0744u+offset]=agc_correct(r.a);m->words[0745u+offset]=agc_correct(r.l);m->words[0]=agc_extend(old_hi);m->words[1]=agc_extend(old_lo);if(!offset)break;
 }
 m->words[0]=agc_extend(m->words[061]);m->words[2]=3;m->words[5]=03605;*irq=1;return CM_TRIG_OK;
}
