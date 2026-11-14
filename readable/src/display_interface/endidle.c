/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, part 7: a flashing display waiting in
 * ENDIDLE, the astronaut's answer back to the user, and the displays that
 * come next (LM pages 1367-1372, CM pages 1478-1482), bank 10. The faithful
 * C is c64-port/src/lanec_di_*.c.
 *
 * "ALL FLASHING DISPLAYS HAVE 3 RETURNS TO THE USER FROM ASTRONAUT
 * RESPOSES.  A TERMINATE (V34) BRANCHES TO THE USER'S CALL CADR +1.  A
 * PROCEED (V33) BRANCHES TO THE USER'S CALL CADR +2.  AN ENTER OR RECYCLE
 * (V32) BRANCHES TO THE USER'S CALL CADR +3." OUTHERE holds which: 0, 1
 * or 2 (-1 for none). */
#include "display_interface.h"

/* CLEANEND -- LM p. 1367, CM p. 1478: no verb and noun: a job (FINDVAC,
 * "ONE LOWER THAN DISPLAYS SLEEPING") to JAMTERM. */
void cleanend(void)
{
    set_A(PRIO32);
    tc_at((word)(CLEANEND + 1u)); /* TC FINDVAC; 2CADR JAMTERM */
    continue_at(FINDVAC);
}

/* ISITPRIO -- LM p. 1367, CM p. 1478: "IS PINBRFLG, MARKIDFLG SET": if
 * neither, PRIOBORT; else ENDOFJOB. */
void isitprio(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(ITISMASK);
    if (A_is_zero()) {
        continue_at(PRIOBORT);
        return;
    }
    continue_at(ENDOFJOB);
}

/* RESTSLEP -- LM p. 1368, CM p. 1479: "SET NVSLEEP BITS" (UPENT2). */
void restslep(void)
{
    ca(ADDRESS_OF(GENMASK));
    mask(ASTROMSK);
    tc_at((word)(RESTSLEP + 2u)); /* TC UPENT2; OCT24100 OCT 24100 */
    continue_at(UPENT2);
}

/* REST -- LM p. 1368, CM p. 1478: NVMONOPT found the DSKY busy. "IS
 * SOMEONE IN ENDIDLE": "YES", ENDOFJOB; if not RESTSLEP. */
void rest(void)
{
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE: continue_at(ENDOFJOB); return; /* "YES" */
    case IS_PLUS_ZERO: restslep(); return;
    case NEGATIVE: continue_at(ENDOFJOB); return;
    case IS_MINUS_ZERO: restslep(); return; /* the next word is RESTSLEP's first */
    }
}

/* RESTSLEP +4 -- LM p. 1368, CM p. 1479, where UPENT2 returns: NVSUBUSY puts
 * the job to sleep until the DSKY is free, then wakes it at the kind's
 * NVCADR (REDOPRIO, MARKPLAY, PLAYJUM1): "BUSY OR ABORT IF ILLEGAL". */
void restslep_after_upent2(void)
{
    ca(indexed(ADDRESS_OF(NVCADR), display_kind()));
    tc_at((word)(RESTSLEP + 6u)); /* TC NVSUBUSY */
    continue_at(NVSUBUSY);
}

/* FLASHSUB -- LM p. 1368, CM p. 1479: FLASHON. */
void flashsub(void)
{
    tc_at(FLASHSUB);
    continue_at(FLASHON);
}

/* FLASHSUB +1 -- LM p. 1368, CM p. 1479, where FLASHON returns: "COPINDEX
 * DESTROYED BY ENDIDLE" (kept in COPMPAC), and the kind's ENDIDLE bits up
 * (UPENT2; ITISMASK, "ENDIDLE ALLOW"). */
void flashsub_plus1(void)
{
    ca(ADDRESS_OF(COPINDEX));
    ts(ADDRESS_OF(COPMPAC));
    ca(ADDRESS_OF(GENMASK));
    mask(IDLEMASK);
    tc_at((word)(FLASHSUB + 5u)); /* TC UPENT2; ITISMASK OCT 40040 */
    continue_at(UPENT2);
}

/* CLEANEND +4 -- LM p. 1367, CM p. 1478, where FINDVAC returns: TCF FLASHSUB
 * +1. */
void cleanend_after_findvac(void)
{
    flashsub_plus1();
}

/* ITISMASK +012 -- LM p. 1368, CM p. 1479: TC ENDIDLE, the astronaut's
 * answer. */
void goidle(void)
{
    tc_at((word)(IDLERET1 - 1u));
    continue_at(ENDIDLE);
}

/* UNSETR1 -- LM p. 1369, CM p. 1480: "RESET REPEAT AND RETURN REQUEST"
 * (the kind's bit in R1SAVE), and SUPERSW to superbank 0, "205 ONLY
 * MARKBRAN USERS IN". */
