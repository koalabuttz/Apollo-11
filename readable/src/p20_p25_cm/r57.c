/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 3 (Comanche055 pp. 570-571): R57, the "OPTICS
 * CALIBRATION ROUTINE" (bank 37).
 *
 * P23 calls R57 to measure the trunnion bias: the angle the sextant's
 * trunnion reads when the shaft's line of sight and the landmark's are
 * superimposed, which "MAY NOT BE EXACTLY ZERO BECAUSE OF UNEVEN HEATING OF
 * THE OPTICS". R57 keeps its return in EGRESS, aborts P23 if the marking
 * system or an extended verb is busy, and sets V59FLAG so that the mark is
 * a calibration mark. Verb 59 asks for it (GOMARKFR); MARKDISP, where
 * SXTMARK comes after the mark, shows the trunnion angle with verb 06 noun
 * 87, and on proceed R57B stores it in TRUNBIAS. ENDR57 clears V59FLAG,
 * ends the marking (ENDMARK, a job of its own) and goes back through
 * EGRESS.
 *
 * Each call out of the file (BANKCALL, UPFLAG, DOWNFLAG, BLANKET, NOVAC)
 * leaves Z at the routine and Q at the word after the TC, where the routine
 * comes back to; ENDOFJOB, GOTOPOOH and BAILOUT don't come back. A display
 * through GOMARKFR comes back to the word after its CADR on terminate, the
 * next on proceed, the next on enter (recycle) and the next on its
 * immediate return. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r57a(void);
static void r57b(void);
static void endr57(void);
static void p23abrt(void);

/* R57 -- p. 570: the return in EGRESS, and out of the interpreter. */
static const word r57_words[] = {
    ORDERS(STQ, EXIT),
    ADDRESS(EGRESS),
};

/* R57 +2 -- after the EXIT: "BIT2 = MARKING SYSTEM IN USE", "BIT3 =
 * EXTENDED VERB IN PROGRESS" (EXTVBACT's bits). Either "SET, THEREFORE
 * ABORT"; else the marking system's bit set. (A CAF leaves no overflow for
 * the TS to skip on, and the MASK no negative for the CCS.) */
void r57_after_exit(void)
{
    set_A(EBANK7);
    if (ts(EBANK_ADDRESS)) not_modelled();
    set_A(SIX);
    mask_at(ADDRESS_OF(EXTVBACT));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: /* "SET, THEREFORE ABORT" */
        tc_at(R57 + 7u);
        p23abrt();
        break;
    case IS_PLUS_ZERO: r57_not_set(); break;
    default: not_modelled();
    }
}

/* R57 +010: "NOT SET": "SET IT", and on to R57A. */
void r57_not_set(void)
{
    set_A(BIT2);
    ads(ADDRESS_OF(EXTVBACT));
    r57a();
}

/* R57A -- p. 570. "SET V59FLAG (BIT 12 FLAG 5 TO INDICATE", "CALIBRATION
 * MARK". */
static void r57a(void)
{
    tc_to(R57A, UPFLAG); /* ADRES V59FLAG */
}

/* R57A +2: verb 59, the request for the calibration mark. */
void r57a_after_v59flag(void)
{
    set_A(V59NB);
    tc_to(R57A + 3u, BANKCALL); /* CADR GOMARKFR */
}

/* R57A +5: "TERMINATE". */
void r57a_terminate(void)
{
    tc_to(R57A + 5u, GOTOPOOH);
}

/* R57A +6: proceed, to ENDR57. */
void r57a_proceed(void)
{
    tc_at(R57A + 6u);
    endr57();
}

/* R57A +7: enter, to ENDR57 too. */
void r57a_enter(void)
{
    tc_at(R57A + 7u);
    endr57();
}

/* R57A +010: the display's immediate return: "BLANK OUT R1,R2,R3"
 * (BLANKET's bits 1 to 3), */
void r57a_immediate_return(void)
{
    set_A(SEVEN);
    tc_to(R57A + 011u, BLANKET);
}

/* R57A +012 -- p. 571: and the job ends. */
void r57a_after_blanket(void)
{
    tc_to(R57A + 012u, ENDOFJOB);
}

/* MARKDISP -- p. 571: "STORE TRUNNION ANGLE (OCDU)": verb 06 noun 87, by
 * GOMARKFR. */
void markdisp(void)
{
    set_A(V06N87NB);
    tc_to(MARKDISP + 1u, BANKCALL); /* CADR GOMARKFR */
}

/* MARKDISP +3: "TERMINATE". */
void markdisp_terminate(void)
{
    tc_to(MARKDISP + 3u, GOTOPOOH);
}

/* MARKDISP +4: "PROCEED". */
void markdisp_proceed(void)
{
    tc_at(MARKDISP + 4u);
    r57b();
}

/* MARKDISP +5: "ENTER (RECYCLE)", another mark. */
void markdisp_enter(void)
{
    tc_at(MARKDISP + 5u);
    r57a();
}

/* MARKDISP +6: the immediate return: "BLANK OUT R1,R3" (BLANKET's bits 1
 * and 3), */
void markdisp_immediate_return(void)
{
    set_A(FIVE);
    tc_to(MARKDISP + 7u, BLANKET);
}

/* MARKDISP +010: and the job ends. */
void markdisp_after_blanket(void)
{
    tc_to(MARKDISP + 010u, ENDOFJOB);
}

/* R57B: the trunnion angle (MRKBUF1 +5, "CONTAINS TRUNNION") less
 * 19.77DEG (OCT 61740) by MSU, which takes the difference of two CDU
 * readings in two's complement and gives it in ones', to TRUNBIAS.
 * 19.77DEG is put "INTO ERASABLE FOR MSU", in L. (A CA of a constant
 * leaves no overflow for the TS to skip on; an MSU's would be refused.) */
static void r57b(void)
{
    set_A(NINETEEN_77DEG);
    if (ts(L_ADDRESS)) not_modelled();
    ca(ADDRESS_OF(MRKBUF1) + 5u);
    msu_at(L_ADDRESS); /* EXTEND; MSU L */
    if (ts(ADDRESS_OF(TRUNBIAS))) not_modelled();
    endr57();
}

/* ENDR57: "RESET V59FLAG". */
static void endr57(void)
{
    tc_to(ENDR57, DOWNFLAG); /* ADRES V59FLAG */
}

/* ENDR57 +2: ENDMARK as a job of priority 14 (NOVAC): "THIS JOB CLEARS
 * BIT IN", "MARKING IN R57 SO R53 CAN TAKE OVER". (A CAF leaves no
 * overflow for the TS to skip on.) */
void endr57_after_v59flag(void)
{
    set_A(EBANK5);
    if (ts(EBANK_ADDRESS)) not_modelled();
    set_A(PRIO14);
    tc_to(ENDR57 + 5u, NOVAC); /* 2CADR ENDMARK */
}

/* ENDR57 +010: back through EGRESS, in interpretive code. */
void endr57_after_novac(void)
{
    tc_at(ENDR57 + 010u);
    intpret();
}

static const word endr57_words[] = {
    ORDER(GOTO),
    ADDRESS(EGRESS),
};

/* P23ABRT: P23's abort, alarm 1211. */
static void p23abrt(void)
{
    tc_to(P23ABRT, BAILOUT); /* OCT 01211 */
}

/* ---- The interpretive code ---- */

const struct interpretive_code R57_CODE = {037u, R57, COUNT(r57_words), r57_words};
const struct interpretive_code ENDR57_CODE = {037u, ENDR57 + 011u, COUNT(endr57_words),
                                              endr57_words};

#endif
