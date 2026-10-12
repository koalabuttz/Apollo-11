/* SPDX-License-Identifier: MIT */
#ifndef AGC_IMAGE_OPS_H
#define AGC_IMAGE_OPS_H
#include "agc_multiword.h"
#include "overlay_runtime.h"
/* Register effects of single AGC basic instructions on the canonical
 * 2048-word image (words 0/1 raw 16-bit A/L, Q held 15-bit, EB/FB/BB
 * coupled, 020-023 edited, ZERO fixed at +0). These are helpers for
 * compiled native translations of fixed source sequences: the caller names
 * each instruction and its effective erasable address (INDEX already
 * applied) at compile time. There is no instruction word, decoder or
 * program counter here; Z, fixed memory and I/O channels are never
 * operands. Operands in fixed memory are passed by value through the *f
 * forms. Every helper ends with L overflow-corrected and sign-extended, as
 * the end of every original instruction does.
 *
 * Erasable addresses are 0-01777 as the instruction sees them; 01400-01777
 * select the EB window. Skip results: TS/OVSK return 1 when the next
 * instruction is skipped; CCS returns 0 (+), 1 (+0), 2 (-), 3 (-0). */
typedef apollo_overlay_state agc_image;

unsigned ai_ea(const agc_image *, unsigned address);
agc_word ai_word(const agc_image *, unsigned address);
/* Plain 15-bit store with the editing/bank effects of an assignment. */
void ai_put(agc_image *, unsigned address, agc_word);
void ai_ca(agc_image *, unsigned);
void ai_caf(agc_image *, agc_word);
void ai_cs(agc_image *, unsigned);
void ai_csf(agc_image *, agc_word);
void ai_ad(agc_image *, unsigned);
void ai_adf(agc_image *, agc_word);
void ai_su(agc_image *, unsigned);
void ai_suf(agc_image *, agc_word);
void ai_mask(agc_image *, unsigned);
void ai_maskf(agc_image *, agc_word);
uint8_t ai_ts(agc_image *, unsigned);
uint8_t ai_ovsk(const agc_image *);
void ai_xch(agc_image *, unsigned);
void ai_lxch(agc_image *, unsigned);
void ai_qxch(agc_image *, unsigned);
void ai_ads(agc_image *, unsigned);
void ai_incr(agc_image *, unsigned);
void ai_aug(agc_image *, unsigned);
void ai_dim(agc_image *, unsigned);
/* Double-word forms name the high word K (the source operand). */
void ai_das(agc_image *, unsigned);
void ai_dca(agc_image *, unsigned);
void ai_dcaf(agc_image *, const agc_word[2]);
void ai_dcs(agc_image *, unsigned);
void ai_dcsf(agc_image *, const agc_word[2]);
void ai_dxch(agc_image *, unsigned);
void ai_mp(agc_image *, unsigned);
void ai_mpf(agc_image *, agc_word);
void ai_dv(agc_image *, unsigned);
void ai_dvf(agc_image *, agc_word);
uint8_t ai_ccs(agc_image *, unsigned);
uint8_t ai_ccsf(agc_image *, agc_word);
/* BZF/BZMF outcome on the current A (1 = branch taken). */
uint8_t ai_bzf(const agc_image *);
uint8_t ai_bzmf(const agc_image *);
/* TC: Q = return address (a CADR within the current bank). */
void ai_tc(agc_image *, agc_word return_address);
#define ai_double(s) ai_ad((s), 0u)
#define ai_com(s) ai_cs((s), 0u)
#define ai_dcom(s) ai_dcs((s), 0u)
#define ai_ddoubl(s) ai_das((s), 0u)
#define ai_square(s) ai_mp((s), 0u)
#define ai_zl(s) ai_lxch((s), 7u)
#define ai_zq(s) ai_qxch((s), 7u)
#endif
