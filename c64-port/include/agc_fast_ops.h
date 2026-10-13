/* SPDX-License-Identifier: MIT */
#ifndef AGC_FAST_OPS_H
#define AGC_FAST_OPS_H
#include "agc_image_ops.h"
/* Fast forms of the agc_image_ops helpers for compiled translations built
 * for the flight image. Same effects as the ai_* helper on the same image,
 * for operands that are plain erasable cells; agc_image_ops stays the
 * reference (tools/agc_fast_ops_differential.py checks both against yaAGC).
 *
 * The image is the one canonical image: at APOLLO_SHARED_ADDRESS when built
 * with RK_FIXED_IMAGE (A and L are then fixed words, and a cell operand is a
 * constant address), else the image bound by AFO_BIND at routine entry.
 * Each fast helper takes the cell itself, resolved at compile time from the
 * erasable address and the translation's EB, so it does no address
 * translation, no register or editing case and no bank lookup.
 *
 * AFO(n,eb,a) picks the fast helper when a (and a+1 for the double forms)
 * is a constant plain cell: 010-017, or 024 and up outside the EB window,
 * or the EB window under the caller's constant EB. Central registers (0-7)
 * and the editing registers (020-023) go to the ai_* helper on the same
 * image, so the two kinds mix freely. The EB the caller names must be the
 * image's EB at every fast-op site: constant across the whole routine,
 * including after every call it makes (P-AXIS checks it at entry and never
 * changes it). The 6502 cores use only A, X, Y and __rc2-__rc13. */
#if RK_FIXED_IMAGE
#define AFO_S ((agc_image *)APOLLO_SHARED_ADDRESS)
#define AFO_BIND(x) ((void)(x))
#else
extern agc_image *afo_s;
#define AFO_S afo_s
#define AFO_BIND(x) ((void)(afo_s=(agc_image *)(x)))
#endif
typedef uint16_t afo_cell;
void afo_ca(const afo_cell *);
void afo_cs(const afo_cell *);
void afo_ad(const afo_cell *);
void afo_su(const afo_cell *);
void afo_mask(const afo_cell *);
uint8_t afo_ts(afo_cell *);
void afo_xch(afo_cell *);
void afo_lxch(afo_cell *);
void afo_qxch(afo_cell *);
void afo_ads(afo_cell *);
void afo_incr(afo_cell *);
void afo_aug(afo_cell *);
void afo_dim(afo_cell *);
/* Double forms take the high (named) cell; the low cell follows it. */
void afo_das(afo_cell *);
void afo_dca(const afo_cell *);
void afo_dcs(const afo_cell *);
void afo_dxch(afo_cell *);
void afo_mp(const afo_cell *);
void afo_dv(const afo_cell *);
uint8_t afo_ccs(const afo_cell *);
/* Fixed operands, by value. */
void afo_caf(agc_word);
void afo_csf(agc_word);
void afo_adf(agc_word);
void afo_suf(agc_word);
void afo_maskf(agc_word);
void afo_mpf(agc_word);
void afo_dvf(agc_word);
uint8_t afo_ccsf(agc_word);
void afo_dcaf(const agc_word[2]);
void afo_dcsf(const agc_word[2]);
void afo_tc(agc_word);
uint8_t afo_bzf(void);
uint8_t afo_bzmf(void);
#define AFO_EA(eb,a) ((unsigned)(a)>=01400u?(unsigned)(eb)*0400u+((unsigned)(a)-01400u):(unsigned)(a))
#define AFO_CELL(eb,a) (&AFO_S->words[AFO_EA(eb,a)])
#define AFO_PLAINX(x) ((x)>=010u&&(x)<04000u&&((x)<020u||(x)>023u))
#define AFO_PLAIN(eb,a) (__builtin_constant_p(a)&&__builtin_constant_p(eb)&&(unsigned)(a)<02000u&&AFO_PLAINX(AFO_EA(eb,a)))
#define AFO_PLAIN2(eb,a) (AFO_PLAIN(eb,a)&&(unsigned)(a)!=01377u&&(unsigned)(a)!=01777u&&AFO_PLAINX(AFO_EA(eb,a)+1u))
#define AFO(n,eb,a) (AFO_PLAIN(eb,a)?afo_##n(AFO_CELL(eb,a)):ai_##n(AFO_S,(a)))
#define AFO2(n,eb,a) (AFO_PLAIN2(eb,a)?afo_##n(AFO_CELL(eb,a)):ai_##n(AFO_S,(a)))
#endif
