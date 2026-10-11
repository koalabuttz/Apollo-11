/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_TRIG_H
#define GUIDANCE_TRIG_H
#include "guidance_norm.h"
typedef struct {agc_dp buf2;agc_word polish,tem2,tem3,cduspot[6],sin[6],cos[6];} p66_trig_scratch;
/* Original double-precision Hastings SIN/COS, half-amplitude output, angle
 * in revolutions. MODE+0/-1 retained; unsupported MODE rejects unchanged.
 * CDULOGIC converts one 15-bit two's-complement CDU count in MPAC0 to a DP
 * one's-complement revolution fraction. Third word/MODE retained by logic.
 * All objects ordinary-word/exclusively owned and producer exclusion needed.
 * Numeric pointer/return debris is modeled, with no native control dispatch. */
uint8_t p66_sine(p66_preparation *,p66_scale_scratch *,p66_divide_scratch *,p66_norm_scratch *,p66_trig_scratch *);
uint8_t p66_cosine(p66_preparation *,p66_scale_scratch *,p66_divide_scratch *,p66_norm_scratch *,p66_trig_scratch *);
uint8_t p66_cdu_logic(p66_preparation *);
/* Source CDUTRIGS, live stable hardware words X/Y/Z. Preserves vectorMPAC
 * except+2; writes angle/trig arrays in Y/Z/X order. basic_return is the
 * historical caller's 12-bit Q word retained in TEM2, not a native return.
 * basic_bankset is bank23 plus three EB bits (046000..046007); EXIT stores
 * it in VBUF4 while restoring the original caller bank. */
uint8_t p66_cdu_trig(p66_preparation *,p66_scale_scratch *,p66_divide_scratch *,p66_norm_scratch *,p66_trig_scratch *,const agc_word cdu_xyz[3],agc_word basic_return,agc_word basic_bankset);
#endif
