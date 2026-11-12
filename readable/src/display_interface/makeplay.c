/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, part 3: MAKEPLAY, which decides whether a
 * display may go out now (LM pages 1358-1360, CM pages 1469-1471), bank 10.
 * The faithful C is c64-port/src/lanec_di_*.c.
 *
 * "DISPLAY PRIORITIES WORK AS FOLLOWS": "PRIORITY DISPLAYS INTERRUPT ALL
 * OTHER TYPES OF INTERNAL DISPLAYS.  A PRIORITY DISPLAY INTERRUPTING
 * ANOTHER PRIORITY DISPLAY WILL CAUSE AN ABORT UNLESS BIT14 IS SET FOR
 * THE LINUS ROUTINE." A display that may not go out yet is put to sleep
 * (GOSLEEPS); one that takes the DSKY from another puts that one to sleep
 * in its place (JOBXCHS). */
#include "display_interface.h"

/* PRIOPLAY -- LM p. 1360, CM p. 1471: the kind (A) to COPINDEX, and GOPLAY
 * (NVDSP). An overflow in A skips to PRIOPLAY +2, which is EXDSPRET. */
void prioplay(void)
{
    if (ts(ADDRESS_OF(COPINDEX))) {
        continue_at(EXDSPRET);
        return;
    }
    continue_at(GOPLAY);
}

/* PLAYJUM1 -- LM p. 1360, CM p. 1471: the NORMAL display (2). */
void playjum1(void)
{
    set_A(TWO);
    prioplay();
}

/* KEEPPRIO -- LM p. 1359, CM p. 1470: "START UP PRIO DISPLAY". */
void keepprio(void)
{
    set_A(ZERO);
    prioplay();
}

/* REDOPRIO -- LM p. 1359, CM p. 1470: "SAVE TIME PRIODSP SENT OUT", and
 * KEEPPRIO. */
void redoprio(void)
{
    ca(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(PRIOTIME));
    keepprio();
}

/* SETMARK -- LM p. 1359, CM p. 1469: JOBXCHS with the MARK asleep (0). */
void setmark(void)
{
    set_A(ZERO);
    continue_at(JOBXCHS);
}

/* SETNORM -- LM p. 1359, CM p. 1470: JOBXCHS with the NORMAL asleep (1). */
void setnorm(void)
{
    set_A(ONE);
    continue_at(JOBXCHS);
}

/* OKTOCOPY -- LM p. 1359, CM p. 1470: the PRIO display may go out. Its
 * words to the PRIO set (COPYNORM), WITCHONE (who has the DSKY), and wake
 * that one (JOBWAKE). */
void oktocopy(void)
{
    tc_at(OKTOCOPY);
    copynorm();
    tc_at((word)(OKTOCOPY + 1u));
    witchone();
    tc_at((word)(OKTOCOPY + 2u)); /* TC JOBWAKE */
    continue_at(JOBWAKE);
}

/* OKTOCOPY +3 -- LM p. 1359, CM p. 1470, where JOBWAKE returns: the woken
 * job to ENDOFJOB (XCHTOEND), and REDOPRIO. */
void oktocopy_after_jobwake(void)
{
    tc_at((word)(OKTOCOPY + 3u));
    xchtoend();
    redoprio();
}

/* ASKIFNRM -- LM p. 1359, CM p. 1469: "NORMAL ACTIVE" (FLAGWRD4 "BITS
 * 13+8"): if so SETNORM, if not OKTOCOPY. */
void askifnrm(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(OCT10200);
    if (A_is_zero()) {
        oktocopy(); /* "NO" */
        return;
    }
    setnorm();
}

/* HIPRIO -- LM p. 1358, CM p. 1469: "MARK ACTIVE": if so SETMARK, if not
 * ASKIFNRM. */
void hiprio(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(OCT40400);
    if (A_is_zero()) {
        askifnrm(); /* "NO" */
        return;
    }
    setmark();
}

/* PRIOBORT -- LM p. 1360, CM p. 1470: abort 1502. "PRIOBORT IS BRANCHED
 * TO WHEN (1) A NORMAL DISPLAY IS REQUESTED AND ANOTHER NORMAL DISPLAY IS
 * ALREADY ACTIVE" or "(2) A PRIORITY DISPLAY IS REQUESTED WHEN ANOTHER
 * PRIORITY DISPLAY IS ALREADY ACTIVE". */
void priobort(void)
{
    tc_at(PRIOBORT); /* TC POODOO; OCT 1502 */
    continue_at(POODOO);
}

/* MAKEPRIO +4 -- LM p. 1358, CM p. 1469, LINUSCHR's Q+1 return (not a
 * LINUS): "IS PRIO IN ENDIDLE OR BUSY": "YES, ABORT"; if not HIPRIO. */
