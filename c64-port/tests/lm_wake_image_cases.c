/* SPDX-License-Identifier: MIT */
#include "lm_wake_image_cases.h"
#include "lm_wake_cases.h"
void lm_wake_image_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 lm_wake_input(s,io,(uint16_t)(n%8192u));
 if(n>=8192u){
  /* An interrupt/task wakes the dormant running slot while idle or queued.
   * Other sleeping matches deliberately compete; source first-slot wins. */
  s->words[0167]=agc_negate((agc_word)(01000u*((n/8u)%31u+1u)+0110u));
  s->words[0164]=agc_correct(s->words[0]);s->words[0165]=(agc_word)((n%8u)|((n/8u)%8u)*020u);
  s->words[067]=n&1u?AGC_NEG_ZERO:(agc_word)(12u*(n%7u+1u));
 }
}
