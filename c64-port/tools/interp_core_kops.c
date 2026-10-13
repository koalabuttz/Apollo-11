/* SPDX-License-Identifier: MIT */
/* Host stand-ins for the resident kernel's image ops (ABI 2 services
 * k_ca ... k_bzmf), for the compact interpreter built with INTERP_K_OPS
 * off the C64: each is agc_image_ops on the image the core last bound, as
 * the resident's k_<op> is ai_<op> on the canonical image. Lets the fuzz
 * check the flight module's op mix against the verification interpreter. */
#include "agc_image_ops.h"
apollo_overlay_state *interp_core_k_image;
#define S interp_core_k_image
#define KOP(n,T) void k_##n(T a); void k_##n(T a){ai_##n(S,a);}
#define KOPS(n,T) uint8_t k_##n(T a); uint8_t k_##n(T a){return ai_##n(S,a);}
KOP(ca,unsigned) KOP(caf,agc_word) KOP(cs,unsigned) KOP(csf,agc_word) KOP(ad,unsigned) KOP(adf,agc_word)
KOP(maskf,agc_word) KOPS(ts,unsigned) KOP(xch,unsigned) KOP(lxch,unsigned) KOP(ads,unsigned) KOP(dxch,unsigned)
KOPS(ccs,unsigned) KOP(tc,agc_word) KOP(dcaf,const agc_word *)
/* (ABI 2.4: INTERP_K_OPS2) */
KOP(mask,unsigned) KOP(incr,unsigned) KOP(das,unsigned) KOP(dcsf,const agc_word *)
uint8_t k_bzf(void); uint8_t k_bzf(void){return ai_bzf(S);}
uint8_t k_bzmf(void); uint8_t k_bzmf(void){return ai_bzmf(S);}
