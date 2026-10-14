/* SPDX-License-Identifier: MIT */
#ifndef AGC_FAST_WINDOW_H
#define AGC_FAST_WINDOW_H
#include "agc_fast_ops.h"
/* One-byte forms of the fast helpers for cells in the EBANK 6 window
 * (01400-01776 under EB 6, the DAP's erasable). A call site passes the
 * cell's offset in the window as one immediate byte, where the afo_* form
 * takes the cell's address: with an address at every site, a long routine
 * keeps its 20-odd cell addresses live across every call and spills them
 * (a third of TJETLAW). Each afw_* helper is the afo_* helper of the same
 * name on that cell, so the effects are the same; the cost is one call.
 *
 * AFW(n,eb,a) is the afw_* call when a is a constant in the window and the
 * caller's constant EB is 6 (the rule of agc_fast_ops.h: it must be the
 * image's EB at the site), else the AFO(n,eb,a) choice. */
#define AFW_EBN 6u
#define AFW_IN(eb,a) (__builtin_constant_p(a)&&(eb)==AFW_EBN&&(unsigned)(a)>=01400u&&(unsigned)(a)<01777u)
#define AFW_OFF(a) ((uint8_t)((unsigned)(a)-01400u))
void afw_ca(uint8_t);
void afw_cs(uint8_t);
void afw_ad(uint8_t);
void afw_su(uint8_t);
void afw_mask(uint8_t);
uint8_t afw_ts(uint8_t);
void afw_xch(uint8_t);
void afw_lxch(uint8_t);
void afw_qxch(uint8_t);
void afw_ads(uint8_t);
void afw_incr(uint8_t);
void afw_aug(uint8_t);
void afw_dim(uint8_t);
void afw_das(uint8_t);
void afw_dca(uint8_t);
void afw_dcs(uint8_t);
void afw_dxch(uint8_t);
void afw_mp(uint8_t);
void afw_dv(uint8_t);
uint8_t afw_ccs(uint8_t);
#endif
