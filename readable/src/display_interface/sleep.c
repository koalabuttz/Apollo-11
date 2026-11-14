/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, part 5: display jobs put to sleep and
 * woken, and INITDSP (LM pages 1363-1364, CM pages 1474-1475), bank 10. The
 * faithful C is c64-port/src/lanec_di_*.c.
 *
 * A display that may not go out yet sleeps under its kind's CADR
 * (WAKECADR: MARKPLAY for a MARK, PLAYJUM1 for a NORMAL), with its kind's
 * waiting bits up in FLAGWRD4. "ORDER OF WAITING DISPLAYS": "ASTRONAUT",
 * "PRIORITY", "INTERRUPTED MARK", "INTERRUPTED NORMAL", "MARK TO BE
 * REQUESTED", "MARK WAITING", "NORMAL WAITING". The flag routines UPENT2 and DOWNENT2 take their
 * mask from the word after the call (WAITMASK, IDLEMASK, XCHQADD,
 * MARKFMSK), and return past it. */
#include "display_interface.h"

/* FACEREG (MPAC): the kind asleep, 0 MARK or 1 NORMAL, the index into
 * WAKECADR, MARKOCT and the pair at BIT5. A -0 (CS ONE plus a MARK's 1)
 * indexes nothing, like +0. Another value is not a state the program
 * makes. */
static word sleeper(void)
{
    word kind = fetch(ADDRESS_OF(FACEREG));

    if (kind > 1u && kind != MINUS_ZERO) not_modelled();
    return kind;
}

/* GOSLEEPS -- LM p. 1363, CM p. 1474: the kind's waiting bits (PRIOOCT's
 * word, masked by WAITMASK) up in FLAGWRD4. */
void gosleeps(void)
{
    ca(indexed(ADDRESS_OF(PRIOOCT), display_kind()));
    mask(WAITMASK);
    tc_at((word)(GOSLEEPS + 3u)); /* TC UPENT2; WAITMASK OCT 3004 */
    continue_at(UPENT2);
}

/* XCHSLEEP -- LM p. 1363, CM p. 1474: wake the job asleep under the kind's
 * CADR ("FIND CADR IN JOB AREA"). */
void xchsleep(void)
{
    ca(indexed(ADDRESS_OF(WAKECADR), sleeper()));
    inhint();
    tc_at((word)(XCHSLEEP + 3u)); /* TC JOBWAKE */
    continue_at(JOBWAKE);
}

/* GOSLEEPS +5 -- LM p. 1363, CM p. 1474, where UPENT2 returns: the kind less
 * one (a MARK 0, a NORMAL 1) to FACEREG, and XCHSLEEP. */
void gosleeps_after_upent2(void)
{
    if (display_kind() == 0u) not_modelled(); /* a PRIO display does not sleep */
    set_A(negate(ONE));
    ad_at(ADDRESS_OF(COPINDEX));
    ts(ADDRESS_OF(FACEREG));
    xchsleep();
}

/* XCHSLEEP +5 -- LM p. 1363, CM p. 1474: "REPLACE SAME CADR BUT NEW JOB
 * AREA": this job sleeps under the CADR (JOBSLEEP). */
void xchsleep_after_xchtoend(void)
{
    ca(indexed(ADDRESS_OF(WAKECADR), sleeper()));
    continue_at(JOBSLEEP);
}

/* XCHSLEEP +4 -- LM p. 1363, CM p. 1474, where JOBWAKE returns: XCHTOEND
 * "CAUSES AWAKENED JOB TO GO TO ENDOFJOB", and XCHSLEEP +5. */
void xchsleep_after_jobwake(void)
{
    sleeper();
    tc_at((word)(XCHSLEEP + 4u));
    xchtoend();
    xchsleep_after_xchtoend();
}

/* JOBXCHS -- LM p. 1363, CM p. 1474: A "CONTROLS TYPE OF DISPLAY PUT TO
 * SLEEP" (0 MARK, 1 NORMAL), to FACEREG. WITCHONE finds who waits for the
 * DSKY, and JOBWAKE wakes it. */
void jobxchs(void)
{
    if (A > 1u) not_modelled();
    ts(ADDRESS_OF(FACEREG));
    tc_at((word)(JOBXCHS + 1u));
    witchone();
    tc_at((word)(JOBXCHS + 2u)); /* TC JOBWAKE */
    continue_at(JOBWAKE);
}

/* JOBXCHS +010 -- LM p. 1363, CM p. 1474, where XCHNYLOC returns: the kind's
 * idle and sleep bits (MARKOCT's word, masked by IDLESLEP) down in
 * FLAGWRD4. */
void jobxchs_after_xchnyloc(void)
{
    ca(indexed(ADDRESS_OF(MARKOCT), sleeper()));
    mask(IDLESLEP);
    tc_at((word)(JOBXCHS + 013u)); /* TC DOWNENT2; IDLEMASK OCT 74004 */
    continue_at(DOWNENT2);
}

/* JOBXCHS +3 -- LM p. 1363, CM p. 1474, where JOBWAKE returns: FACEREG to
 * the woken job's FACEREG (its register set at LOCCTR, -1 if none was
 * found), and XCHNYLOC with XCHSLEEP's address (XCHQADD): the woken job goes
 * on at XCHSLEEP. */
