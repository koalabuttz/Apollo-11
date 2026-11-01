/* SPDX-License-Identifier: MIT */
/* R60_62.agc (Luminary099 pp. 472-485, Comanche055 pp. 390-398): R60,
 * the attitude maneuver routine programs call to turn the spacecraft to an
 * attitude (VECPOINT works out the gimbal angles that point an axis along a
 * direction), and R62, verb 49's maneuver to gimbal angles the crew loads.
 * The faithful C is c64-port/src/lm_r60_r62.c and cm_r60_r62.c.
 *
 * Basic code (R60 and R62, and the LM's BALLANGS) round stretches of
 * interpretive code (VECPOINT, the CALLs of it, and the LM's BALLANGS),
 * which are in the listing's notation, run by the readable interpreter.
 * The entries below are the basic code's: R60's and R62's starts, the
 * words BANKCALL, MAKECADR, UPFLAG, DOWNFLAG, PHASCHNG, PRIOCHNG and LINUS
 * return to, the displays' returns (terminate, proceed, enter and
 * GOPERF2R's immediate return), the words after each EXIT, and the LM's
 * G+N,AUTO and ISITAUTO, which other files call. The binding is
 * src/binding/r60_62_routines.c. */
#ifndef READABLE_R60_62_H
#define READABLE_R60_62_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

extern const struct interpretive_code REDOMANN_CODE;
extern const struct interpretive_code REDOMANC_CODE;
extern const struct interpretive_code VECPOINT_CODE;

/* R62 */
void r62disp(void);
void r62disp_terminate(void);
void r62disp_proceed(void);
void r62disp_enter(void);
void gomove_after_upflag(void);
void gomove_after_bankcall(void);

/* R60 */
void toball(void);
void endmanuv(void);
void endmanu1_after_downflag(void);
void startmnv(void);
void chklinus_after_phaschng(void);
void chklinus_after_linus(void);
void relinus(void);
void toballc(void);
#if LUMINARY099
extern const struct interpretive_code BALLANGS_CODE;
extern const struct interpretive_code ROT180_CODE;

void r60lem(void);
void r60lem_after_makecadr(void);
void toballa(void);
void toballa_terminate(void);
void toballa_proceed(void);
void toballa_enter(void);
void toballa_immediate_return(void);
void toballa_after_chklinus(void);
void toballc_after_ballangs(void);
void toballc_after_g_n_auto(void);
void automanv_after_godspr(void);
void relinus_after_priochng(void);
void relinus_not_p20(void);
void relinus_after_p25_phaschng(void);
void relinus_after_p20_phaschng(void);
void rer60_after_upflag(void);
void g_n_auto(void);
void isitauto(void);
void ballangs(void);
void ballangs_after_makecadr(void);
void endball(void);
#else
void r60csm(void);
void r60csm_after_makecadr(void);
void toball_terminate(void);
void toball_proceed(void);
void toball_enter(void);
void toball_immediate_return(void);
void toball_after_chklinus(void);
void toballc_after_godspr(void);
void relinus_after_upflag(void);
void relinus_after_targ1flg(void);
void relinus_after_priochng(void);
void goredo20_after_phaschng(void);
#endif

#endif
