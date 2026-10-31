/* SPDX-License-Identifier: MIT */
/* SXTMARK.agc, Comanche055 only (listing pages 222-235): the sextant's
 * sighting marks. The faithful C is c64-port/src/cm_sxtmark.c.
 *
 * How a sighting goes. SXTMARK takes the marks wanted in A, a VAC area
 * for them (MARKOK, MKVACFND), and starts the job MKVB51, which flashes
 * V51 ("PLEASE MARK"). Each press of the MARK button raises MARKRUPT: it
 * keeps the five CDUs and the time at once, and MARKIT has WAITLIST run
 * MARKDIF after CDUCHKWD's delay. MARKDIF compares the CDUs then with the
 * ones at the mark (DIFCHK): more than 3 bits apart, the ship was turning
 * too fast, alarm 121, and the mark is lost. Otherwise MARKCONT keeps it:
 * in the VAC area (MARK2, VACSTOR), or for R21 in MRKBUF1. When the last
 * mark wanted is in, the job MKVB50 flashes V50N25 ("PLEASE PERFORM": R1
 * 16, terminate marking); MARK REJECT takes the last mark back (MKREJECT)
 * and V51 comes back. On proceed, ENTANSWR leaves the VAC area in MARKSTAT
 * and the count of marks in QPRET's word, and ENDMARKS wakes the caller.
 *
 * MARKSTAT keeps the state: the VAC area's address in bits 1-9, bit 10
 * set by a mark (a reject clears it: "DONT ACCEPT TWO REJECTS TOGETHER"),
 * and in bits 12-14 the marks still wanted. QPRET +n, where n is the VAC
 * area's address, is the next free slot of seven words in the area
 * (VAC +1 on): the time, then CDUY, CDUS (the shaft), CDUZ, CDUT (the
 * trunnion) and CDUX.
 *
 * The basic code runs in bank 07 (MARKDIF, MKACPT and DIFCHK in 10),
 * where the binding (src/binding/sxtmark_routines.c) runs it. Each call
 * out of the file (BANKCALL, CHECKMM, ALARM, NOVAC, WAITLIST, GENTRAN...)
 * ends a function with Q and Z set, and the word the call returns to is
 * an entry of its own. */
#include "sxtmark.h"
#include "../agc/names.h"

#if COMANCHE055

static void sxtmark_plus3(void);
static void mkabort(void);
static void keycall(void);
static void mkreject(void);
static void mrkrejct(void);
static void reject2(void);
static void reject3(void);
static void market(void);
static void market3(void);
static void hundred14_alm(void);
static void mark2(void);
static void vacstor(void);
static void vacstor_plus2(void);
static void markit1(void);
static void markit(void);
static void termsxt(void);
static void termp03(void);
static void jamit(void);
static void mkvb51_plus3(void);
static void markdif_plus7(void);
static void markdif_plus11(void);
static void mkacpt(void);
static void tc_q(void);

/* ---- Helpers ---- */

/* INDEX k before an instruction on an erasable word: the AGC adds C(k)
 * to the whole instruction word, and while the sum stays within the
 * instruction's 10-bit address it is the address plus the index, as
 * numbers. Not modelled, as in the faithful port: a sum outside erasable
 * memory (the AGC's would be another instruction). */
static unsigned index_into(unsigned address, word index)
{
    long at = (long)address + ((index & SIGN_BIT) ? -(long)negate(index) : (long)index);

    if (at < 0 || at >= 02000) not_modelled();
    return (unsigned)at;
}

/* INDEX k; TS n. Not modelled either, as in the faithful port: a TS into
 * FBANK, Z or BBANK. */
static bool ts_indexed(unsigned address, word index)
{
    unsigned at = index_into(address, index);

    if (at >= FBANK_ADDRESS && at <= BBANK_ADDRESS) not_modelled();
    return ts(at);
}

/* INDEX k; XCH or ADS n: not modelled on the central registers (0-7), as
 * in the faithful port. */
static unsigned indexed_word(unsigned address, word index)
{
    unsigned at = index_into(address, index);

    if (at <= ZERO_ADDRESS) not_modelled();
    return at;
}

/* INDEX k; DXCH n. DXCH's instruction holds n +1, the second word; not
 * modelled with that one among the central registers, or past erasable
 * memory. */
static void dxch_indexed(unsigned address, word index)
{
    unsigned second = indexed_word(address + 1u, index);

    dxch(second - 1u);
}

/* INDEX MARKSTAT; TS 0 -1: MARKSTAT is a VAC area's address, and the word
 * before it is the area's VACnUSE. Not modelled, as in the faithful port,
 * for a MARKSTAT that is not an area's address. */
static void ts_vacnuse(void)
{
    unsigned area;

    for (area = 1u; area <= VAC_AREAS; ++area) {
        if (MARKSTAT == VACNUSE_ADDRESS(area) + 1u) {
            ts(VACNUSE_ADDRESS(area));
            return;
        }
    }
    not_modelled();
}

