/* SPDX-License-Identifier: MIT */
/* UPDATE_PROGRAM.agc, part 1: the verbs V70-V73, in the extended verb bank
 * (43), and the check that P27 may run (LM page 1388, CM pages 1499-1500).
 * The faithful C is c64-port/src/lm_update.c and cm_update.c. */
#include "update_program.h"

/* V70UPDAT, V71UPDAT, V72UPDAT and V73UPDAT -- LM p. 1388, CM p. 1499.
 * "COMES HERE ON V70E" and so on: the verb less 70 (UP70-UP73, the
 * constants ZERO to THREE) in UPVERBSV, "SAVE UPVERB UNTIL IT'S OK TO
 * ENTER P27", and TESTXACT: "GRAB DISPLAY IF AVAILABLE, OTHERWISE
 * TURN*OPERATOR ERROR* ON AND TERMINATE JOB". */
static void save_verb(word up)
{
    set_A(up);
    ts(ADDRESS_OF(UPVERBSV)); /* V73UPDAT +1 */
    tc_to((word)(V73UPDAT + 2u), TESTXACT);
}

void v70updat(void)
{
    save_verb(UP70); /* TCF V73UPDAT +1 */
}

void v71updat(void)
{
    save_verb(UP71);
}

void v72updat(void)
{
    save_verb(UP72);
}

void v73updat(void)
{
    save_verb(UP73);
}

#if COMANCHE055
/* CKMDMORE -- CM p. 1500, with MODREG neither +0 nor -0: "NOW CHECK FOR
 * PROGRAM WHICH CAN BE INTERRUPTED BY P27." True if it returned (TC Q,
 * "ALLOW UPDATE TO PROCEED"): the computer is the CM's ("CHECK IF
 * COMPUTER IS LGC": FLAGWRD5 bit 8 is set in the CM) and the program is
 * P02 (MODREG with no bit but bit 2). False if it went on to UPERROR: "IT'S THE LEM +
 * MODE IS NOT P00" (UPERLEM), or "IT'S THE CMC AND MODE IS NOT P00 OR
 * P02" (UPERCMC). CS TWO; MASK MODREG leaves no -0, on which the AGC would
 * run UPERROR's CADR as an instruction. */
static bool ckmdmore(void)
{
    cs(ADDRESS_OF(FLAGWRD5));
    mask(BIT8);
    if (ccs_register(A_ADDRESS) == POSITIVE) return false; /* UPERLEM */
    set_A(negate(TWO));                                    /* CS TWO */
    mask_at(ADDRESS_OF(MODREG));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: return false; /* UPERCMC */
    case IS_PLUS_ZERO: return_through_q(); return true;
    case NEGATIVE: return false; /* (the word after TC Q is UPERROR) */
    case IS_MINUS_ZERO: break;
    }
    not_modelled();
    return false;
}
#endif

/* UPERROR -- LM p. 1388, CM p. 1500: "TURN ON 'OPERATOR ERROR' LIGHT" and
 * "GO TO COMMON UPDATE PROGRAM EXIT", out of the extended verb bank to
 * UPERROUT +2. */
static void uperror(void)
{
    tc_to(UPERROR, POSTJUMP); /* CADR UPERROUT +2 */
}

/* V73UPDAT +3 -- LM p. 1388, CM p. 1499, where TESTXACT returns with the
 * display grabbed: "CHECK IF UPDATE ALLOWED", "FIRST CHECK FOR MODREG = +0,
 * -0" ("+0 = P00, -0 = FRESH START"). Then "SAVE CURRENT MAJOR MODE" in
 * UPOLDMOD, UPVERB "TO INDICDATE TO P27 WHICH EXTENDED VERB CALLED IT",
 * UPCOUNT "TO 1", and POSTJUMP to UPPART2: "LEAVE EXTENDED VERB BANK AND GO
 * TO UPDATE PROGRAM(P27) BANK." */
void v73updat_after_testxact(void)
{
    ca(ADDRESS_OF(MODREG));
    if (!A_is_zero()) {
#if LUMINARY099
        uperror();
        return;
#else
        tc_at((word)(V73UPDAT + 6u)); /* TC CKMDMORE */
        if (!ckmdmore()) {
            uperror();
            return;
        }
#endif
    }
    ca(ADDRESS_OF(MODREG)); /* "UPDATE ALLOWED." */
    ts(ADDRESS_OF(UPOLDMOD));
    ca(ADDRESS_OF(UPVERBSV));
    ts(ADDRESS_OF(UPVERB));
    set_A(ONE);
    ts(ADDRESS_OF(UPCOUNT));
#if LUMINARY099
    tc_to((word)(V73UPDAT + 016u), POSTJUMP); /* CADR UPPART2 */
#else
    tc_to((word)(V73UPDAT + 015u), POSTJUMP);
#endif
}
