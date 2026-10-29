/* SPDX-License-Identifier: MIT */
#ifndef LM_T6RUPT_ROUTINES_H
#define LM_T6RUPT_ROUTINES_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 T6-RUPT_PROGRAMS.agc, its INTERRUPT_LEAD_INS.agc vector and
 * WAITLIST.agc's interrupt exits as native routines of the control-transfer
 * ABI (docs/LM-T6RUPT.md); io is p66_bias_io and every routine needs
 * program->hardware (channels 5, 6 and 13, in_interrupt).
 *   T6RUPT vector 04004   DXCH ARUPT; DCA T6ADR; DTCB to DOT6RUPT
 *   DOT6RUPT 17,2065      LXCH BANKRUPT; QXCH QRUPT; TC T6JOBCHK; then
 *                         TCF RESUME
 *   T6JOBCHK 17,2027      CCS TIME6: positive, return (TC Q); +0, TC
 *                         CCSHOLE; -0, T6JOB: stop the clock, shift
 *                         T6FURTHA into T6NEXT and T6NEXT into TIME6 and
 *                         NXT6ADR, restart the clock (or leave it stopped
 *                         when the next time would overflow), and fire the
 *                         jets NXT6ADR names (0 P, 4 U, 13 V) at WRITEP-1,
 *                         WRITEU-1 or WRITEV-1
 *   WRITEP-1 05753, WRITEP 05754   (NEXTP) into channel 6, return
 *   WRITEU-1 05757, WRITEU 05760   (NEXTU) into channel 5's U jets, return
 *   WRITEV-1 05770, WRITEV 05771   (NEXTV) into channel 5's V jets, return
 *   RESUME 05270, NOQRSM 05272, NOQBRSM 05274
 *                         restore Q (RESUME only), BB from BANKRUPT (not
 *                         NOQBRSM), A and L from ARUPT, RELINT, and end the
 *                         interrupt at ZRUPT-1.
 * A return (TC Q) leaves Z at Q and Q = 3, as the original's RETURN does. A
 * negative nonzero TIME6 (T6JOBCHK +3 branches to itself), a NXT6ADR other
 * than 0, 4 or 13, or a T6 call outside EBANK 6 has no producer and is
 * EXEC_OUT_OF_SCOPE with the image, io and hardware unchanged. */
extern const agc_routine_table lm_t6rupt_routines;
/* The RESUME family's four entries (the flight store's pinned set). */
extern const agc_routine_table lm_t6rupt_hot_routines;
/* The jet writes (WRITEP, WRITEU, WRITEV and their -1 entries). */
extern const agc_routine_table lm_t6rupt_jets_hot_routines;
#define LM_T6RUPT_VECTOR 04004u
#define LM_T6JOBCHK 02027u
#define LM_DOT6RUPT 02065u
#define LM_T6_BANK 036000u
#define LM_WRITEP 05754u
#define LM_WRITEU 05760u
#define LM_WRITEV 05771u
#define LM_RESUME 05270u
#define LM_NOQRSM 05272u
#define LM_NOQBRSM 05274u
#define LM_CCSHOLE 05705u
#define LM_TIME6 031u
#define LM_NXT6ADR 01465u
#define LM_T6NEXT 01466u
#define LM_T6FURTHA 01470u
#define LM_NEXTP 01472u
#define LM_NEXTU 01473u
#define LM_NEXTV 01474u
#endif
