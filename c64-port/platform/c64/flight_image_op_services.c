/* SPDX-License-Identifier: MIT */
/* Flight modules' image ops through the resident (docs/FLIGHT-STORE.md,
 * runtime sharing): a flight module links with -Wl,--wrap=ai_<op> for each
 * op below, so its calls reach the resident kernel's k_<op> service
 * (src/resident_kernel_body.inc, docs/overlay-v2-services.json) instead of a
 * private copy. k_<op>(a) is ai_<op>(ks,a) on the canonical image at
 * APOLLO_SHARED_ADDRESS, the only image a flight module is ever handed
 * (the frame's state), so the image argument is dropped. Ops the kernel does
 * not wrap (DV, MP, DCA, ...), and those whose export would cost the resident
 * more than its 2 KB floor allows (DAS 716 bytes, INCR, QXCH, MASK, DCSF;
 * measured), stay linked into each module. */
#include "agc_image_ops.h"
#define FS_OP(n,T) void k_##n(T); void __wrap_ai_##n(agc_image *s,T a); \
 void __wrap_ai_##n(agc_image *s,T a){(void)s;k_##n(a);}
#define FS_OPS(n,T) uint8_t k_##n(T); uint8_t __wrap_ai_##n(agc_image *s,T a); \
 uint8_t __wrap_ai_##n(agc_image *s,T a){(void)s;return k_##n(a);}
FS_OP(ca,unsigned) FS_OP(caf,agc_word) FS_OP(cs,unsigned) FS_OP(csf,agc_word) FS_OP(ad,unsigned) FS_OP(adf,agc_word)
FS_OP(maskf,agc_word) FS_OPS(ts,unsigned) FS_OP(xch,unsigned) FS_OP(lxch,unsigned)
FS_OP(ads,unsigned) FS_OP(dxch,unsigned)
FS_OPS(ccs,unsigned) FS_OP(tc,agc_word) FS_OP(dcaf,const agc_word *)
uint8_t k_bzf(void); uint8_t __wrap_ai_bzf(const agc_image *s);
uint8_t __wrap_ai_bzf(const agc_image *s){(void)s;return k_bzf();}
uint8_t k_bzmf(void); uint8_t __wrap_ai_bzmf(const agc_image *s);
uint8_t __wrap_ai_bzmf(const agc_image *s){(void)s;return k_bzmf();}