/* ---- Starting the marks ---- */

/* SXTMARK -- CM p. 222.
 * "NUMBER OF MARKS WANTED" (A) into RUPTREG1. A TS that overflows skips
 * a word: here the CAF SIX, which leaves A +1 or -1 for the MASK. */
void sxtmark(void)
{
    inhint();
    if (ts(ADDRESS_OF(RUPTREG1)))
        sxtmark_plus3();
    else
        sxtmark_plus2();
}

/* SXTMARK +2, where P51-P53 enter (CADR SXTMARK +2). */
void sxtmark_plus2(void)
{
    set_A(SIX);
    sxtmark_plus3();
}

/* SXTMARK +3. EXTVBACT's "BIT2 = MARKING SYSTEM IN USE", "BIT3 = EXTENDED
 * VERB IN PROGRESS": "SET THEREFORE ABORT". "NOT SET": set bit 2 ("RESET
 * IN ENDMARK") and find a VAC area. CCS of the masked word is + or +0;
 * after the overflow, A was -1, and a negative word skips the CAF BIT2
 * (CCS leaves it less one, which the ADS adds), -0 the ADS too. */
static void sxtmark_plus3(void)
{
    mask_at(ADDRESS_OF(EXTVBACT));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        tc_at(SXTMARK + 5u); /* TC MKABORT */
        mkabort();
        return;
    case IS_PLUS_ZERO:
        set_A(BIT2);
        /* fall through */
    case NEGATIVE:
        ads(ADDRESS_OF(EXTVBACT));
        /* fall through */
    case IS_MINUS_ZERO:
        tc_at(SXTMARK + 010u); /* TC MARKOK */
        markok();
        return;
    }
}

/* MKABORT -- CM p. 223. TC BAILOUT; OCT 01211. */
static void mkabort(void)
{
    tc_at(MKABORT);
    continue_at(BAILOUT);
}

/* MARKOK -- CM p. 223. Also where BAILOUT returns.
 * "FIND VAC AREA": CCS VAC1USE, then VAC2USE ... VAC5USE, each followed
 * by TC MKVACFND. A free area's word holds its own address, so CCS
 * leaves the address less one in A for MKVACFND; a taken one's +0, and
 * the ladder goes on. Past the last: TC BAILOUT; OCT 01207.
 *
 * Never, as a VACnUSE is never negative: a negative word goes on to the
 * TC MKVACFND after the next CCS (VAC5USE's to the OCT 01207: not
 * modelled, as in the faithful port), and -0 skips the next area's CCS
 * (VAC5USE's lands on MKVACFND itself). */
void markok(void)
{
    unsigned area = 1u;

    while (area <= VAC_AREAS) {
        word at = MARKOK + 2u * (area - 1u); /* CCS VACnUSE */

        switch (ccs_at(VACNUSE_ADDRESS(area))) {
        case POSITIVE:
            tc_at(at + 1u);
            mkvacfnd();
            return;
        case IS_PLUS_ZERO: area += 1u; break;
        case NEGATIVE:
            if (area == VAC_AREAS) not_modelled();
            tc_at(at + 3u);
            mkvacfnd();
            return;
        case IS_MINUS_ZERO:
            if (area == VAC_AREAS) {
                mkvacfnd();
                return;
            }
            area += 2u;
            break;
        }
    }
    tc_at(MARKOK + 012u); /* TC BAILOUT; OCT 01207 */
    continue_at(BAILOUT);
}

/* MKVACFND -- CM p. 223.
 * "ADDRESS OF VAC AREA" (the word after its VACnUSE) into MARKSTAT;
 * QPRET +area, the "NEXT AVAILABLE MARK SLOT", gets it too, the first
 * slot; +0 in the area's VACnUSE: "STORE VAC AREA OCCUPIED". Then CHECKMM: "BACKUP MARK
 * ROUTINE USES SXTMARK" (P53, P54) goes straight back to the caller.
 * Never, as AD TWO on an address cannot overflow: an overflow would skip
 * the INDEX A, and the TS QPRET would keep A's +1 or -1. */
void mkvacfnd(void)
{
    ad(TWO);
    if (ts(ADDRESS_OF(MARKSTAT)))
        ts(ADDRESS_OF(QPRET));
    else
        ts_indexed(ADDRESS_OF(QPRET), a_word());
    set_A(ZERO);
    ts_vacnuse();
    tc_at(MKVACFND + 7u); /* TC CHECKMM; MM 53 */
    continue_at(CHECKMM);
}

/* MKVACFND +011, CHECKMM's return when not in P53: TCF +2; TC CHECKMM;
 * MM 54. */
void mkvacfnd_after_checkmm53_no(void)
{
    tc_at(MKVACFND + 013u);
    continue_at(CHECKMM);
}

