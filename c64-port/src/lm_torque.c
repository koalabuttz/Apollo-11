/* SPDX-License-Identifier: MIT
 * Native source control flow and arithmetic, no AGC instruction decoder. */
#include "lm_torque.h"
#include "agc_multiword.h"
/* Original fixed constants, individually matched to the assembled rope. */
enum { minus_100ms=074631, eleven=013, bit10=01000 };
static uint16_t physical(uint16_t bank, uint16_t logical) {
 return (uint16_t)(bank*0400u + logical-01400u);
}
uint16_t lm_torque(apollo_overlay_state *image, lm_torque_io *io) {
 uint16_t *w, bank, axis, tp, jets, temporary, down, i;
 agc_register a, l, q; agc_register_pair product;
 if(image==0 || io==0) return 0;
 w=image->words;
 if(w[3]>03400u || (w[3]&0377u)!=0 || w[4]!=034000u ||
    w[6]!=(uint16_t)(034000u | (w[3]>>8)) || w[5]!=LM_TORQUE_RATELOOP ||
    w[7]!=0 || io->interrupt_enabled!=0 || (io->superbank&~0160u)!=0) return 0;
 for(i=3;i<APOLLO_SHARED_WORDS;++i) if(w[i]>077777u) return 0;
 bank=(uint16_t)(w[3]>>8);
 tp=physical(bank,01524); jets=physical(bank,01521);
 temporary=physical(bank,01737); down=physical(bank,01513);
 l=w[1]; q=w[2];
 for(axis=2;;--axis) {
  uint16_t pulse=w[tp+axis];
  enum agc_ccs_class sign=agc_classify(pulse);
  w[physical(bank,01744)]=axis;
  q=(uint16_t)(axis*2u);
  a=agc_diminished_abs(pulse);
  if(sign!=AGC_PLUS_ZERO) {
   /* -0 skips the AD at 3651, reaching SMALLTJU with +0. */
   if(sign!=AGC_MINUS_ZERO)
    a=agc_add_register(a,agc_extend(LM_TORQUE_MINUS_100MST6));
   if(a==0 || (a&0100000u)!=0) {
    w[tp+axis]=0;
    product=agc_multiply(agc_extend(pulse),eleven);
    a=product.l; l=product.l;
   } else {
    a=agc_extend(sign==AGC_POSITIVE ? LM_TORQUE_MINUS_100MST6 :
                 (uint16_t)(LM_TORQUE_MINUS_100MST6^077777u));
    a=agc_add_register(a,agc_extend(pulse));
    w[tp+axis]=agc_correct(a);
    sign=agc_classify(w[tp+axis]);
    /* Saturated duration has magnitude >=1, so CCS reaches CS/CA. */
    a=agc_extend(sign==AGC_POSITIVE ? (uint16_t)(minus_100ms^077777u) : minus_100ms);
   }
  }
  product=agc_multiply(a,w[jets+axis]);
  a=product.l; l=product.l;
  w[temporary+axis]=agc_correct(a);
  product=agc_multiply(a,bit10);
  a=product.a; l=product.l;
  if(a==0 || (a&0100000u)!=0) { a=(agc_register)(a^0177777u); ++q; }
  a=agc_add_register(a,agc_extend(w[down+q]));
  w[down+q]=agc_correct(a);
  a=agc_diminished_abs(axis);
  if(axis==0) break;
 }
 a=agc_add_register(agc_extend(w[temporary+1u]),agc_extend(w[temporary+2u]));
 product=agc_multiply(a,w[physical(bank,01532)]);
 a=product.a; l=product.l; w[physical(bank,01747)]=agc_correct(a);
 a=agc_add_register(agc_extend((uint16_t)(w[temporary+2u]^077777u)),
                    agc_extend(w[temporary+1u]));
 product=agc_multiply(a,w[physical(bank,01531)]);
 a=product.a; l=product.l; w[physical(bank,01746)]=agc_correct(a);
 w[0]=a; w[1]=l; w[2]=q; w[5]=LM_TORQUE_BACKP;
 return 1;
}
