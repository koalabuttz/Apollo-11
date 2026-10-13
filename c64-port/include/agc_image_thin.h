/* SPDX-License-Identifier: MIT */
#ifndef AGC_IMAGE_THIN_H
#define AGC_IMAGE_THIN_H
#include "agc_image_ops.h"
/* One-argument wrappers around the agc_image_ops helpers, for compiled
 * translations built for the flight image (the thin kernel's idiom). Each
 * call site is an operand load and a JSR: the image is the one canonical
 * image (at APOLLO_SHARED_ADDRESS with RK_FIXED_IMAGE, else bound at entry
 * with AIT_BIND), an operand below 0400 is passed in one byte, and an
 * EB-window operand (01400-01777) as its byte offset in the window. The
 * wrappers only forward, so a translation built with them has exactly the
 * effects of the same source built on agc_image_ops. */
#if RK_FIXED_IMAGE
#define AIT_S ((agc_image *)APOLLO_SHARED_ADDRESS)
#define AIT_BIND(x) ((void)(x))
#else
static agc_image *ait_s;
#define AIT_S ait_s
#define AIT_BIND(x) ((void)(ait_s=(agc_image *)(x)))
#endif
#define AIT_NI static __attribute__((noinline,unused))
#define AIT_OP(n) \
 AIT_NI void ait_##n(unsigned a){(ai_##n)(AIT_S,a);} \
 AIT_NI void ait_##n##8(uint8_t a){(ai_##n)(AIT_S,a);} \
 AIT_NI void ait_##n##e(uint8_t a){(ai_##n)(AIT_S,01400u+a);}
#define AIT_OPS(n) \
 AIT_NI uint8_t ait_##n(unsigned a){return (ai_##n)(AIT_S,a);} \
 AIT_NI uint8_t ait_##n##8(uint8_t a){return (ai_##n)(AIT_S,a);} \
 AIT_NI uint8_t ait_##n##e(uint8_t a){return (ai_##n)(AIT_S,01400u+a);}
#define AIT_OPF(n) \
 AIT_NI void ait_##n(agc_word k){(ai_##n)(AIT_S,k);} \
 AIT_NI void ait_##n##8(uint8_t k){(ai_##n)(AIT_S,k);} \
 AIT_NI void ait_##n##e(uint8_t k){(ai_##n)(AIT_S,(agc_word)(01400u+k));}
AIT_OP(ca) AIT_OP(cs) AIT_OP(ad) AIT_OP(su) AIT_OP(mask) AIT_OP(xch) AIT_OP(lxch) AIT_OP(qxch) AIT_OP(ads)
AIT_OP(incr) AIT_OP(aug) AIT_OP(dim) AIT_OP(das) AIT_OP(dca) AIT_OP(dcs) AIT_OP(dxch) AIT_OP(mp) AIT_OP(dv)
AIT_OPS(ts) AIT_OPS(ccs)
AIT_OPF(caf) AIT_OPF(csf) AIT_OPF(adf) AIT_OPF(suf) AIT_OPF(maskf) AIT_OPF(mpf) AIT_OPF(dvf) AIT_OPF(tc)
AIT_NI void ait_dcaf(const agc_word *k){ai_dcaf(AIT_S,k);}
AIT_NI void ait_dcsf(const agc_word *k){ai_dcsf(AIT_S,k);}
AIT_NI uint8_t ait_bzf(void){return ai_bzf(AIT_S);}
AIT_NI uint8_t ait_bzmf(void){return ai_bzmf(AIT_S);}
/* The narrowest wrapper for a constant operand. */
#define AIT_ARG(n,a) (__builtin_constant_p(a)&&(unsigned)(a)>=01400u&&(unsigned)(a)<02000u?ait_##n##e((uint8_t)((a)-01400u)): \
 __builtin_constant_p(a)&&(unsigned)(a)<0400u?ait_##n##8((uint8_t)(a)):ait_##n(a))
/* AIT_REDIRECT: route the ai_* calls of the including translation through
 * the wrappers (s is still evaluated, for the parameter it names). */
#define AIT_REDIRECT
#endif