/* MKVACFND +012, in P53: TCF SWRETURN. */
void mkvacfnd_after_checkmm53_yes(void)
{
    continue_at(SWRETURN);
}

/* MKVACFND +015, not in P54 either: "DESIRED NUMBER OF MARKS IN 12-14"
 * (MP BIT12 leaves RUPTREG1 times BIT12 in L) added to MARKSTAT, and
 * "ENTER MARK JOB", MKVB51. */
void mkvacfnd_after_checkmm54_no(void)
{
    set_A(BIT12);
    mp_at(ADDRESS_OF(RUPTREG1));
    xch(L_ADDRESS);
    ads(ADDRESS_OF(MARKSTAT));
    set_A(PRIO32);
    tc_at(MKVACFND + 025u); /* TC NOVAC; 2CADR MKVB51 */
    continue_at(NOVAC);
}

/* MKVACFND +016, in P54: TCF SWRETURN. */
void mkvacfnd_after_checkmm54_yes(void)
{
    continue_at(SWRETURN);
}

/* MKVACFND +030, MKVB51's job started: back to SXTMARK's caller,
 * "SAME AS MODEEXIT". */
void mkvacfnd_after_novac(void)
{
    relint();
    continue_at(SWRETURN);
}

/* ---- Releasing the mark system ---- */

/* MKRELEAS -- CM p. 224.
 * "SHOW MARK SYSTEM NOW AVAILABLE": MARKSTAT +0, and its VAC area's
 * VACnUSE its own address back (INDEX A; TS 0, A being the address less
 * one). With no area (+0) the TS 0 is TS A, which changes nothing. (CCS
 * of a masked word is never negative.) Then on to MKRLEES. */
void mkreleas(void)
{
    set_A(ZERO);
    xch(ADDRESS_OF(MARKSTAT));
    mask(LOW9);
    if (ccs_at(A_ADDRESS) == POSITIVE) ts_indexed(A_ADDRESS, a_word());
    mkrlees();
}

/* MKRLEES -- CM p. 224.
 * Clear OPTMODES bit 9, the "COARSE OPTICS RETURN FLAG"; OPTIND -1, "KILL
 * COARSE OPTICS"; and "DISABLE OPTICS ERROR COUNTER" (channel 12 bit 2).
 * Then back to the caller (TC SWRETURN). */
void mkrlees(void)
{
    inhint();
    set_A(negate(BIT9));
    mask_at(ADDRESS_OF(OPTMODES));
    ts(ADDRESS_OF(OPTMODES));
    set_A(NEGONE);
    ts(ADDRESS_OF(OPTIND));
    set_A(negate(BIT2));
    wand_channel(CHAN12);
    relint();
    tc_at(MKRLEES + 012u); /* TC SWRETURN */
    continue_at(SWRETURN);
}

/* ---- The MARK, MARK REJECT and navigation DSKY keys ---- */

/* MARKRUPT -- CM p. 225.
 * The interrupt of the navigation panel's keys. "STORE CDUS AND OPTICS
 * NOW" (MKCDUT ... MKCDUX) and the time (MKT2T1, and SAMPTIME, "RUPT TIME
 * FOR NOUN 65"), and Q in QRUPT. Then channel 16: bit 6, "ITS A MARK";
 * bit 7, "ITS A MARK REJECT"; else a key (KEYCALL).
 *
 * The TSes skip a word if A overflows: TS BANKRUPT the CA CDUT (MKCDUT
 * gets A's +1 or -1), TS QRUPT the CAF BIT6 (A's +1 or -1 tests channel
 * 16 instead). After a -1 the RAND leaves a word that may be negative,
 * and CCS goes on to the second RAND with it less one. */
void markrupt(void)
{
    if (!ts(ADDRESS_OF(BANKRUPT))) ca(ADDRESS_OF(CDUT));
    ts(ADDRESS_OF(MKCDUT));
    ca(ADDRESS_OF(CDUS));
    ts(ADDRESS_OF(MKCDUS));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(MKCDUY));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(MKCDUZ));
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(MKCDUX));
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(MKT2T1));
    dca(ADDRESS_OF(MKT2T1));
    dxch(ADDRESS_OF(SAMPTIME));
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(QRUPT))) set_A(BIT6);
    rand_channel(NAVKEYIN);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        tc_at(MARKRUPT + 027u); /* TC MARKIT */
        markit();
        return;
    case IS_PLUS_ZERO:
        set_A(BIT7);
        /* fall through */
    case NEGATIVE: break;
    case IS_MINUS_ZERO:
        /* Never: no word here has all bits set. The AGC would run the
         * second RAND NAVKEYIN without its EXTEND, as TC 16. */
        not_modelled();
        return;
    }
    rand_channel(NAVKEYIN);
    /* CCS A: + or +0 (a positive word's bits of channel 16). */
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc_at(MARKRUPT + 034u); /* TC MKREJECT */
        mkreject();
        return;
    }
    keycall();
}

