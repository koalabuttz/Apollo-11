/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, part 1: blanking and the mark displays (LM
 * pages 1354-1356, CM pages 1465-1467). BLANKET, ENDMARK and CLEARMRK are in
 * fixed-fixed; from MARKEND on, bank 10. The faithful C is
 * c64-port/src/lanec_di_*.c.
 *
 * "ALL EXTENDED VERBS AND MARK ROUTINES SHOULD USE EXTENDED VERB (MARK)
 * DISPLAYS." A mark entry stores its verb and noun in PLAYTEM1 and goes
 * on with its flags in A (bit 15 for a mark) to GOFLASH2, or for an R
 * routine to GODSPRS. "A MARK DISPLAY INTERRUPTS ANY NORMAL DISPLAY." */
#include "display_interface.h"

/* BLANKET -- LM p. 1354, CM p. 1465.
 * "CALLING SEQUENCE FOR BLANKING": A holds BITX, "X=1,2,3 BLANK R1,R2,R3
 * RESPECTIVELY". "THE USER MUST USE A DISPLAY ROUTINE THAT ENDS IN R
 * FIRST": that left the new display job's register set offset in MPAC +5
 * (BOTHJOBS), so the bits not yet in PLAYTEM4 are added into that job's
 * copy of it. */
void blanket(void)
{
    word job = fetch(ADDRESS_OF(MPAC) + 5u);

    if (job > 01376u - ADDRESS_OF(PLAYTEM4)) not_modelled(); /* past the job's words */
    if (!ts(ADDRESS_OF(MPAC) + 6u)) cs(ADDRESS_OF(PLAYTEM4));
    mask_at(ADDRESS_OF(MPAC) + 6u);
    ads(indexed(ADDRESS_OF(PLAYTEM4), job));
    tc_through(Q_ADDRESS); /* "RETURN TO USER HERE" */
}

/* ENDMARK -- LM p. 1354, CM p. 1465: TC POSTJUMP; CADR MARKEND. "ALL
 * EXTENDED VERB ROUTINES THAT HAVE AT LEAST ONE FLASHING DISPLAY MUST TCF
 * ENDMARK OR TCF ENDEXT WHEN FINISHED." */
void endmark(void)
{
    tc_at(ENDMARK);
    continue_at(POSTJUMP);
}

/* CLEARMRK +2 -- LM p. 1354, CM p. 1465: the extended verb display bit down
 * in FLAGWRD4 (bit 1). */
void clearmrk_plus2(void)
{
    inhint();
#if LUMINARY099
    set_A(negate(XDSPBIT));
#else
    set_A(negate(BIT1));
#endif
    mask_at(ADDRESS_OF(FLAGWRD4));
    ts(ADDRESS_OF(FLAGWRD4));
    relint();
    tc_through(Q_ADDRESS);
}

/* CLEARMRK -- LM p. 1354, CM p. 1465: EXTVBACT to +0 (no extended verb
 * active), and CLEARMRK +2. */
void clearmrk(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(EXTVBACT));
    clearmrk_plus2();
}

/* MARKEND -- LM p. 1354, CM p. 1465: TC CLEARMRK, and TCF MARKOVER. */
void markend(void)
{
    tc_at(MARKEND);
    clearmrk();
    continue_at(MARKOVER);
}

/* GOMARS -- LM p. 1355, CM p. 1466: "BIT15 SET FOR ALL MARK REQUESTS". */
void gomars(void)
{
    set_A(BIT15);
    goflash2();
}

/* GOMARK -- LM p. 1354, CM p. 1466: "ENTRANCE FOR MARK GODSP". */
void gomark(void)
{
    if (ts(ADDRESS_OF(PLAYTEM1))) { /* an overflow skips GOMARS's CAF */
        goflash2();
        return;
    }
    gomars();
}

/* GOMARKF -- LM p. 1355, CM p. 1466: "ENTRANCE FOR MARK GOFLASH", flags
 * "MARK, FLASH". */
void gomarkf(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), MARKFMSK);
    goflash2();
}

/* KLEENEX -- LM p. 1355, CM p. 1466: "CLEAN OUT EXTENDED VERBS", a mark
 * GOFLASH of verb and noun +0. */
void kleenex(void)
{
    set_A(ZERO);
    gomarkf();
}

/* MARKFORM -- LM p. 1355, CM p. 1466: "MARK, PERFORM, FLASH". */
void markform(void)
{
    set_A(MPERFMSK);
    goflash2();
}