void unsetr1(void)
{
    cs(indexed(ADDRESS_OF(BIT3), display_kind()));
    mask_at(ADDRESS_OF(R1SAVE));
    ts(ADDRESS_OF(R1SAVE));
    set_A(ZERO);
    tc_at((word)(UNSETR1 + 5u)); /* TC SUPERSW */
    continue_at(SUPERSW);
}

/* ITISMASK +1 -- LM p. 1368, CM p. 1479, where UPENT2 returns: "IS THIS A
 * REPEAT AND RETURN DISPLAY" (R1SAVE's bit for the kind): "YES", UNSETR1.
 * "SEE IF SOMEONE ALREADY IN ENDIDLE" (CADRSTOR): if so ISITPRIO, if not
 * ENDIDLE. */
void itismask_after_upent2(void)
{
    word kind = display_kind();

    ca(ADDRESS_OF(R1SAVE));
    mask_at(indexed(ADDRESS_OF(BIT3), kind));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        unsetr1();
        return;
    }
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE: isitprio(); return;
    case IS_PLUS_ZERO: goidle(); return; /* TCF +2 */
    case NEGATIVE: isitprio(); return;
    case IS_MINUS_ZERO: goidle(); return;
    }
}

/* TIMECHEK -- LM p. 1369, CM p. 1480: a PRIO display answered: "THE
 * ASTRONAUT MUST RESPOND TO A PRIORITY DISPLAY NO SOONER THAN 2 SECONDS
 * FROM THE TIME THE PROGRAM SENT OUT THE REQUEST". The time since
 * PRIOTIME (CCS, COM and AD OCT37776 take its magnitude across TIME1's
 * overflow), less 2 seconds: too soon, KEEPPRIO, the display again; else
 * NORMRET. */
void timechek(void)
{
    cs(ADDRESS_OF(TIME1));
    ad_at(ADDRESS_OF(PRIOTIME));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: com();            /* fall through */
    case IS_PLUS_ZERO: ad(OCT37776); /* fall through */
    case NEGATIVE: ad(ONE);          /* fall through */
    case IS_MINUS_ZERO: break;
    }
    ad(MINUS_2SEC);
    if (A_is_zero_or_minus()) {
        continue_at(KEEPPRIO);
        return;
    }
    normret();
}

/* ENDOUT -- LM p. 1369, CM p. 1479: the answer (A) to OUTHERE. "CHECK
 * NATURE OF ENDIDLE RETURN" (FLAGWRD4 bits 14 and 15): "PRIO ENDIDLE
 * RETURN", "NORMAL ENDIDLE RETURN", "MARK ENDIDLE RETURN". (An overflow
 * in A skips the CA.) */
void endout(void)
{
    if (!ts(ADDRESS_OF(OUTHERE))) ca(ADDRESS_OF(FLAGWRD4));
    mask(OCT60000);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: timechek(); return; /* "PRIO ENDIDLE RETURN" */
    case IS_PLUS_ZERO: normret(); return;
    case NEGATIVE: markret(); return;
    case IS_MINUS_ZERO: timechek(); return; /* the next word is TIMECHEK's first */
    }
}

/* OKTOENT -- LM p. 1369, CM p. 1479: an enter (2). */
void oktoent(void)
{
    set_A(TWO);
    endout();
}

/* IDLERET1 -- LM p. 1368, CM p. 1479: where ENDIDLE returns on a
 * terminate: TCF TERMATE. */
void idleret1(void)
{
    termate();
}

/* IDLERET1 +1 -- LM p. 1368, CM p. 1479: "ENDIDLE RETURNS HERE ON
 * PROCEED". */
void idleret1_plus1(void)
{
    proceed();
}

/* IDLERET1 +2 -- LM p. 1368, CM p. 1479: an enter, or a load verb: "V21 OR
 * V22 OR V23 ON DSKY" (the verb less LOWLOAD, 22, then DIM, is zero):
 * LOADITIS; else OKTOENT. */
void idleret1_plus2(void)
{
    set_A(negate(LOWLOAD));
    ad_at(ADDRESS_OF(MPAC)); /* "VERBREG" */
    dim(A_ADDRESS);
    if (A_is_zero()) {
        loaditis();
        return;
    }
    oktoent();
}

/* NORMWAKE -- LM p. 1369, CM p. 1480: WAKEPLAY for the NORMAL display. */
void normwake(void)
{
    set_A(ONE);
    continue_at(WAKEPLAY);
}

/* ENDRET +4 -- LM p. 1369, CM p. 1480: back to the user at the kind's
 * CADRFLSH plus the answer (in MPAC +3), the kind's ENDIDLE and pinbranch
 * bits down: "REMOVE ENDIDLE AND PINBRANCH BITS". (An overflow in A skips
 * the CA.) */