/* KEYCALL -- CM p. 226.
 * "NOT MARK OR MKREJECT, SEE IF KEYCODE" (channel 16 bits 1-5): "IT,S A
 * KEY CODE", on to PINBALL's KEYCOM (POSTJUMP); "ALARM IF NO INBITS",
 * alarm 113. */
static void keycall(void)
{
    set_A(LOW5); /* OCT37 */
    rand_channel(NAVKEYIN);
    if (A_is_zero()) {
        tc_at(KEYCALL + 7u); /* +3 TC ALARM; OCT 113 */
        continue_at(ALARM);
        return;
    }
    tc_at(KEYCALL + 5u); /* TC POSTJUMP; CADR KEYCOM */
    continue_at(POSTJUMP);
}

/* KEYCALL +011, after the alarm: TC RESUME. */
void keycall_after_alarm(void)
{
    tc_at(KEYCALL + 011u);
    continue_at(RESUME);
}

/* MARKIT -- CM p. 233.
 * Check the mark's CDUs after "DELAY OF CDUCHKWD CS IF PNZ" (CCS leaves
 * it less one, AD ONE puts it back), "10 MS IF NO CHECK" (+0, -0 or
 * negative): WAITLIST runs MARKDIF then. */
static void markit(void)
{
    if (ccs_at(ADDRESS_OF(CDUCHKWD)) == NEGATIVE) set_A(ZERO);
    ad(ONE);
    tc_at(MARKIT + 5u); /* TC WAITLIST; 2CADR MARKDIF */
    continue_at(WAITLIST);
}

/* MARKIT +010, MARKDIF's task set: TCF RESUME. */
void markit_after_waitlist(void)
{
    continue_at(RESUME);
}

/* ---- Is the mark good? ---- */

/* MARKDIF -- CM p. 234. A task, from MARKIT.
 * "IF DELAY CHECK IS ZERO OR NEG, ACP MARK" (MKACPT). Otherwise "SEE IF
 * VEHICLE RATE TO MUCH AT MARK": DIFCHK each CDU in turn, X, Y and Z,
 * with MKNDX "SET INDEX -1". DIFCHK returns to the word after its TC (TC
 * Q), so each check goes on there: MARKDIF +7, +011 and MKACPT. */
void markdif(void)
{
    ca(ADDRESS_OF(CDUCHKWD));
    if (A_is_zero_or_minus()) {
        mkacpt();
        return;
    }
    set_A(negate(BIT1));
    ts(ADDRESS_OF(MKNDX));
    ca(ADDRESS_OF(MKCDUX));
    tc_at(MARKDIF + 6u); /* TC DIFCHK */
    difchk();
}

/* MARKDIF +7: the Y CDU. */
static void markdif_plus7(void)
{
    ca(ADDRESS_OF(MKCDUY));
    tc_at(MARKDIF + 010u); /* TC DIFCHK */
    difchk();
}

/* MARKDIF +011: the Z CDU. */
static void markdif_plus11(void)
{
    ca(ADDRESS_OF(MKCDUZ));
    tc_at(MARKDIF + 012u); /* TC DIFCHK */
    difchk();
}

/* MKACPT -- CM p. 234. "MARK DATA OK, WHAT DO WE DO WITH IT": MARKCONT,
 * in bank 07 (IBNKCALL). */
static void mkacpt(void)
{
    tc_at(MKACPT); /* TC IBNKCALL; CADR MARKCONT */
    continue_at(IBNKCALL);
}

/* DIFCHK -- CM p. 234. Also where MKACPT's IBNKCALL returns.
 * "INCREMENT INDEX" and "GET MARK(ICDU) - CURRENT(ICDU)" (A holds the
 * mark's CDU; MKNDX picks CDUX, CDUY or CDUZ). A difference of +-0 or
 * "NOT GREATER" than 3 bits returns (TC Q); more is the alarm 121,
 * "COUPLED WITH PROGRAM ALARM", and "DO NOT ACCEPT" (TCF TASKOVER).
 * CCS takes one off the difference's magnitude, AD NEG2 two more. Not
 * modelled, as in the faithful port: an index that takes the MSU below
 * the editing registers (CYR) or out of erasable memory. */
void difchk(void)
{
    unsigned cdu;

    incr(ADDRESS_OF(MKNDX));
    cdu = index_into(ADDRESS_OF(CDUX), MKNDX);
    if (cdu < CYR_ADDRESS) not_modelled();
    msu_at(cdu);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: break; /* TCF +4 */
    case IS_PLUS_ZERO: tc_q(); return;
    case NEGATIVE: break; /* TCF +2 */
    case IS_MINUS_ZERO: tc_q(); return;
    }
    ad(NEG2);
    if (A_is_zero_or_minus()) { /* BZMF -3: TC Q */
        tc_q();
        return;
    }
    tc_at(DIFCHK + 014u); /* TC ALARM; OCT 00121 */
    continue_at(ALARM);
}