void jobxchs_after_jobwake(void)
{
    word job = fetch(ADDRESS_OF(LOCCTR));

    sleeper();
    if (job != 077776u && job > LAST_JOB) not_modelled();
    ca(ADDRESS_OF(FACEREG));
    ts(indexed(ADDRESS_OF(FACEREG), job));
    set_A(XCHQADD);
    tc_at((word)(JOBXCHS + 7u));
    xchnyloc();
    jobxchs_after_xchnyloc();
}

/* IDLEMASK +1 -- LM p. 1364, CM p. 1474, where DOWNENT2 returns: "BIT SHOWS
 * PRIO INTERRUPTED NORM OR MARK", "BIT5 FOR MARK, BIT4 FOR NORMAL" (the word
 * after BIT5), and bit 3 too, up in FLAGWRD4. "FLAG ROUTINE DOES RELINT". */
void idlemask_after_downent2(void)
{
    ca(indexed(ADDRESS_OF(BIT5), sleeper()));
    ad(FOUR);
    tc_at((word)(JOBXCHS + 020u)); /* TC UPENT2; XCHQADD GENADR XCHSLEEP */
    continue_at(UPENT2);
}

/* XCHQADD +1 -- LM p. 1364, CM p. 1475, where UPENT2 returns: a mark over a
 * NORMAL display (FLAGWRD4 bit 3) goes on by MARKPLAY, else OKTOCOPY. */
void xchqadd_after_upent2(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
#if LUMINARY099
    mask(MKOVBIT);
#else
    mask(BIT3);
#endif
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(GENMARK); /* "USED AS GENADR FOR JOBWAKE" */
        continue_at(MARKPLAY);
        return;
    }
    continue_at(OKTOCOPY);
}

/* WAKEPLAY -- LM p. 1364, CM p. 1475: A, the kind to wake (0 MARK, 1
 * NORMAL), to TEMPOR2; its sleep bits (BITS5+11, or the word after it,
 * BITS4+10) and bit 3 down in FLAGWRD4. An overflow in A skips the INDEX
 * (TS leaves +1 or -1). */
void wakeplay(void)
{
    if (A > 1u && overflow_of(A) == 0) not_modelled();
    if (ts(ADDRESS_OF(TEMPOR2)))
        ca(ADDRESS_OF(BITS5P11));
    else
        ca(indexed(ADDRESS_OF(BITS5P11), fetch(ADDRESS_OF(TEMPOR2))));
    ad(FOUR);
    tc_at((word)(WAKEPLAY + 4u)); /* TC DOWNENT2; MARKFMSK OCT 40010 */
    continue_at(DOWNENT2);
}

/* MARKWAKE -- LM p. 1364, CM p. 1475: WAKEPLAY for the MARK. */
void markwake(void)
{
    set_A(ZERO);
    wakeplay();
}

/* MARKFMSK +1 -- LM p. 1364, CM p. 1475, where DOWNENT2 returns: wake the
 * kind's job (WAKECADR). */
void markfmsk_after_downent2(void)
{
    word kind = fetch(ADDRESS_OF(TEMPOR2));

    if (kind > 1u && kind != MINUS_ZERO) not_modelled();
    ca(indexed(ADDRESS_OF(WAKECADR), kind));
    inhint();
    tc_at((word)(WAKEPLAY + 011u)); /* TC JOBWAKE */
    continue_at(JOBWAKE);
}

/* MARKFMSK +5 -- LM p. 1364, CM p. 1475, where JOBWAKE returns: TCF
 * ENDRET. */
void markfmsk_after_jobwake(void)
{
    continue_at(ENDRET);
}

/* INITDSP -- LM p. 1364, CM p. 1475. "ALL .1 RESTARTS BRANCH DIRECTLY TO
 * INITDSP." "RESTORE MOST RECENT NORMAL EBANK", and RESTREG, "SUPERBANK
 * AND JOB PRIORITY", to SUPERSW. */
void initdsp(void)
{
    ca(ADDRESS_OF(EBANKTEM));
    ts(EBANK_ADDRESS);
    ca(ADDRESS_OF(RESTREG));
    tc_at((word)(INITDSP + 3u)); /* TC SUPERSW */
    continue_at(SUPERSW);
}

/* INITDSP +4 -- LM p. 1364, CM p. 1475, where SUPERSW returns: the job's
 * priority (PRIOCHNG). */
void initdsp_after_supersw(void)
{
    mask(PRIO37);
    tc_at((word)(INITDSP + 5u)); /* TC PRIOCHNG */
    continue_at(PRIOCHNG);
}

/* INITDSP +6 -- LM p. 1364, CM p. 1475, where PRIOCHNG returns. "IF THE MOST
 * RECENT NORMAL DISPLAY REQUEST WAS NOT FINISHED, CONTROL IS SENT BACK TO
 * THE LAST NORMAL USER.": to TEMPFLSH (the NORMAL user's call CADR +1) less
 * 3, the CAF before the TC BANKCALL, which makes the request again. */
void initdsp_after_priochng(void)
{
    set_A(negate(THREE));
    ad_at(ADDRESS_OF(TEMPFLSH));
    continue_at(BANKJUMP);
}
