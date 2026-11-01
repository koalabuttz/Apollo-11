/* SPDX-License-Identifier: MIT */
/* S-BAND_ANTENNA_FOR_LM.agc (Luminary099 pp. 486-489) and
 * S-BAND_ANTENNA_FOR_CM.agc (Comanche055 pp. 934-935): R05, verb 64, the
 * angles to point the steerable S-band antenna at the earth. The faithful
 * C is c64-port/src/lm_sband_antenna.c and cm_sband_antenna.c.
 *
 * Both are in bank 42 (FB 32 with the superbank bit). Basic code starts
 * the routine and shows the angles; between the two, interpretive code in
 * the listing's notation (SBAND_CODE), run by the readable interpreter,
 * works them out. The entries below are the basic code's: the start, the
 * word BANKCALL comes back to after R02BOTH, the word after the EXIT, the
 * words PRIOCHNG, BLANKET and (the CM's) DELAYJOB come back to, and
 * GOMARKFR's four returns. The binding is
 * src/binding/sband_antenna_routines.c. */
#ifndef READABLE_SBAND_ANTENNA_H
#define READABLE_SBAND_ANTENNA_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

extern const struct interpretive_code SBAND_CODE;

void sbandant(void);
void sbandant_after_r02both(void); /* SBANDANT +2 */
void sbandant_after_exit(void);    /* the word after the EXIT */
void sbandant_terminate(void);     /* GOMARKFR's returns */
void sbandant_proceed(void);
void sbandant_recycle(void);
void sbandant_immediate_return(void);
void sbandant_after_blanket(void);
#if LUMINARY099
void sbandant_after_prio5(void); /* PRIOCHNG's returns */
void sbandant_after_prio4(void);
#else
void sbandant_after_delayjob(void);
#endif

#endif