/* DIFCHK +016, after the alarm: TCF TASKOVER. */
void difchk_after_alarm(void)
{
    continue_at(TASKOVER);
}

/* TC Q: DIFCHK's return. The AGC runs the word in Q as an instruction:
 * the return address left by the TC DIFCHK, TC <address>, so Q becomes 3
 * and the program goes on there (MARKDIF +7, +011, MKACPT). From DIFCHK's
 * own entry Q can hold anything, and the AGC runs it, as the faithful
 * port does: any address of erasable or fixed memory, to go on at; or 3,
 * TC 3, which is RELINT, after which the AGC runs the next word, EBANK's,
 * as TC <EBANK> (EBANK below 02000 would be another instruction: not
 * modelled, as in the faithful port, nor a word in Q with bits 13-15 or
 * one of the central registers' addresses). */
static void tc_q(void)
{
    word to = overflow_corrected(Q);

    if (to == EBANK_ADDRESS && EBANK >= 02000u) {
        relint();
        tc_at(EBANK_ADDRESS);
        continue_at(EBANK);
    } else {
        if (to <= ZERO_ADDRESS || to > 07777u) not_modelled();
        tc_at(Q_ADDRESS);
        continue_at(to);
    }
    switch (Z) { /* the words of bank 10 that go on here */
    case MARKDIF: markdif(); return;
    case MARKDIF + 7u: markdif_plus7(); return;
    case MARKDIF + 011u: markdif_plus11(); return;
    case MKACPT: mkacpt(); return;
    case DIFCHK: difchk(); return;
    case DIFCHK + 016u: difchk_after_alarm(); return;
    default: return; /* elsewhere: the program goes on at Z */
    }
}

/* ---- Keeping the mark ---- */

/* MARKCONT -- CM p. 228. MKACPT's, through IBNKCALL.
 * "R21 MARK (SPECIAL MARKING FOR R21)" (STATE +2 bit 14): MARKIT1.
 * "NOT SET THEREFORE REGULAR MARKING": MARKET. */
void markcont(void)
{
    set_A(BIT14);
    mask_at(ADDRESS_OF(STATE) + 2u);
    if (A_is_zero()) {
        market();
        return;
    }
    markit1();
}

/* MARKIT1 -- CM p. 228. "SPECIAL FOR R21": "TRANSFER MRKBUF1 TO MRKBUF2"
 * (GENTRAN, SIX: seven words), then "CURRENT MARK DATA TO MARKBUF1". */
static void markit1(void)
{
    set_A(SIX);
    tc_at(MARKIT1 + 1u); /* TC GENTRAN; ADRES MRKBUF1; ADRES MRKBUF2 */
    continue_at(GENTRAN);
}

/* MARKIT1 +4. */
void markit1_after_gentran_mrkbuf2(void)
{
    set_A(SIX);
    tc_at(MARKIT1 + 5u); /* TC GENTRAN; ADRES MKT2T1; ADRES MRKBUF1 */
    continue_at(GENTRAN);
}

/* MARKIT1 +010: TCF TASKOVER. */
void markit1_after_gentran_mrkbuf1(void)
{
    continue_at(TASKOVER);
}

/* MARKET -- CM p. 228.
 * "SEE IF MARKS CALLED FOR": "COLLECT MARKS" (MARK2) while MARKSTAT holds
 * a VAC area. Otherwise a mark is only wanted for V59's "SPECIAL DISPLAY
 * JOB" (MARKDISP, "CALIBRATION MARK (SET) FOR P23"): the marking system
 * in use (EXTVBACT bit 2) and V59FLAG (STATE +5 bit 12) set. Else alarm
 * 122, "MARKING NOT CALLED FOR".
 * Never, as MARKSTAT is never negative: a negative word skips the CAF
 * TWO (CCS leaves it less one), -0 the MASK too (A +0, to MARKET3). */
static void market(void)
{
    switch (ccs_at(ADDRESS_OF(MARKSTAT))) {
    case POSITIVE:
        tc_at(MARKET + 1u); /* TC MARK2 */
        mark2();
        return;
    case IS_PLUS_ZERO:
        set_A(TWO);
        /* fall through */
    case NEGATIVE:
        mask_at(ADDRESS_OF(EXTVBACT));
        /* fall through */
    case IS_MINUS_ZERO: break;
    }
    if (A_is_zero()) {
        market3();
        return;
    }
    set_A(BIT12);
    mask_at(ADDRESS_OF(STATE) + 5u);
    if (A_is_zero()) {
        market3();
        return;
    }
    set_A(PRIO5);
    tc_at(MARKET + 013u); /* TC NOVAC; 2CADR MARKDISP */
    continue_at(NOVAC);
}

