/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099) and RT8_OP_CODES.agc (Comanche055): the
 * basic routines an interpretive RTB runs (return to basic: RTB names a
 * routine of these, the interpreter's SWCALL goes there, and the routine
 * ends back at DANZIG). Each ends where the listing leaves the file
 * (rtb_op_codes.c); an order pair that RTBs to one runs on from there to
 * its DANZIG (rtb_run_on(), src/binding/rtb_run_on.c). The
 * CM's QUALITY1 and QUALITY2, interpretive code, are QUALITY1_CODE and
 * QUALITY2_CODE (quality.c). */
#ifndef READABLE_RTB_OP_CODES_H
#define READABLE_RTB_OP_CODES_H

#include <stdbool.h>
#include "../agc/agc.h"

void loadtime(void);
void cdulogic(void);
void sgnagree(void);
void sgnagree_after_tpagree(void); /* SGNAGREE +1 */
void one_sto2s(void);
void one_sto2s_plus1(void); /* where 1TO2SUB returns */
void v1sto2s(void);
void v1sto2s_plus1(void); /* where 1TO2SUB returns, three times */
void v1sto2s_plus4(void);
void v1sto2s_plus8(void);
void tpmode(void);
void two_v1sto2s(void);
void two_v1sto2s_plus1(void); /* where 1TO2SUB returns, twice */
void two_v1sto2s_plus4(void);
word cduinc(void);
void pulseimu(void);
void pulseimu_after_imupulse(void); /* PULSEIMU +4 */
void signmpac(void);
void dpmode(void);
void normunx1(void);
void normunit(void);
void normunit_after_vecagree(void); /* NORMUNIT +5 */
void vecsgnag(void);
void vecsgnag_after_vecagree(void); /* VECSGNAG +2 */
#if !LUMINARY099
void readpips(void);
void vecmode(void);
void incrcdus(void);
void incrcdus_plus4(void); /* where CDUINC returns, three times */
void incrcdus_plus7(void);
void incrcdus_plus10(void);
void transp1(void);
void transp2(void);

struct interpretive_code;
extern const struct interpretive_code QUALITY1_CODE, QUALITY2_CODE;
#endif

#endif
