/* SPDX-License-Identifier: MIT */
/* UPDATE_PROGRAM.agc, both missions (Luminary099 pages 1386-1396,
 * Comanche055 pages 1497-1507): P27, "THE UPDATE PROGRAM", which "PROCESSES
 * COMMANDS AND DATA INSERTIONS REQUESTED BY THE GROUND VIA UPLINK". The
 * faithful C is c64-port/src/lm_update.c and cm_update.c.
 *
 * The ground keys an extended verb up the uplink, and P27 asks for its
 * data on the DSKY (V21N01, flashing), one component at a time, into
 * UPBUFF:
 *   - V70, "LIFTOFF TIME INCREMENT": a double precision time added to
 *     TEPHEM and taken from the clock and both state vectors' times;
 *   - V71, "CONTIGUOUS BLOCK UPDATE": a count II, an ECADR, and II-2
 *     words for erasable from that ECADR on;
 *   - V72, "SCATTER UPDATE": a count II, then (II-1)/2 pairs of an ECADR
 *     and its word;
 *   - V73, "OCTAL CLOCK INCREMENT": a double precision time added to the
 *     clock.
 * Then V21N02 asks the ground to check the data: a component number
 * brings that component back to be keyed again, V33 stores the update
 * (UPSTORE), V34 ends P27 with nothing stored. A V32 at any request asks
 * again.
 *
 * The missions run the same code but for one check: P27 takes an update
 * only in P00 or after a fresh start (MODREG +0 or -0), and the CM's
 * CKMDMORE takes one in P02 too, when the computer is the CM's (FLAGWRD5
 * bit 8 set; with it clear, "IT'S THE LEM"). The CM writes BIT3 where the
 * LM names it
 * VERIFBIT. V70-V73 are in the extended verb bank (43), the rest of P27
 * in bank 04 (the CM's in bank 27).
 *
 * Each function below is an entry: a label, or the word after a call out
 * of the file, where that routine returns (the name says which). A run
 * ends where control leaves the file. Not modelled, as the faithful port
 * does not take them: an update that would write the central registers or
 * the counters (erasable below 061), a TS that overflows where the program
 * makes no overflow, and an UPVERB, UPTEMP or Q that sends a computed jump
 * anywhere but the words the listing means. */
#ifndef READABLE_UPDATE_PROGRAM_H
#define READABLE_UPDATE_PROGRAM_H

#include "../agc/names.h"

/* The instruction TCF k, as INDEX UPVERB changes it. */
#define TCF_TO(k) ((word)(010000u | (k)))

/* TS where the program makes no overflow: one would skip the next word,
 * which the faithful port does not take. */
static inline void ts_no_overflow(unsigned address)
{
    if (ts(address)) not_modelled();
}

/* TC Q, back to the word in Q. This file's faithful port goes to Q as a
 * word (overflow-corrected), and the readable code matches the faithful
 * port. */
static inline void return_through_q(void)
{
    Q = overflow_corrected(Q);
    tc_through(Q_ADDRESS);
}

/* The verbs (verbs.c), bank 43. */
void v70updat(void);
void v71updat(void);
void v72updat(void);
void v73updat(void);
void v73updat_after_testxact(void); /* V73UPDAT +3 */

/* Loading UPBUFF (loading.c). */
void uppart2(void);
void uppart2_after_phaschng(void);
void uppart2_after_newmodex(void);
void ohwell1_terminate(void); /* GOXDSPF's returns: V34, */
void ohwell1_proceed(void);   /* V33, */
void ohwell1_data(void);      /* and data or V32 */
void ohwell2_terminate(void);
void ohwell2_proceed(void);
void ohwell2_data(void);
void upverify_terminate(void);
void upverify_proceed(void);
void upverify_data(void);

/* Storing it (store.c): UPSTORE and the clock. */
void upstore(void);
void upstore_after_phaschng(void);
void upend73_exit(void); /* UPEND73 +5 */
void upfndvac_after_findvac(void);
void upjob(void);
void upwake(void); /* UPWAKE +1, after the interpretive EXIT */
void upwake_after_phaschng(void);
void upwake_after_upflag(void);
void timedidl(void);
void timedidr(void);
void timedidr_after_phaschng(void);
void deltatok_after_tpagree(void);
void deltatok_after_phaschng(void);

/* The updates (updates.c). */
void upend70(void);
void upend70_clock_error(void);    /* UPEND70 +4, TIMEDIDL's error return */
void upend70_after_timedidl(void); /* UPEND70 +5 */
void upend70_after_phaschng(void);
void upend70_after_phaschng2(void);
void upend71(void);
void upend72(void);

/* The exits (exits.c). */
void upout(void);
void upout4(void);       /* UPOUT +1 */
void upout_plus_2(void); /* with the mode in A */
void upout_after_newmodex(void);
void uperrout(void);
void uperrout_after_falton(void);
void uperrout_quiet(void); /* UPERROUT +2 */
void uperrout_quiet_after_falton(void);
void upactoff(void);

/* UPJOB's interpretive words, CALL INTSTALL and UPWAKE EXIT. */
extern const struct interpretive_code UPJOB_CODE;

#endif