/* MARKET +016, MARKDISP's job started: "TRANSFER MARK DATE TO MARKDOWN". */
void market_after_novac(void)
{
    set_A(SIX);
    tc_at(MARKET + 017u); /* TC GENTRAN; ADRES MKT2T1; ADRES MARKDOWN */
    continue_at(GENTRAN);
}

/* MARKET +022: and to MRKBUF1, "FOR SPECIAL DISPLAY OF SHAFT AND
 * TRUNNION IF V59 ACTING". */
void market_after_gentran_markdown(void)
{
    set_A(SIX);
    tc_at(MARKET + 023u); /* TC GENTRAN; ADRES MKT2T1; ADRES MRKBUF1 */
    continue_at(GENTRAN);
}

/* MARKET +026: TCF TASKOVER. */
void market_after_gentran_mrkbuf1(void)
{
    continue_at(TASKOVER);
}

/* MARKET3 -- CM p. 228. Alarm 122, "MARKING NOT CALLED FOR". */
static void market3(void)
{
    tc_at(MARKET3); /* TC ALARM; OCT 122 */
    continue_at(ALARM);
}

/* MARKET3 +2: TCF TASKOVER. */
void market3_after_alarm(void)
{
    continue_at(TASKOVER);
}

/* 114ALM -- CM p. 228. Alarm 114, "MARK NOT WANTED". */
static void hundred14_alm(void)
{
    tc_at(HUNDRED14_ALM); /* TC ALARM; OCT 114 */
    continue_at(ALARM);
}

/* 114ALM +2: TCF TASKOVER. */
void hundred14_alm_after_alarm(void)
{
    continue_at(TASKOVER);
}

/* MARK2 -- CM p. 229.
 * "SEE IF MARKS WANTED-REDUCE MARKS WANTED" (A is MARKSTAT less one, and
 * 74K takes the rest of a mark off bits 12-14): none wanted, alarm 114.
 * "SET BIT10 TO ENABLE REJECT" (COM; MASK BIT10: the bit if clear). Then
 * "PICK UP MARK SLOT-POINTER" (QPRET +area) into ITEMP2, "INCREMENT
 * POINTER" by seven, and on to VACSTOR.
 * Never, as these sums cannot overflow: an overflow at TS MARKSTAT would
 * skip the COM, and at the indexed TS QPRET VACSTOR's EXTEND, so its DCA
 * MKT2T1 would run as CA MKT2T1 +1. */
static void mark2(void)
{
    ad(SEVENTY_FOUR_K);
    if (A_is_zero_or_minus()) { /* BZMF 114ALM */
        hundred14_alm();
        return;
    }
    if (!ts(ADDRESS_OF(MARKSTAT))) com();
    mask(BIT10);
    ads(ADDRESS_OF(MARKSTAT));
    mask(LOW9);
    ts(ADDRESS_OF(ITEMP1));
    xch(indexed_word(ADDRESS_OF(QPRET), a_word()));
    ts(ADDRESS_OF(ITEMP2));
    ad(SEVEN);
    if (ts_indexed(ADDRESS_OF(QPRET), ITEMP1)) {
        ca(ADDRESS_OF(MKT2T1) + 1u);
        vacstor_plus2();
        return;
    }
    vacstor();
}

/* VACSTOR -- CM p. 229.
 * The mark into its slot, ITEMP2: the time (MKT2T1) in the first two
 * words, then CDUY, CDUS, CDUZ, CDUT and CDUX (INDEX ITEMP2; TS 2 to TS
 * 6). "IF ALL MARKS MADE FLASH VB50" (MARKSTAT bits 12-14 +0): the job
 * MKVB50. */
static void vacstor(void)
{
    dca(ADDRESS_OF(MKT2T1));
    vacstor_plus2();
}

/* VACSTOR +2: INDEX ITEMP2; DXCH 0 on. */
static void vacstor_plus2(void)
{
    dxch_indexed(0u, ITEMP2);
    ca(ADDRESS_OF(MKCDUY));
    ts_indexed(2u, ITEMP2);
    ca(ADDRESS_OF(MKCDUS));
    ts_indexed(3u, ITEMP2);
    ca(ADDRESS_OF(MKCDUZ));
    ts_indexed(4u, ITEMP2);
    ca(ADDRESS_OF(MKCDUT));
    ts_indexed(5u, ITEMP2);
    ca(ADDRESS_OF(MKCDUX));
    ts_indexed(6u, ITEMP2);
    set_A(PRIO34);
    mask_at(ADDRESS_OF(MARKSTAT));
    if (!A_is_zero()) {
        continue_at(TASKOVER);
        return;
    }
    set_A(PRIO32);
    tc_at(VACSTOR + 031u); /* TC NOVAC; 2CADR MKVB50 */
    continue_at(NOVAC);
}

/* VACSTOR +034, MKVB50's job started: TCF TASKOVER. */
void vacstor_after_novac(void)
{
    continue_at(TASKOVER);
}

/* ---- MARK REJECT ---- */

