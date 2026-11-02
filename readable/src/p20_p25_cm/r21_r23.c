/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 3 (Comanche055 pp. 572-573): R21 and R23 for the CSM,
 * the sighting marks of rendezvous navigation (bank 37).
 *
 * The crew calls R21 by verb 57 and R23 by verb 54. R21CSM sets R21MARK,
 * so that MARKRUPT keeps each mark in MRKBUF1; both clear the mark buffers
 * (-1, "NO MARK"). With R23FLG set (COAS marking), verb 06 noun 94 shows
 * the shaft and trunnion first and verb 53 asks for each "ALT LOST SIGHT
 * MARK", whose time and CDU angles R23 reads itself; else verb 51 asks for
 * the marks. On proceed the last mark goes from MRKBUF1 to MRKBUF2 for R22
 * (GENTRAN), and R21END ends the marking: KLEENEX, R21MARK cleared, and
 * ENDMARK.
 *
 * Each call out of the file (BANKCALL, UPFLAG, DOWNFLAG, GENTRAN) leaves Z
 * at the routine and Q at the word after the TC, where the routine comes
 * back to (GENTRAN past its two ADRES words); ENDMARK doesn't come back. A
 * display through GOMARKF or GOMARK2 comes back to the word after its CADR
 * on terminate, the next on proceed and the next on enter. */
#include "p20_p25_cm.h"
#include "../agc/names.h"

#if COMANCHE055

static void r23csm_display(void);
static void r21csma(void);
static void r21end(void);

/* R21CSM -- p. 572. "SET R21MARK" ("BIT 14 FLAG 2"), and on to R23CSM. */
void r21csm(void)
{
    tc_to(R21CSM, UPFLAG); /* ADRES R21MARK */
}

/* R23CSM: MRKBUF1 and MRKBUF2 -1, no mark. "TEST R23FLG": "NOT SET
 * REGULAR R21 MARKING" (R21C1); set, "R23 BACKUP MARKING". (A CA of a
 * constant leaves no overflow for the TS to skip on.) */
void r23csm(void)
{
    set_A(NEGONE);
    if (ts(ADDRESS_OF(MRKBUF1))) not_modelled();
    if (ts(ADDRESS_OF(MRKBUF2))) not_modelled();
    ca(ADDRESS_OF(FLAGWRD1));
    mask(R23BIT);
    if (A_is_zero()) { /* EXTEND; BZF R21C1 */
        r21c1();
        return;
    }
    r23csm_display();
}

/* R23CSM +7: verb 06 noun 94, by GOMARKF: "DISPLAY SHAFT + TRUNNION". */
static void r23csm_display(void)
{
    set_A(V0694);
    tc_to(R23CSM + 010u, BANKCALL); /* CADR GOMARKF */
}

/* R23CSM +012: "TERM". */
void r23csm_terminate(void)
{
    tc_at(R23CSM + 012u);
    r21end();
}

/* R23CSM +013: "PROC": TC +2, to R23CSM1. */
void r23csm_proceed(void)
{
    tc_at(R23CSM + 013u);
    r23csm1();
}

/* R23CSM +014: "ENTER": TC -5, the display again. */
void r23csm_enter(void)
{
    tc_at(R23CSM + 014u);
    r23csm_display();
}

/* R23CSM1 -- p. 573: verb 53, by GOMARK2: "PERFORM ALT LOST SIGHT MARK". */
void r23csm1(void)
{
    set_A(V53);
    tc_to(R23CSM1 + 1u, BANKCALL); /* CADR GOMARK2 */
}

/* R23CSM1 +3: "V34: TERMINATE R23". */
void r23csm1_terminate(void)
{
    tc_at(R23CSM1 + 3u);
    r21end();
}

/* R23CSM1 +4: "PROCEED: END BACK UP MARKING (R23)". */
void r23csm1_proceed(void)
{
    r21csma(); /* TCF R21CSMA */
}

/* R23CSM1 +5: enter, a mark: "TRANSFER MRKBUF1 TO MRKBUF2" (GENTRAN, six
 * words), */
void r23csm1_mark(void)
{
    set_A(SIX);
    tc_to(R23CSM1 + 6u, GENTRAN); /* ADRES MRKBUF1; ADRES MRKBUF2 */
}

/* R23CSM1 +011: and the new mark into MRKBUF1: "READ TIME" (TIME2 and
 * TIME1), "READ CDU ANGLES" (CDUY, CDUZ and CDUX, to MRKBUF1 +2, +4 and
 * +6). GENTRAN holds interrupts; RELINT releases them. Then verb 53
 * again. (The CDU counters hold 15 bits: no overflow for a TS to skip
 * on.) */
void r23csm1_after_gentran(void)
{
    dca(ADDRESS_OF(TIME2)); /* EXTEND; DCA TIME2 */
    dxch(ADDRESS_OF(MRKBUF1));
    ca(ADDRESS_OF(CDUY));
    if (ts(ADDRESS_OF(MRKBUF1) + 2u)) not_modelled();
    ca(ADDRESS_OF(CDUZ));
    if (ts(ADDRESS_OF(MRKBUF1) + 4u)) not_modelled();
    ca(ADDRESS_OF(CDUX));
    if (ts(ADDRESS_OF(MRKBUF1) + 6u)) not_modelled();
    relint();
    tc_at(R23CSM1 + 023u);
    r23csm1();
}

/* R21C1: verb 51, by GOMARK2: the R21 marks. */
void r21c1(void)
{
    set_A(V51NB);
    tc_to(R21C1 + 1u, BANKCALL); /* CADR GOMARK2 */
}

/* R21C1 +3: "V34: TERMINATE R21". */
void r21c1_terminate(void)
{
    tc_at(R21C1 + 3u);
    r21end();
}

/* R21C1 +4: "PROCEED: END R21". */
void r21c1_proceed(void)
{
    r21csma(); /* TCF R21CSMA */
}

/* R21C1 +5: "RECYCLE". */
void r21c1_recycle(void)
{
    r21c1(); /* TCF R21C1 */
}

/* R21CSMA: MRKBUF1 "IF -1 NO MARK": "ZERO = NO MARK" (MRKBUF1 +1); else
 * "MARK THEREFORE TRANSFER IT TO MRKBUF2" (GENTRAN, six words, R21CSM1). */
static void r21csma(void)
{
    ca(ADDRESS_OF(MRKBUF1));
    ad(ONE);
    if (A_is_zero()) { /* EXTEND; BZF R21END */
        r21end();
        return;
    }
    set_A(SIX);
    tc_to(R21CSM1, GENTRAN); /* ADRES MRKBUF1; ADRES MRKBUF2 */
}

/* R21CSM1 +3: interrupts released, and on to R21END. */
void r21csm1_after_gentran(void)
{
    relint();
    r21end();
}

/* R21END: KLEENEX, */
static void r21end(void)
{
    tc_to(R21END, BANKCALL); /* CADR KLEENEX */
}

/* R21END +2: "RESET R21MARK" ("BIT 14 FLAG 2"), */
void r21end_after_kleenex(void)
{
    tc_to(R21END + 2u, DOWNFLAG); /* ADRES R21MARK */
}

/* R21END +4: and "END MARKING AND ENDJOB". */
void r21end_after_r21mark(void)
{
    tc_to(R21END + 4u, ENDMARK);
}

#endif