void makeprio_after_linuschr(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(OCT20100);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        priobort();
        return;
    }
    hiprio();
}

/* MAKEPRIO -- LM p. 1358, CM p. 1469: a PRIO display (index 0). LINUSCHR
 * comes back to MAKEPRIO +3 for a LINUS (TCF HIPRIO, "LINUS RETURN"), or
 * goes on by Q+1 to MAKEPRIO +4, or to ENDOFJOB. */
void makeprio(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(COPINDEX));
    tc_at((word)(MAKEPRIO + 2u)); /* TC LINUSCHR */
    linuschr();
    if (Z == MAKEPRIO + 3u) hiprio();
}

/* OKTOPLAY +015 -- LM p. 1360, CM p. 1471, where JOBWAKE returns: the woken
 * job to ENDOFJOB (XCHTOEND), and PLAYJUM1. */
void oktoplay_after_jobwake(void)
{
    tc_at((word)(OKTOPLAY + 015u));
    xchtoend();
    playjum1();
}

/* OKTOPLAY -- LM p. 1360, CM p. 1471: a NORMAL display may go out. Its
 * words to the NORMAL set (COPIES2), the user's priority and superbank to
 * RESTREG for a restart, and "PRIO OR MARK GOING": GOSLEEPS. If not,
 * WITCHONE ("IS IT NVSUB BUSY, ENDIDLE OR NOONE") and JOBWAKE. */
void oktoplay(void)
{
    tc_at(OKTOPLAY);
    copies2();
    ca(ADDRESS_OF(USERPRIO));
    ror_superbnk();
    ts(ADDRESS_OF(RESTREG));
    ca(ADDRESS_OF(FLAGWRD4));
    mask(PMMASK);
    if (ccs_register(A_ADDRESS) != IS_PLUS_ZERO) {
        continue_at(GOSLEEPS);
        return;
    }
    tc_at((word)(OKTOPLAY + 013u));
    witchone();
    tc_at((word)(OKTOPLAY + 014u)); /* TC JOBWAKE */
    continue_at(JOBWAKE);
}

/* IFLEGAL +4 -- LM p. 1359, CM p. 1470, LINUSCHR's Q+1 return (not a LINUS):
 * a NORMAL display may go out if the last one was not a flashing one
 * (EBANKTEM bit 4), or if not, unless "ARE ANY NORMS ASLEEP" finds one; then
 * PRIOBORT. */
void iflegal_after_linuschr(void)
{
    cs(ADDRESS_OF(EBANKTEM));
    mask(BIT4);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        oktoplay(); /* "NO" */
        return;
    }
    ca(ADDRESS_OF(FLAGWRD4));
    mask(NBUSMASK);
    if (A_is_zero()) {
        oktoplay(); /* "NO" */
        return;
    }
    priobort();
}

/* IFLEGAL -- LM p. 1359, CM p. 1470: a NORMAL display (index 2). LINUSCHR
 * comes back to IFLEGAL +3 for a LINUS (TCF OKTOPLAY, "LINUS RETURN"). */
void iflegal(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(COPINDEX));
    tc_at((word)(IFLEGAL + 2u)); /* TC LINUSCHR */
    linuschr();
    if (Z == IFLEGAL + 3u) oktoplay();
}

/* MAKEPLAY -- LM p. 1359, CM p. 1470: the user's priority to USERPRIO,
 * and "RAISE PRIORITY FOR FAST JOBS AFTER WAKE" (PRIOCHNG to 33). */
void makeplay(void)
{
    ca(ADDRESS_OF(PRIORITY));
    mask(PRIO37);
    ts(ADDRESS_OF(USERPRIO));
    set_A(PRIO33);
    tc_at((word)(MAKEPLAY + 4u)); /* TC PRIOCHNG */
    continue_at(PRIOCHNG);
}

/* MAKEPLAY +5 -- LM p. 1359, CM p. 1470, where PRIOCHNG returns: "IS IT MARK
 * OR PRIO OR NORM" (PLAYTEM4 bits 15 and 7): "ITS PRIO", IFLEGAL for a
 * NORMAL display, "ITS MARK". MAKEMARK's ways on to SETNORM and PRIOPLAY are
 * taken here too, as the faithful port takes them. */
void makeplay_after_priochng(void)
{
    ca(ADDRESS_OF(PLAYTEM4));
    mask(BITS15_PLUS_7);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: makeprio(); return; /* "ITS PRIO" */
    case IS_PLUS_ZERO: iflegal(); return;
    case NEGATIVE: break;                  /* TCF MAKEMARK: "ITS MARK" */
    case IS_MINUS_ZERO: iflegal(); return; /* the next word is IFLEGAL's first */
    }
    continue_at(MAKEMARK);
    makemark();
    if (Z == SETNORM)
        setnorm();
    else if (Z == PRIOPLAY)
        prioplay();
}