/* MKREJECT -- CM p. 230.
 * For R21 (STATE +2 bit 14): "-0 IN TIME IS FLAG TO R22 SIGNIFYING A
 * REJECTED MARK" (MRKBUF1 -1). Else "REGULAR REJECT" (MRKREJCT). */
static void mkreject(void)
{
    set_A(BIT14);
    mask_at(ADDRESS_OF(STATE) + 2u);
    if (A_is_zero()) {
        mrkrejct();
        return;
    }
    set_A(NEGONE);
    ts(ADDRESS_OF(MRKBUF1));
    tc_at(MKREJECT + 6u); /* TC RESUME */
    continue_at(RESUME);
}

/* MRKREJCT -- CM p. 230.
 * "SEE IF MARKS BEING ACCEPTED" (MARKSTAT's VAC area): REJECT2. If not,
 * alarm 112, "MARKS NOT BEING ACCEPTED". A negative MARKSTAT (never)
 * would run the alarm's OCT 112 as TC 112: not modelled, as in the
 * faithful port; -0 goes straight to the TC RESUME. */
static void mrkrejct(void)
{
    switch (ccs_at(ADDRESS_OF(MARKSTAT))) {
    case POSITIVE:
        tc_at(MRKREJCT + 1u); /* TC REJECT2 */
        reject2();
        return;
    case IS_PLUS_ZERO:
        tc_at(MRKREJCT + 2u); /* TC ALARM; OCT 112 */
        continue_at(ALARM);
        return;
    case NEGATIVE: not_modelled(); return;
    case IS_MINUS_ZERO: mrkrejct_after_alarm(); return;
    }
}

/* MRKREJCT +4, after the alarm: TC RESUME. */
void mrkrejct_after_alarm(void)
{
    tc_at(MRKREJCT + 4u);
    continue_at(RESUME);
}

/* REJECT2 -- CM p. 230.
 * "SEE IF MARK HAD BEEN MADE SINCE LAST REJECT, AND SET BIT10 TO ZERO TO
 * SHOW MARK REJECT": if so, REJECT3; if not, alarm 110, "DONT ACCEPT TWO
 * REJECTS TOGETHER". (CCS of a masked word is never negative.) */
static void reject2(void)
{
    set_A(negate(BIT10));
    mask_at(ADDRESS_OF(MARKSTAT));
    xch(ADDRESS_OF(MARKSTAT));
    mask(BIT10);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc_at(REJECT2 + 5u); /* TC REJECT3 */
        reject3();
        return;
    }
    tc_at(REJECT2 + 6u); /* TC ALARM; OCT 110 */
    continue_at(ALARM);
}

/* REJECT2 +010, after the alarm: TC RESUME. */
void reject2_after_alarm(void)
{
    tc_at(REJECT2 + 010u);
    continue_at(RESUME);
}

/* REJECT3 -- CM p. 231.
 * "DECREMENT POINTER TO REJECT MARK" (QPRET +area, by seven), and
 * "INCREMENT MARKS WANTED": if bits 12-14 were +0 (all marks were in), the
 * job MKVB51 asks for one more, "TO INDICATE MORE MARKS WANTED". (CCS of a
 * masked word is never negative.) */
static void reject3(void)
{
    set_A(LOW9);
    mask_at(ADDRESS_OF(MARKSTAT));
    ts(ADDRESS_OF(ITEMP1));
    set_A(negate(SEVEN));
    ads(indexed_word(ADDRESS_OF(QPRET), ITEMP1));
    set_A(BIT12);
    ad_at(ADDRESS_OF(MARKSTAT));
    xch(ADDRESS_OF(MARKSTAT));
    mask(PRIO34);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc_at(REJECT3 + 013u); /* TC RESUME */
        continue_at(RESUME);
        return;
    }
    set_A(PRIO32);
    tc_at(REJECT3 + 015u); /* TC NOVAC; 2CADR MKVB51 */
    continue_at(NOVAC);
}

/* REJECT3 +020, MKVB51's job started: TC RESUME. */
void reject3_after_novac(void)
{
    tc_at(REJECT3 + 020u);
    continue_at(RESUME);
}

/* ---- The mark verbs: V51 and V50N25 ---- */

/* MKVB51 -- CM p. 232. A job.
 * "CLEAR DISPLAY FOR MARK VERB" (KLEENEX), then "DISPLAY MARK VB51"
 * (GOMARK4), which returns three ways: V34, "TERMINATE"; V33, "PROCEED -
 * MARKING DONE"; ENTER, "RECYCLE TO INITIAL MARK DISPLAY". */
void mkvb51(void)
{
    tc_at(MKVB51); /* TC BANKCALL; CADR KLEENEX */
    continue_at(BANKCALL);
}

/* MKVB51 +2. */
void mkvb51_after_kleenex(void)
{
    set_A(VB51);
    mkvb51_plus3();
}

