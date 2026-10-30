/* SPDX-License-Identifier: MIT */
/* The restart tables: RESTART_TABLES.agc of Luminary099 (LM, listing pages
 * 238-243) and Comanche055 (CM, pages 211-221), and the CM's NEWPHASE,
 * which that file also holds. The faithful C is c64-port/src/
 * lm_restart_table_data.inc, cm_restart_table_data.inc and
 * cm_restart_routines.c.
 *
 * The listing's own description (LM p. 238, CM p. 211), in short: each
 * group has an even and an odd table. An odd phase's entry is three words,
 * a priority or delta time (PRDTTAB) and a 2CADR (CADRTAB); an even
 * phase's is two such entries. For a job the priority is positive
 * (FINDVAC) or negative (NOVAC) and the 2CADR positive. For a task the
 * 2CADR is stored complemented, and the time is direct (positive), the
 * complement of the address holding it, or -0 (start now). For a LONGCALL
 * the GENADR is complemented, the BBCON not, and PRDTTAB holds the address
 * of its double delta time.
 *
 * RESTARTS (restarts.c) reads them as the AGC does, from fixed memory in
 * bank 01; the words are here in order from SIZETAB, generated from the
 * listing (tools/restart_tables.py, checked by make names-check). */
#include "restart.h"
#include "../agc/names.h"

const word restart_tables[] = {
#if LUMINARY099
#include "lm_restart_tables.inc"
#else
#include "cm_restart_tables.inc"
#endif
};
const unsigned restart_table_words = RESTART_TABLE_WORDS;

#if COMANCHE055
/* NEWPHASE and NUFAZ+10 -- CM p. 221 (in fixed-fixed).
 * A faster phase change than PHASCHNG, for a type A phase only: the phase
 * in A, the group after the TC. A negative phase also sets the group's
 * TBASE, and is stored positive. Then the phase and its complement go
 * into the group's PHASE and -PHASE ("DEBRIS: A,L,TEMPG"). */
void newphase(void)
{
    inhint();
    ts(L_ADDRESS);       /* "SAVE FOR FURTHER USE" */
    set_A(read_word(Q)); /* NDX Q; CA 0: "OBTAIN THE GROUP NUMBER" */
    Q = add16(Q, 1u);    /* INCR Q: "OBTAIN THE RETURN ADDRESS" */
    double_A();          /* "SAVE THE GROUP IN A FORM USED FOR INDEXING" */
    ts(ADDRESS_OF(TEMPG));
    switch (ccs_register(L_ADDRESS)) { /* "SEE IF WE ARE TO SET TBASE" */
    case POSITIVE:                     /* TCF +7: "NO, THE DELTA T WAS POSITIVE" */
    case IS_PLUS_ZERO: break;          /* TCF +6 */
    case NEGATIVE:
        incr_A(); /* NUFAZ+10: "SET TBASE AND STORE PHASE CORRECTLY" */
                  /* fall through */
    case IS_MINUS_ZERO:
        ts(L_ADDRESS);
        set_A(negate(TIME1)); /* "SET TBASE" */
        ts(OF_GROUP(TBASE1, TEMPG));
        break;
    }
    A = (reg16)~L; /* CS L: "NOW PUT THE PHASE IN THE RIGHT TABLE LOC" */
    dxch(OF_GROUP(MPHASE1, TEMPG));
    relint();
    tc_through(Q_ADDRESS); /* TC Q: "NOW RETURN TO CALLER" */
}
#endif