void endret_plus4(void)
{
    word kind = fetch(ADDRESS_OF(COPMPAC));

    if (kind > 2u) not_modelled();
    ad_at(indexed(ADDRESS_OF(CADRFLSH), kind));
    if (!ts(ADDRESS_OF(MPAC) + 3u)) ca(ADDRESS_OF(GENMASK));
    mask(PINIDMSK);
    tc_at((word)(ENDRET + 011u)); /* TC DOWNENT2; PINIDMSK OCT 74044 */
    continue_at(DOWNENT2);
}

/* ENDRET -- LM p. 1369, CM p. 1480: OUTHERE "IS - MEANS ENDOFJOB TO
 * ENDRET"; else ENDRET +4 with the answer in A. */
void endret(void)
{
    switch (ccs_at(ADDRESS_OF(OUTHERE))) {
    case POSITIVE: ad(ONE); break; /* CCS took one off; TCF +2 */
    case IS_PLUS_ZERO: break;      /* TCF +2: "NORMAL ENDIDLE EXIT" */
    case NEGATIVE: continue_at(ENDOFJOB); return;
    case IS_MINUS_ZERO: break;
    }
    endret_plus4();
}

/* PINIDMSK +1 -- LM p. 1369, CM p. 1480, where DOWNENT2 returns: "BLANK
 * EVERYTHING EXCEPT MM" (NVSUB). */
void pinidmsk_after_downent2(void)
{
    set_A(negate(THREE));
    tc_at((word)(ENDRET + 014u)); /* TC NVSUB */
    continue_at(NVSUB);
}

/* ENDIT -- LM p. 1369, CM p. 1480: "RETURN TO USER" priority (PRIOCHNG). */
void endit(void)
{
    ca(ADDRESS_OF(USERPRIO));
    mask(PRIO37);
    tc_at((word)(ENDIT + 2u)); /* TC PRIOCHNG */
    continue_at(PRIOCHNG);
}

/* PINIDMSK +3 -- LM p. 1369, CM p. 1480, where NVSUB returns: TCF +1,
 * ENDIT. */
void pinidmsk_after_nvsub(void)
{
    endit();
}

/* ENDIT +3 -- LM p. 1369, CM p. 1480, where PRIOCHNG returns: to the user,
 * at MPAC +3 (BANKJUMP). */
void endit_after_priochng(void)
{
    ca(ADDRESS_OF(MPAC) + 3u);
    continue_at(BANKJUMP);
}

/* IMMEDRET -- LM p. 1370, CM p. 1481: to the kind's CADRFLSH plus A. */
void immedret(void)
{
    ad_at(indexed(ADDRESS_OF(CADRFLSH), display_kind()));
    continue_at(BANKJUMP);
}

/* UNSETR1 +6 -- LM p. 1370, CM p. 1481 (IMMEDRET -1), where SUPERSW returns:
 * "RETURN TO USER" at the immediate return, the call CADR +4. */
void unsetr1_after_supersw(void)
{
    set_A(THREE);
    immedret();
}

/* TERMATE -- LM p. 1370, CM p. 1481: "ASTRONAUT TERMINATE (V34) RETURNS
 * TO" the call CADR +1 (0). */
void termate(void)
{
    set_A(ZERO);
    endout();
}

/* LINUSCHR -- LM p. 1370, CM p. 1481: "IS THIS A LINUS" (bit 14 of the
 * flags): "NO", by Q+1 to the word after the caller's return. "YES, IS IT
 * ALREADY IN ENDIDLE" (the same CADR as the kind's CADRFLSH): "NO", TC Q.
 * "IS THE ASTRONAUT BUSY": "END THE NEW DISPLAY", else TC Q. */
void linuschr(void)
{
    cs(ADDRESS_OF(PLAYTEM4));
    mask(BIT14);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(Q_PLUS_1);
        return;
    }
    cs(ADDRESS_OF(PLAYTEM3));
    ad_at(indexed(ADDRESS_OF(CADRFLSH), fetch(ADDRESS_OF(COPINDEX))));
    if (!A_is_zero()) {
        tc_through(Q_ADDRESS);
        return;
    }
    switch (ccs_at(ADDRESS_OF(DSPLOCK))) {
    case POSITIVE:
        tc_at((word)(LINUSCHR + 013u)); /* TC ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    case IS_PLUS_ZERO: tc_through(Q_ADDRESS); return;
    case NEGATIVE: proceed(); return;     /* never. The next words are PROCEED's: CAF ONE */
    case IS_MINUS_ZERO: endout(); return; /* never. And its TCF ENDOUT */
    }
}

/* PROCEED -- LM p. 1370, CM p. 1481: "ASTRONAUT PROCEED (V33) RETURNS" to
 * the call CADR +2 (1). */
