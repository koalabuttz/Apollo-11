/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, the state vector moves: MOVEACSM and
 * MOVEPCSM (LM p. 1211, CM p. 1315), MOVEALEM and MOVEPLEM (LM p. 1212,
 * CM p. 1316), and SETBANK (LM p. 1213, CM p. 1317). */
#include "integration_init.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

/* SETBANK, from the TC SETBANK at "at": BBANK from INTBANK ("BBCON
 * INTEGRV": the integration's F-bank and its E-bank, RRECT's), A 41
 * (FORTYONE), and back (TC Q). */
static void setbank(word at)
{
    tc_at(at);
    set_A(INTBANK);                        /* CAF INTBANK */
    if (ts(BBANK_ADDRESS)) not_modelled(); /* (CAF leaves no overflow) */
    set_A(FORTYONE);                       /* CAF FORTYONE */
    tc_through(Q_ADDRESS);                 /* TC Q */
}

/* A move from the routine at "at": SETBANK, then words 41 down to 0 of
 * "from" to "to" (INDEX DIFEQCNT), DIFEQCNT counting down (CCS; "IS
 * TRANSFER COMPLETE"), and DANZIG ("COMPLETE -- RETURN"). */
static void move(word at, unsigned from, unsigned to)
{
    setbank(at);
    for (;;) {
        if (ts(ADDRESS_OF(DIFEQCNT))) not_modelled(); /* (41 to 0: no overflow) */
        ca(indexed(from, fetch(ADDRESS_OF(DIFEQCNT))));
        if (ts(indexed(to, fetch(ADDRESS_OF(DIFEQCNT))))) not_modelled(); /* (after CA) */
        enum sign count = ccs_at(ADDRESS_OF(DIFEQCNT));
        if (count == POSITIVE) continue;           /* TCF MOVE... +1, "NO-LOOP" */
        if (count != IS_PLUS_ZERO) not_modelled(); /* (it counts down from 41) */
        break;
    }
    tc_at((word)(at + 010u)); /* TC DANZIG */
    continue_at(DANZIG);
}

/* MOVEACSM: "TRANSFERS RRECT TO RRECT +41 TO RRECTCSM TO RRECTCSM +41". */
void moveacsm(void)
{
    move(MOVEACSM, ADDRESS_OF(RRECT), ADDRESS_OF(RRECTCSM));
}

/* MOVEPCSM: RRECTCSM to RRECT ("PTOACSM TRANSFERS RRECTCSM TO RRECTCSM +41
 * TO RRECT TO RRECT +41"). */
void movepcsm(void)
{
    move(MOVEPCSM, ADDRESS_OF(RRECTCSM), ADDRESS_OF(RRECT));
}

/* MOVEALEM: "TRANSFERS RRECT TO RRECT +41 TO RRECTLEM TO RRECTLEM +41". */
void movealem(void)
{
    move(MOVEALEM, ADDRESS_OF(RRECT), ADDRESS_OF(RRECTLEM));
}

/* MOVEPLEM: RRECTLEM to RRECT ("PTOALEM TRANSFERS RRECTLEM TO RRECTLEM +41
 * TO RRECT TO RRECT +41"). */
void moveplem(void)
{
    move(MOVEPLEM, ADDRESS_OF(RRECTLEM), ADDRESS_OF(RRECT));
}
