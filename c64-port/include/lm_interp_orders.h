/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERP_ORDERS_H
#define LM_INTERP_ORDERS_H
#include "agc_multiword.h"
#include "overlay_runtime.h"
#include "p66_bias.h"
/* General compiled Luminary099 interpretive order bodies and address
 * formation paths (companion to lm_interp_steps.h; same contract: canonical
 * image, raw 16-bit A/L, coherent banks, producers excluded by the caller).
 * `k` names the caller's fixed operand data when ADDRWD selects a fixed
 * word (NULL: the live erasable operand at C(ADDRWD)). Bodies end at DANZIG
 * (06061) unless stated. */

/* DIRADRES with the operand word given (STODL/STOVL load address). */
void lmi_diradres(apollo_overlay_state *, agc_word operand, agc_word target);
/* DIRADRES / ADDRESS when the next word is an order pair: PUSHUP. */
void lmi_diradres_pushup(apollo_overlay_state *, agc_word next, agc_word target);
void lmi_address_pushup(apollo_overlay_state *, agc_word next, agc_word target);
/* NEWOPS with any positive store code, or STADR with the complemented store
 * word: the store, then Z at its continuation (DANZIG, DODLOAD 06460,
 * DODLOAD* 06144, DOVLOAD 06463, DOVLOAD* 06466, or 15BITADR for STOTC). */
void lmi_store_code(apollo_overlay_state *, agc_word code);
void lmi_stadr(apollo_overlay_state *, agc_word word);
/* DODLOAD/DOVLOAD: load code into CYR, ending at DIRADRES 06106. */
void lmi_dodload(apollo_overlay_state *);
void lmi_dovload(apollo_overlay_state *);
void lmi_dload_op(apollo_overlay_state *, const agc_word *k);
void lmi_vload(apollo_overlay_state *, const agc_word *k);
void lmi_pdvl(apollo_overlay_state *, const agc_word *k);
void lmi_pddl_op(apollo_overlay_state *, const agc_word *k);
void lmi_push(apollo_overlay_state *);
/* Scalar arithmetic: DAD, DSU, BDSU (ENDVXV), DMP (DMP1), DMPR. */
void lmi_dad(apollo_overlay_state *, const agc_word *k);
void lmi_dsu(apollo_overlay_state *, const agc_word *k);
void lmi_bdsu(apollo_overlay_state *, const agc_word *k);
void lmi_dmp(apollo_overlay_state *, const agc_word *k);
void lmi_dmpr(apollo_overlay_state *, const agc_word *k);
/* Short shifts: OPJUMP2 for a short shift code, ending with Z at SHORTT
 * (02017) or SHORTV (02121) in bank 0; those bodies end at DANZIG. */
void lmi_short_dispatch(apollo_overlay_state *);
void lmi_shortt(apollo_overlay_state *);
void lmi_shortv(apollo_overlay_state *);
/* Addressed shifts: GSHIFT (C(ADDRWD) = shift word), its GENSHIFT tail
 * (A = LOW7, FBANK 0), and TSLC (store address in ADDRWD). */
void lmi_gshift(apollo_overlay_state *);
void lmi_genshift(apollo_overlay_state *);
void lmi_tslc(apollo_overlay_state *);
/* COMP, SIGN, ABS, DSQ, VSQ and DOT. */
void lmi_comp(apollo_overlay_state *);
void lmi_sign(apollo_overlay_state *, const agc_word *k);
void lmi_abs(apollo_overlay_state *);
void lmi_dsq(apollo_overlay_state *);
void lmi_vsq(apollo_overlay_state *);
void lmi_dot(apollo_overlay_state *, const agc_word *k);
/* DDV/BDDV through the bank-0 divide (DANZIG, also on division overflow). */
void lmi_ddv(apollo_overlay_state *, const agc_word *k);
void lmi_bddv(apollo_overlay_state *, const agc_word *k);
/* SQRT, ABVAL/ABS and UNIT: 0 at DANZIG, 1 at POODOO1 (alarm 1302, Q 03404). */
uint8_t lmi_sqrt(apollo_overlay_state *);
uint8_t lmi_abvalabs(apollo_overlay_state *);
uint8_t lmi_unit(apollo_overlay_state *);
void lmi_vxsc(apollo_overlay_state *, const agc_word *k);
void lmi_vxv(apollo_overlay_state *, const agc_word *k);
/* VPROJ 07425 ((MPAC.X)X: DOT through PREDOT, then DVXSC) to DANZIG. */
void lmi_vproj(apollo_overlay_state *, const agc_word *k);
/* BOV(B) (bank 1): DANZIG, GOTO 06646 (then lmi_goto) or RTB 02450. */
enum {LMI_BOV_NONE,LMI_BOV_GOTO,LMI_BOV_RTB,LMI_BOV_OUT_OF_SCOPE};
/* BPL/BMN order: 1 = take GOTO (W[5]=06646), 0 = DANZIG. */
uint8_t lmi_bplbmn(apollo_overlay_state *);
uint8_t lmi_bov(apollo_overlay_state *);
/* RTB (bank 1, POLISH = the routine's CADR) to the routine's first word,
 * with FBANK switched and Q = SWRETURN 04631; SWRETURN back to DANZIG. */
