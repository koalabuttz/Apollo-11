/* SPDX-License-Identifier: MIT */
/* AUTOMATIC_MANEUVERS.agc (Comanche055 pp. 1034-1036, bank 21): the end
 * of the jet switching logic. JTIME turns the change in rate the phase
 * plane's region called for (KMPAC) into the axis' jet-on time, "T  =
 * J/M(DELTA W)": times the crew's inertia to torque ratio J/M ("FOR 1 JET
 * OPERATION") and BIT11, by SMALLMP twice, held to POSMAX or NEGMAX. DOJET
 * stores it in TAU, TAU1 or TAU2 and goes round JLOOP for the next axis;
 * after the last, T6PROG sets phase 3, the jet selection logic, for the
 * next T5 interrupt. The faithful C is c64-port/src/cm_auto_man.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- JTIME -- p. 1035 ---- */

/* JTIME: "PICK UP S/C INERTIA/TORQUE RATIO" for the axis, J/M, J/M1 or
 * J/M2, times KMPAC (SMALLMP). */
void jtime(void)
{
    ca(maneuver_index_read(ADDRESS_OF(J_M), spndx()));
    tc_to(JTIME + 2u, SMALLMP);
}

/* JTIME +3, after SMALLMP: times BIT11 (SMALLMP again). */
void jtime_bit11(void)
{
    set_A(BIT11);
    tc_to(JTIME + 4u, SMALLMP);
}

/* JTIME +5, after SMALLMP: the jet-on time, "SCALED 16384/1600 SEC": the
 * lower word of KMPAC (TAUNORM) when the upper one is +0 or -0, POSMAX or
 * NEGMAX by its sign otherwise. DOJET stores it as the axis' time, TAU
 * indexed by SPNDX. (The TS's overflow skip, for an A that holds an
 * overflow, is not honored by the faithful port here; nor is it in this
 * code.) */
void jtime_ccs(void)
{
    switch (ccs_at(ADDRESS_OF(KMPAC))) {
    case POSITIVE: set_A(POSMAX); break;
    case NEGATIVE: set_A(NEGMAX); break;
    default: ca(ADDRESS_OF(KMPAC) + 1u); break; /* TAUNORM */
    }
    ts(index_ts(ADDRESS_OF(TAU), spndx())); /* DOJET */
    dojet_next();
}

/* DOJET +2: the next axis, SPNDX less one, at JLOOP; after the last (SPNDX
 * +0), T6PROG. A negative SPNDX, which the loop never leaves, runs on into
 * ZEROCMDS: the three times zeroed, then T6PROG; -0 joins it at its TS TAU
 * (A is +0 either way). */
void dojet_next(void)
{
    switch (ccs_at(ADDRESS_OF(SPNDX))) {
    case POSITIVE: continue_at(JLOOP); return;
    case IS_PLUS_ZERO: continue_at(T6PROG); return;
    case NEGATIVE: set_A(ZERO); break; /* ZEROCMDS */
    default: break;
    }
    ts(ADDRESS_OF(TAU));
    ts(ADDRESS_OF(TAU1));
    ts(ADDRESS_OF(TAU2));
    continue_at(T6PROG);
}

/* ---- T6PROG -- p. 1036 ---- */

/* T6PROG: "WHEN THE ROTATION COMMANDS (TAUS) HAVE BEEN DETERMINED",
 * JETADDR, the 2CADR of JETSLECT, the jet selection logic. */
void t6prog(void)
{
    dca(ADDRESS_OF(JETADDR));
    t6prog_t5loc();
}

/* T6PROG +2: "RESET T5LOC FOR PHASE3", and RESUME. */
void t6prog_t5loc(void)
{
    dxch(ADDRESS_OF(T5LOC));
    continue_at(RESUME);
}

#endif