/* MKVB51 +3, with the verb and noun in A (MKVB50's way in too). */
static void mkvb51_plus3(void)
{
    tc_at(MKVB51 + 3u); /* TC BANKCALL; CADR GOMARK4 */
    continue_at(BANKCALL);
}

/* MKVB51 +5: TCF TERMSXT. */
void mkvb51_after_terminate(void)
{
    termsxt();
}

/* MKVB51 +6: TCF ENTANSWR. */
void mkvb51_after_proceed(void)
{
    entanswr();
}

/* MKVB51 +7: TCF MKVB5X. */
void mkvb51_after_enter(void)
{
    mkvb5x();
}

/* TERMSXT -- CM p. 232. "CLEAR MARK ACTIVITY" (CLEARMRK); then in P03,
 * TERMP03; else on to TERM52 (POSTJUMP). */
static void termsxt(void)
{
    tc_at(TERMSXT); /* TC CLEARMRK */
    continue_at(CLEARMRK);
}

/* TERMSXT +1. */
void termsxt_after_clearmrk(void)
{
    tc_at(TERMSXT + 1u); /* TC CHECKMM; MM 03 */
    continue_at(CHECKMM);
}

/* TERMSXT +3, not in P03: TCF +2; TC POSTJUMP; CADR TERM52. */
void termsxt_after_checkmm03_no(void)
{
    tc_at(TERMSXT + 5u);
    continue_at(POSTJUMP);
}

/* TERMSXT +4, in P03: TC TERMP03. */
void termsxt_after_checkmm03_yes(void)
{
    tc_at(TERMSXT + 4u);
    termp03();
}

/* TERMP03 -- CM p. 232. Set TRM03FLG (UPFLAG), which returns to
 * ENTANSWR. */
static void termp03(void)
{
    tc_at(TERMP03); /* TC UPFLAG; ADRES TRM03FLG */
    continue_at(UPFLAG);
}

/* ENTANSWR -- CM p. 232.
 * "PUT VAC ADR IN MARKSTAT AND NO. OF MARKS MADE IN QPRET BEFORE LEAVING
 * SXTMARK": QPRET +area, the next slot, less the area's address is seven
 * times the marks made. Times BIT12 (MP), "THIS PUTS NUMBER
 * MARKS-1 IN A", and AD ONE. "NO MARKS MADE": the difference (+0 or less)
 * as it is, which "R53 WILL PICK UP AND RECYCLE". */
void entanswr(void)
{
    set_A(LOW9);
    mask_at(ADDRESS_OF(MARKSTAT));
    ts(ADDRESS_OF(MARKSTAT));
    com();
    ad_at(index_into(ADDRESS_OF(QPRET), MARKSTAT));
    if (!A_is_zero_or_minus()) { /* BZMF JAMIT */
        mp(BIT12);
        ad(ONE);
    }
    jamit();
}

/* JAMIT -- CM p. 233.
 * "STORE NO OF MARKS MADE" in QPRET +area, and "SERVICE OPTSTALL
 * INTERFACE WITH" ENDMARKS, a task 50 ms on (WAITLIST). Then ENDMARK
 * "KNOCKS DOWN MARKING FLAG + DOES ENDOFJOB". Never in a working program:
 * a count that overflows the TS would skip the INHINT. */
static void jamit(void)
{
    if (!ts_indexed(ADDRESS_OF(QPRET), MARKSTAT)) inhint();
    set_A(FIVE);
    tc_at(JAMIT + 4u); /* TC WAITLIST; 2CADR ENDMARKS */
    continue_at(WAITLIST);
}

/* JAMIT +7, the task set: TC ENDMARK. */
void jamit_after_waitlist(void)
{
    tc_at(JAMIT + 7u);
    continue_at(ENDMARK);
}

/* ENDMARKS -- CM p. 233. The task: wake the caller (IBNKCALL GOODEND,
 * A = 1). */
void endmarks(void)
{
    set_A(ONE);
    tc_at(ENDMARKS + 1u); /* TC IBNKCALL; CADR GOODEND */
    continue_at(IBNKCALL);
}

/* MKVB5X -- CM p. 233. "RE-DISPLAY VB51 IF MORE MARKS WANTED AND VB50 IF
 * ALL IN" (MARKSTAT bits 12-14). (CCS of a masked word is never
 * negative.) */
void mkvb5x(void)
{
    set_A(PRIO34);
    mask_at(ADDRESS_OF(MARKSTAT));
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        mkvb51(); /* TCF MKVB51 */
        return;
    }
    mkvb50();
}

/* MKVB50 -- CM p. 233. A job too. V50N25 with R1 16 ("OCT 16",
 * DSPTEM1): "terminate marking". */
void mkvb50(void)
{
    set_A(R1D1);
    ts(ADDRESS_OF(DSPTEM1));
    set_A(V50N25);
    mkvb51_plus3(); /* TCF MKVB51 +3 */
}

#endif