void proceed(void)
{
    set_A(ONE);
    endout();
}

/* MARKRET -- LM p. 1370, CM p. 1481: a mark's answer: the mark bits (2
 * and 3) down in FLAGWRD4, and ENDRET. */
void markret(void)
{
    set_A(negate(SIX));
    mask_at(ADDRESS_OF(FLAGWRD4));
    inhint();
    ts(ADDRESS_OF(FLAGWRD4));
    relint();
    endret();
}

/* MARKOVER -- LM p. 1371, CM p. 1481: the mark is over, OUTHERE -1 ("MEANS
 * ENDOFJOB TO ENDRET"). "IS NORMAL OR PRIO IN ENDIDLE": NORMBNCH puts it
 * back; if not, NORMRET. */
void markover(void)
{
    set_A(MINUS1);
    ts(ADDRESS_OF(OUTHERE));
    ca(ADDRESS_OF(FLAGWRD4));
    mask(PRIO30);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(NORMBNCH);
        return;
    }
    normret();
}

/* NORMRET +024 -- LM p. 1371, CM p. 1482, where NOVAC returns: TCF
 * ENDRET. */
void normret_after_novac(void)
{
    endret();
}

/* NORMRET +014 -- LM p. 1371, CM p. 1482: a NORMAL display waits in NVSAVE:
 * a job (NOVAC, priority 15) to start it at PLAYJUM1. ENDRET. */
void normret_plus014(void)
{
    ca(ADDRESS_OF(NVSAVE));
    if (A_is_zero()) {
        endret();
        return;
    }
    set_A(PRIO15);
    inhint();
    tc_at((word)(NORMRET + 021u)); /* TC NOVAC; 2CADR PLAYJUM1 */
    continue_at(NOVAC);
}

/* NORMRET +010 -- LM p. 1371, CM p. 1482: "NO, WAS IT A FLASH REQUEST" "OR A
 * GODSPRET" (EBANKTEM's bits 6 and 4): "YES", ENDRET; if not NORMRET
 * +014. */
void normret_plus010(void)
{
    ca(ADDRESS_OF(EBANKTEM));
    mask(OCT50);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        endret();
        return;
    }
    normret_plus014();
}

/* NORMRET +4 -- LM p. 1371, CM p. 1482: "IS NORMAL INTERRUPTED OR WAITING":
 * "YES", NORMWAKE; if not NORMRET +010. */
void normret_plus4(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(BITS4_PLUS_10);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        normwake();
        return;
    }
    normret_plus010();
}

/* NORMRET -- LM p. 1371, CM p. 1482. "LASTPLAY CHECKS TO SEE IF (1) THE
 * LAST NORMAL DISPLAY WAS EITHER INTERRUPTED BY A PRIO OR A MARK" or "(2)
 * THE LAST NORMAL DISPLAY WAS REQUESTED WHILE A HIGHER PRIORITY DISPLAY
 * WAS GOING". First "IS MARK SLEEPING" "OR WAITING": MARKWAKE. */
void normret(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(BITS5_PLUS_11);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(MARKWAKE);
        return;
    }
    normret_plus4();
}

/* MARSLEEP -- LM p. 1371, CM p. 1482: a mark that must wait. "IS MARK
 * ALREADY IN": "YES", ENDOFJOB; else GOSLEEPS. */
void marsleep(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(BITS5_PLUS_11);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(ENDOFJOB);
        return;
    }
    continue_at(GOSLEEPS);
}

/* LOADITIS -- LM p. 1371, CM p. 1482: a load verb answered the display:
 * its noun (MPAC +1, "NOUNREG") the display's: "NO, THEN LOAD IS VALID",
 * OKTOENT. Else "YES, ACCEPT LOAD BUT ASK FOR LAST AGAIN": PINBRNCH. */
void loaditis(void)
{
    word kind = fetch(ADDRESS_OF(COPMPAC));

    if (kind > 2u) not_modelled();
    ca(indexed(ADDRESS_OF(NVWORD), kind));
    mask(LOW7);
    com();
    ad_at(ADDRESS_OF(MPAC) + 1u);
    if (A_is_zero()) {
        oktoent();
        return;
    }
    continue_at(PINBRNCH);
}

/* ERASER -- LM p. 1372, CM p. 1482: "BLANK EVERYTHING EXCEPT MM" (NVSUB). */
void eraser(void)
{
    set_A(negate(THREE));
    tc_at((word)(ERASER + 1u)); /* TC NVSUB */
    continue_at(NVSUB);
}

/* ERASER +2 and +3 -- LM p. 1372, CM p. 1482, NVSUB's returns: ENDOFJOB. */
void eraser_after_nvsub(void)
{
    continue_at(ENDOFJOB);
}
