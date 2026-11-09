/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc (Luminary099 pages 1205-1226, Comanche055
 * pages 1309-1333). The faithful C is c64-port/src/lm_integration_init.c
 * and cm_integration_init.c (and lm_integration_moves.c and
 * cm_integration_moves.c, the moves an RTB runs).
 *
 * The way into orbital integration: it "OPERATES AS A CLOSED INTERPRETIVE
 * SUBROUTINE" (LM p. 1205, CM p. 1309) that moves a state vector, the
 * CSM's or the LM's, to a time (TDEC1), by Kepler's method (conic) or by
 * precision integration, with or without the W-matrix, and keeps the
 * result as the vehicle's permanent vector or not. Pages 1205-1209 (CM
 * 1309-1313) describe its entrances, flags and outputs; the code starts
 * on page 1209 (CM 1313).
 *
 * The orbital integration works on its own state vector, RRECT and the 41
 * words after it (position, velocity, the deviations from the conic, the
 * times and the rest). Each vehicle keeps a permanent copy: RRECTCSM for
 * the CSM, RRECTLEM for the LM. Each move copies one into the other,
 * through SETBANK (the integration's banks), last word first, and ends at
 * DANZIG, as a routine an RTB runs does.
 *
 * Written in parts (README.md):
 *   the moves (moves.c: MOVEACSM to MOVEPLEM and SETBANK, LM pp.
 *     1211-1213, CM pp. 1315-1317), with MEASUREMENT_INCORPORATION;
 *   part 1: STATEINT to NOINT (stateint.c: LM pp. 1209-1210, CM pp.
 *     1313-1314) and the transfers ATOPCSM to USEPIOS (transfers.c: LM
 *     pp. 1210-1213, CM pp. 1314-1317);
 *   part 2: the entries CSMPREC to INTEGRV2 and ALOADED (integrv.c: LM
 *     pp. 1213-1216, CM pp. 1317-1320), A-PCHK to INTEXIT (rectout.c: LM
 *     pp. 1216-1217, CM pp. 1320-1321) and RVCON, TESTLOOP to DT/2MAX
 *     (testloop.c: LM pp. 1217-1219, CM pp. 1321-1324);
 *   part 3: INTSTALL to INTBITAB, and the basic code after INTEXIT's
 *     EXIT (intstall.c: LM pp. 1217, 1219-1220, CM pp. 1321, 1324-1326).
 * The binding (src/binding/integration_init_routines.c) runs the faithful
 * port's routines at the keys no part has written yet. */
#ifndef READABLE_INTEGRATION_INIT_H
#define READABLE_INTEGRATION_INIT_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#ifndef COUNT
#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* A fixed label's bank, and its address in the bank, from its I_ value
 * (bank * 02000 + its place in the bank). */
#define CODE_BANK(i) ((unsigned)((i) >> 10))
#define CODE_Z(i)    ((word)((i) % 02000u + 02000u))

/* The moves. */
void moveacsm(void); /* MOVEACSM: RRECT to RRECTCSM */
void movepcsm(void); /* MOVEPCSM: RRECTCSM to RRECT */
void movealem(void); /* MOVEALEM: RRECT to RRECTLEM */
void moveplem(void); /* MOVEPLEM: RRECTLEM to RRECT */

/* Part 1: STATINT1's interpretive code (to ENDINT's EXIT), SETIFLGS with
 * NOINT's EXIT, and the transfers. */
extern const struct interpretive_code STATINT1_CODE, SETIFLGS_CODE;
extern const struct interpretive_code ATOPCSM_CODE, PTOACSM_CODE, ATOPLEM_CODE, PTOALEM_CODE,
    USEPIOS_CODE;

/* Part 1's basic code: STATEINT's task, STATINT1's start, and the code
 * after ENDINT's and NOINT's EXITs, each from a key. */
void stateint(void);                /* STATEINT: TC PHASCHNG */
void stateint_after_phaschng(void); /* STATEINT +2: TC FINDVAC */
void stateint_after_findvac(void);  /* STATEINT +6: TC TASKOVER */
void statint1(void);                /* STATINT1: TC INTPRET */
void endint_after_exit(void);       /* ENDINT +2: TC PHASCHNG */
void endint_after_phaschng(void);   /* ENDINT +4: TC LONGCALL */
void endint_after_longcall(void);   /* ENDINT +011: TC ENDOFJOB */
void noint_after_exit(void);        /* NOINT +1: TC PHASCHNG */
void noint_after_phaschng(void);    /* NOINT +3: TC DOWNFLAG */
void noint_after_downflag(void);    /* NOINT +5: TC ENDOFJOB */

/* Part 2: the entries (to INTEGRV2's BON GOTO), ALOADED, A-PCHK (to its
 * EXIT), the words after its TC INTPRET (to INTEXIT's EXIT), and RVCON to
 * DT/2MAX. */
extern const struct interpretive_code INTEGRV_CODE, ALOADED_CODE, A_PCHK_CODE, PHEXIT_CODE,
    TESTLOOP_CODE;

/* Part 2's basic code: after A-PCHK's EXIT (the CM's ANDOUT's), each from
 * a key. */
void a_pchk_after_exit(void);     /* A-PCHK (ANDOUT) +3: TC PHASCHNG */
void a_pchk_after_phaschng(void); /* +5: TC UPFLAG */
void a_pchk_after_upflag(void);   /* +7: TC INTPRET */

/* Part 3: the EXITs of INTSTALL and INTWAKE0 (the CM's ERASTAL1 and
 * ERASTAL2 too), and GOBAC's RVQ. */
extern const struct interpretive_code INTSTALL_CODE, INTWAKE0_CODE, GOBAC_RVQ_CODE;
#if COMANCHE055
extern const struct interpretive_code ERASTAL1_CODE, ERASTAL2_CODE;
#endif

/* Part 3's basic code, each from a key. */
void intstall_after_exit(void);    /* INTSTALL +1 */
void intwake(void);                /* INTWAKE */
void intwake_after_phaschng(void); /* INTWAKE +011: CA TBASE2 */
void intwake1(void);               /* INTWAKE1, and INTWAKE0 +1 */
void intwake1_after_jobwake(void); /* INTWAKE1 +3 (the CM's WAKE1 +4) */
void intexit_after_exit(void);     /* INTEXIT +012: CA MPAC */
#if LUMINARY099
void intwake_after_intwake1(void); /* INTWAKE +4 */
#else
void erastal1_after_exit(void); /* ERASTAL1 +1 */
void erastal2_after_exit(void); /* ERASTAL2 +1 */
void eraswak1(void);            /* ERASWAK1 */
void eraswak2(void);            /* ERASWAK2 */
#endif

#endif