void lmi_rtb(apollo_overlay_state *);
void lmi_swreturn(apollo_overlay_state *);
/* POLY 07222 with Q = the address after TC POLY and `table` the caller's
 * fixed words from there (N-1, then a0..aN); ends with Z at the word after
 * the table (TC POLYRET, Q = 0142). */
void lmi_poly(apollo_overlay_state *, const agc_word *table);
/* VAD, VSU, BVSU and TAD (overflow to OVFIND), each to DANZIG. */
void lmi_vad(apollo_overlay_state *, const agc_word *k);
void lmi_vsu(apollo_overlay_state *, const agc_word *k);
void lmi_bvsu(apollo_overlay_state *, const agc_word *k);
void lmi_tad(apollo_overlay_state *, const agc_word *k);
/* ROUND (unary) and SSP (ADDRWD set; `word` the SP word given). */
void lmi_round(apollo_overlay_state *);
void lmi_ssp(apollo_overlay_state *, agc_word word);
/* XAD, XSU, LXA, SXA (POLISH = the erasable address, CYR selects X1/X2). */
void lmi_xad(apollo_overlay_state *);
void lmi_xsu(apollo_overlay_state *);
void lmi_lxa(apollo_overlay_state *);
void lmi_sxa(apollo_overlay_state *);
/* ITA (STQ) 01,2507 with CYR negative: QPRET into the 15ADRERS address,
 * then DANZIG. */
void lmi_ita(apollo_overlay_state *);
/* TIX 01,2415 (Z there, POLISH the branch CADR): 1 when it branches
 * (decremented index stored, Z = GOTO 06646), 0 at DANZIG. */
uint8_t lmi_tix(apollo_overlay_state *);
/* BHIZ: 1 = GOTO 06646 (MPAC high word zero), 0 = DANZIG. */
uint8_t lmi_bhiz(apollo_overlay_state *);
/* CALL (POLISH = the fixed CADR): QPRET set, ends at GOTO 06646. */
void lmi_call(apollo_overlay_state *);
/* GOTO from 06646, through GOTOERS while POLISH names erasable (fetching
 * the CADR through the work area or its EBANK), to the target's NEWOPS. */
void lmi_goto_any(apollo_overlay_state *);
/* 15BITADR (STCALL's entry) to the MISCJUMP body `target`. */
void lmi_15bitadr(apollo_overlay_state *, agc_word word, agc_word target);
/* BZE/GOTO without the transfer: 1 at GOTO 06646 (BZE zero, or GOTO),
 * 0 at DANZIG. */
uint8_t lmi_bzegoto(apollo_overlay_state *);
/* TLOAD 06470 with an erasable operand at C(ADDRWD), to DANZIG (TP mode). */
void lmi_tload(apollo_overlay_state *);
/* TLOAD whose ADDRWD names the fixed TP words k[0..2] (INDEX ADDRWD reads
 * the FBANK). */
void lmi_tload_fixed(apollo_overlay_state *, const agc_word *k);
/* EXIT to the basic word after the order (Z = LOC+1, Q 06747); TC INTPRET
 * from basic code (Q = the next word) to NEWOPS; TPMODE (RTB routine). */
void lmi_exit(apollo_overlay_state *);
void lmi_intpret(apollo_overlay_state *, p66_bias_io *);
void lmi_tpmode(apollo_overlay_state *);
#endif
