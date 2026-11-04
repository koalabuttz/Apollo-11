/* SPDX-License-Identifier: MIT */
/* R30.agc, both missions' (Luminary099 pp. 712-722, Comanche055 pp.
 * 514-524): R30, the orbital parameters display, verb 82, and SR30.1, which
 * works them out. The faithful C is c64-port/src/lm_r30.c and cm_r30.c.
 *
 * The interpretive code is in the listing's notation, run by the readable
 * interpreter; the entries below are the basic code's. Each ends where its
 * code leaves it: at a TC out of the file (Z the routine, Q the return), at
 * a TC INTPRET (Z INTPRET) or at the jump out of the file (BZF ENDEXT).
 * The binding, src/binding/r30_routines.c, runs on from a TC INTPRET into
 * the interpreter, as the faithful port does.
 *
 * The Luminary099 listing spells TICKTFF "TICKTIFF" (R30.agc, p. 715);
 * its assembly, and the Comanche055 listing, spell it TICKTFF, the name
 * used here for both missions. */
#ifndef READABLE_R30_H
#define READABLE_R30_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

extern const struct interpretive_code V82CALL_CODE, V82GOFF1_CODE, OTHSHIP_CODE, THISSHIP_CODE,
    EARTHPAD_CODE, TICKTFF_CODE, V82GON_CODE, V82GON1_CODE, SR30_1_CODE;
#if !LUMINARY099
extern const struct interpretive_code SPLRET1_CODE, CANDEL_CODE;
#endif

void v82call(void);         /* V82CALL */
void v82goff_options(void); /* V82GOFF +1: V04N06 */
void v82goff_display(void); /* V82GOFF +5 */
void v82goff_terminate(void);
void v82goff_proceed(void);
void v82goff_recycle(void);
void v82goff_ticktest(void); /* V82GOFF +11: the first TICKTEST */
void v82goflp_relint(void);  /* V82GOFF +15 */
void v82goflp(void);         /* V82GOFLP */
void v82stall_relint(void);  /* V82GOFLP +8 */
void v82stall(void);         /* V82STALL */
void v82stall_wait(void);    /* V82STALL +4 */
void v82stall_again(void);   /* V82STALL +7 */
void flaggon(void);          /* FLAGGON: V16N44 */
void flaggon_terminate(void);
void flaggon_proceed(void);
void flaggon_recycle(void);
void v82goff1(void);          /* V82GOFF1 */
void v82goff1_vehicle(void);  /* V82GOFF1 +6 */
void tickpart_tper(void);     /* TICKTPER +5 */
void tickpart_tff(void);      /* TICKTFF +5 */
void ticktest(void);          /* TICKTEST */
void ticktest_end(void);      /* TICKTEST +4 */
void ticktest_taskover(void); /* TICKTEST +8 */
void dotick(void);            /* DOTICK */
void dotick_flags(void);      /* DOTICK +4 */
void dotick_taskover(void);   /* DOTICK +8 */
void tfftick(void);           /* TFFTICK */
void tpertick(void);          /* TPERTICK */
void v82gon_findvac(void);    /* V82GON +1 */
void v82gon_relint(void);     /* V82GON +5 */
void v82redsp(void);          /* V82REDSP: V16N44 */
void v82redsp_terminate(void);
void v82redsp_proceed(void);
void v82redsp_recycle(void);
void v82gon1(void);       /* V82GON1 */
void v82gon3(void);       /* V82GON3 */
void v82gon3_again(void); /* V82GON3 +7 */
#if !LUMINARY099
void bothpad_modreg(void);  /* BOTHPAD +3 */
void v82gon2_checkmm(void); /* V82GON2 +3 */
void v82gon2_not_11(void);  /* V82GON2 +5 */
void candel(void);          /* CANDEL */
void splret_modreg(void);   /* SPLRET +1 */
#endif

#endif