/* GOMARK2 -- LM p. 1355, CM p. 1466: "MARK GOPERFS-3 AST. RETURNS". */
void gomark2(void)
{
    if (ts(ADDRESS_OF(PLAYTEM1))) { /* an overflow skips MARKFORM's CAF */
        goflash2();
        return;
    }
    markform();
}

/* GOMARK3 -- LM p. 1355, CM p. 1466: "USED FOR 3COMP DECIMAL PERFORM". */
void gomark3(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), MARK3MSK);
    goflash2();
}

/* GOMARK4 -- LM p. 1355, CM p. 1466: "MARK,PERFORM,FLASH,BLANK". */
void gomark4(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), MARK4MSK);
    goflash2();
}

/* GOMARKR -- LM p. 1355, CM p. 1466: "ENTRANCE FOR MARK GODSPR". */
void gomarkr(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BIT15);
    godspr2();
}

/* GOMARKFR -- LM p. 1355, CM p. 1466: "ENTRANCE FOR MARK GOFLASHR". */
void gomarkfr(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), MARKFMSK);
    godsprs();
}

/* GOMARK2R -- LM p. 1355, CM p. 1466: "MARK GOPERFS-3 AST. RETS+
 * IMMEDIATE RET." */
void gomark2r(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), MPERFMSK);
    godsprs();
}

/* GOMARK3R -- LM p. 1355, CM p. 1466. */
void gomark3r(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), MARK3MSK);
    godsprs();
}

/* MAKEMARK -- LM p. 1355, CM p. 1466: MAKEPLAY's mark. The display's
 * words copied into the MARK set (COPIES, index 1). "IS NORM OR PRIO BUSY
 * OR WAITING": CHKPRIO. "IS MARK SLEEPING DUE TO ASTRO BUSY": if so
 * ENDOFJOB, if not MARKPLAY. */
void makemark(void)
{
    set_A(ONE);
    tc_at(MAKEMARK + 1u); /* TC COPIES */
    copies();
    ca(ADDRESS_OF(FLAGWRD4));
    mask(OCT34300);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        chkprio();
        return;
    }
    ca(ADDRESS_OF(FLAGWRD4));
#if LUMINARY099
    mask(MRKNVBIT);
#else
    mask(BIT9);
#endif
    if (A_is_zero()) {
        markplay(); /* "NO" */
        return;
    }
    continue_at(ENDOFJOB);
}

/* MARKCOP -- LM p. 1356, CM p. 1467: "MARK INDEX" to PRIOPLAY. */
void markcop(void)
{
    set_A(ONE);
    continue_at(PRIOPLAY);
}

/* GOGOMARK -- LM p. 1356, CM p. 1467: a mark PERFORM display (MARKFLAG
 * bit 5) has its verb and noun complemented in MARKNV, which NVDSP undoes.
 * (An overflow in A skips MARKCOP's CAF.) */
void gogomark(void)
{
    cs(ADDRESS_OF(MARKFLAG));
    mask(BIT5);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        markcop();
        return;
    }
    cs(ADDRESS_OF(MARKNV));
    if (ts(ADDRESS_OF(MARKNV))) {
        continue_at(PRIOPLAY);
        return;
    }
    markcop();
}

/* MARKPLAY -- LM p. 1356, CM p. 1467: "RESET MARK OVER NORM, SET MARK"
 * (FLAGWRD4 bits 3 and 1), and GOGOMARK. */
void markplay(void)
{
    inhint();
    set_A(negate(FIVE));
    mask_at(ADDRESS_OF(FLAGWRD4));
    ad(ONE);
    ts(ADDRESS_OF(FLAGWRD4));
    relint();
    gogomark();
}

/* COPYPACS -- LM p. 1356, CM p. 1467: the kind's FLAGWRD4 bits (PRIOOCT)
 * to GENMASK, and its flags to TEMPOR2, "ACTIVE EBANK AND FLAG", and to
 * EBANK. */
void copypacs(void)
{
    word kind = display_kind();

    ca(indexed(ADDRESS_OF(PRIOOCT), kind));
    ts(ADDRESS_OF(GENMASK));
    ca(indexed(ADDRESS_OF(EBANKSAV), kind));
    ts(ADDRESS_OF(TEMPOR2));
    ts(EBANK_ADDRESS);
    tc_through(Q_ADDRESS);
}

/* COPYTOGO -- LM p. 1356, CM p. 1467: MPAC +2 back from MPAC2SAV, and
 * COPYPACS. */
void copytogo(void)
{
    display_kind();
    ca(ADDRESS_OF(MPAC2SAV));
    ts(ADDRESS_OF(MPAC) + 2u);
    copypacs();
}
