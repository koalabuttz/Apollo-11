/* SPDX-License-Identifier: MIT */
#include "cm_rcs_matrix.h"
#include "cm_single_return.h"
static void trig(apollo_overlay_state *m,agc_word argument,uint8_t cosine,agc_word q){m->words[0]=agc_extend(argument);m->words[2]=q;m->words[5]=cosine?05074u:05075u;(void)cm_single_trig(m,cosine);(void)cm_single_return(m);}
/* MSU's modulo-32768 subtraction is two's-complement first; a negative
 * result is diminished once to encode its one's-complement source word.
 * The caller's COM then obtains the CDU-minus-quad argument. */
static agc_word quad_argument(agc_word cdu){agc_word delta=(agc_word)((01224u+agc_negate(cdu)+1u)&AGC_MASK);if(delta&AGC_SIGN)--delta;return agc_negate(delta);}
static void store(apollo_overlay_state *m,unsigned address){m->words[address]=agc_correct(m->words[0]);}
static void multiply(apollo_overlay_state *m,agc_word factor){agc_register_pair r=agc_multiply(m->words[0],factor);m->words[0]=r.a;m->words[1]=r.l;}
enum cm_rcs_matrix_status cm_rcs_matrix(apollo_overlay_state *m){agc_word flag;if(m->words[5]!=03447u||m->words[3]!=03000u||m->words[4]!=044000u||m->words[6]!=044006u||m->words[7]!=0)return CM_RCS_MATRIX_OUT_OF_SCOPE;m->words[1]=agc_extend(agc_correct(m->words[1]));flag=m->words[0102];m->words[0]=agc_extend(flag);if(agc_to_int(flag)<=0)goto end;m->words[0]=(agc_register)(flag&020000u);if(!m->words[0])goto end;trig(m,m->words[034],0,03457);store(m,03240);trig(m,m->words[034],1,03462);store(m,03245);trig(m,quad_argument(m->words[032]),1,03470);store(m,03244);multiply(m,m->words[03245]);store(m,03241);trig(m,quad_argument(m->words[032]),0,03501);store(m,03242);multiply(m,m->words[03245]);m->words[0]=(agc_register)(m->words[0]^0177777u);store(m,03243);
end:m->words[5]=05217;return CM_RCS_MATRIX_OK;}
uint8_t cm_rcs_matrix_constant_read(uint16_t address,uint16_t *out){if(out==0||address!=19271u)return 0;*out=01224;return 1;}
