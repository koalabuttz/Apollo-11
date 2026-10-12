/* SPDX-License-Identifier: MIT */
#ifndef LM_S3435C_CASES_H
#define LM_S3435C_CASES_H
#include "lm_s3435c_routines.h"
#include "lm_s3435b_routines.h"
#include "lm_s3435_routines.h"
#include "lm_p34_routines.h"
/* Deterministic sampled entries for the S34/35.2 and S34/35.5 transfer
 * routines. Case n enters entry n%18 over poisoned erasable debris: the ten
 * interpretive entries (S34/35.2, its INITVEL and LOMAT returns, S34/35.5,
 * FLAGON, the target check's INTPRET return, NOCHG, FLAGOFF, its S34/35.4
 * return and VNPOOH's INTPRET return) at NEWOPS, then the eight basic
 * entries (FLAGON +5, GOFLASH's three returns, the target check, UPFLAG's
 * return, FLAGOFF's EXIT and VNPOOH's return) at Z. Debris covers any
 * vectors and flags (FINALFLG, NTARGFLG), NN zero or not, GDT/2 equal to
 * DELVLVC or not, every VAC, PUSHLOC (as far as the routine's pushes leave
 * room), MODE, EB, IRQ and allowed SUPERBNK; QPRET and SUBEXIT are each one
 * of the real callers' returns or another bank's CADR. Every case runs
 * through the trampoline with this file's routines, lm-s3435b's and
 * lm-s3435's, lm-p34's basic routines (VNPOOH) and the native INTER-BANK
 * routines (BANKCALL) to the first point outside them; the oracle stops at
 * the same point (lm_s3435c_stop). */
#define LM_S3435C_ROUTINES 18u
#define LM_S3435C_PER_ROUTINE 1024u
#define LM_S3435C_CASES (LM_S3435C_ROUTINES*LM_S3435C_PER_ROUTINE)
#define LM_S3435C_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_s3435c_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside their interpretive code, GOFLASH (through BANKCALL),
 * UPFLAG, GOTOPOOH, INTPRET, or UNIT's POODOO1 (alarm 1302). */
int lm_s3435c_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_S3435C_STOPPED 1u
#define LM_S3435C_REFUSED 2u
unsigned lm_s3435c_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_s3435c_digest(apollo_overlay_state *,uint32_t n);
#endif
