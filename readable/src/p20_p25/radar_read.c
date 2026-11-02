/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 7 (Luminary099 pp. 553-563 and 612-613): the radar
 * data read, from LRALT to RDBADEND, and the landing radar's lamps, from
 * R12LITES to VLIGHT, which the read routines call.
 *
 * A job reads a radar by a BANKCALL for one of the lead-ins (LRALT,
 * LRVELZ, LRVELY, LRVELX, RRRDOT, RRRANGE, or LRVEL for five velocity
 * samples): INITREAD sets the number of samples (NSAMP, SAMPLIM), the
 * nominal midpoint of the sampling (TIMEHOLD), the data good bits as they
 * stand (OLDATAGD), and the radar's select bits in channel 13, then goes
 * back through ROADBACK to wait in RADSTALL. Each reading ends in a
 * RADARUPT: RADAREAD copies the count (RNRAD) to the downlink, checks
 * which radar it was, and adds the sample to SAMPLSUM once the data good
 * bit was on before and after it (DGCHECK). After enough good samples it
 * ends at RGOODEND; after too many tries, at DATAFAIL and RDBADEND, with
 * alarm 521 when P20 wants it. A scale change of the rendezvous radar's
 * range or the landing radar's altitude ends the reading at BADRAD (and
 * sets RNGSCFLG); a high scale reading is left at the low scale's units.
 *
 * LRPOS2 drives the landing radar to position 2, and checks once a second
 * that it got there. RADLITES (p. 612) sets the landing radar's altitude
 * and velocity lamps (H and V) after a reading, and R12LITES, R12's, both:
 * on when the data failed (or, for H, the scale was bad), off when it is
 * good, unless the lamp is flashing; then the tracker lamp (RRTRKF).
 *
 * As in parts 1 to 6, each call out of the file leaves Z at the routine
 * and Q at the word after the TC, and a TC or a branch to a label the
 * faithful port keys a routine at stops there. INITREAD, DGCHECK,
 * RESAMPLE, DATAFAIL, SCALECHK, SCALADJ, R77CHECK, RADLITES and the lamps
 * have no keys: they run inside their callers' routines. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- The lead-ins and INITREAD ---- */

static void initread(void);

/* INITREAD -1: "ENTRY TO TAKE ONLY 1 SAMPLE" */
static void initread_one_sample(void)
{
    set_A(ONE);
    initread();
}

/* LRALT -- p. 553: "RADAR DATA READ BY A BANKCALL FOR THE APPROPRIATE
 * LEAD-IN BELOW." Each lead-in is a TC to INITREAD and the radar's select
 * bits for channel 13, which INITREAD finds through Q. LRALT, RRRDOT and
 * RRRANGE take "ONE SAMPLE PER READING"; LRVELZ, LRVELY and LRVELX the
 * number in A. */
void lralt(void)
{
    tc_at(LRALT); /* OCT 17 (ALLREAD) */
    initread_one_sample();
}

void lrvelz(void)
{
    tc_at(LRVELZ); /* OCT 16 */
    initread();
}

void lrvely(void)
{
    tc_at(LRVELY); /* OCT 15 */
    initread();
}

void lrvelx(void)
{
    tc_at(LRVELX); /* OCT 14 */
    initread();
}

void rrrdot(void)
{
    tc_at(RRRDOT); /* OCT 12 */
    initread_one_sample();
}

void rrrange(void)
{
    tc_at(RRRANGE); /* OCT 11 */
    initread_one_sample();
}

/* LRVEL: "THE ENTRY TO THE LR VELOCITY READ ROUTINE WHEN 5 SAMPLES ARE
 * WANTED.  ENTER WITH C(A)= 0,2,4 FOR LRVELZ,LRVELY,LRVELX RESP." The
 * index waits in TIMEHOLD ("STORE VBEAM INDEX HERE MOMENTARILY"; a TS that
 * overflows skips the CAF FIVE), and INDEX TIMEHOLD makes the TCF LRVELZ a
 * TCF to the beam's lead-in. Not modelled: an index to any other word. */
void lrvel(void)
{
    word to;

    if (!ts(ADDRESS_OF(TIMEHOLD))) set_A(FIVE); /* "SPECIFY FIVE SAMPLES" */
    to = indexed_instruction(read_word(LRVEL + 3u), fetch(ADDRESS_OF(TIMEHOLD)));
    if (OPERATION_CODE(to) != 1u ||
        (ADDRESS_FIELD(to) != LRVELZ && ADDRESS_FIELD(to) != LRVELY && ADDRESS_FIELD(to) != LRVELX))
        not_modelled();
    continue_at(ADDRESS_FIELD(to));
}

/* INITREAD -- p. 554, with interrupts held and A the number of samples:
 * "GET DT OF MIDPOINT OF NOMINAL SAMPLING INTERVAL (ASSUMES NO BAD SAMPLES
 * WILL BE ENCOUNTERED)" (MP BIT3: the number times 4, centiseconds, in
 * TIMEHOLD +1). NSAMP gets the samples still wanted after the first, and
 * SAMPLIM the tries (the number again: "INSERT FOLLOWING INSTRUCTION TO
 * GET 2N TRIES FOR N SAMPLES." is a DOUBLE left out). Then "READ CURRENT
 * VALUE OF DATA GOOD BITS." into OLDATAGD, "REMOVE ALL RADAR BITS" from
 * channel 13 and "SET NEW RADAR BITS", the word after the lead-in's TC;
 * TIMEHOLD becomes the "TIME OF NOMINAL MIDPOINT", SAMPLSUM +0. Not
 * modelled: a number of samples with overflow. */
static void initread(void)
{
    inhint();
    if (ts(ADDRESS_OF(TIMEHOLD))) not_modelled();
    mp(BIT3);
    dxch(ADDRESS_OF(TIMEHOLD));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: /* the number less one */
        ts(ADDRESS_OF(NSAMP));
        ad(ONE);
        ts(ADDRESS_OF(SAMPLIM));
        break;
    case IS_PLUS_ZERO:
        ad(ONE);
        ts(ADDRESS_OF(SAMPLIM));
        break;
    case NEGATIVE: ts(ADDRESS_OF(SAMPLIM)); break; /* its magnitude less one */
    case IS_MINUS_ZERO: break;
    }
    set_A(DGBITS);
    rand_channel(033);
    ts(ADDRESS_OF(OLDATAGD));
    set_A(negate(ALLREAD));
    wand_channel(013);
    set_A(read_word(Q)); /* INDEX Q; CAF 0 */
    wor_channel(013);
    dca(ADDRESS_OF(TIME2));
    das(ADDRESS_OF(TIMEHOLD));
    set_A(ZERO);
    ts(L_ADDRESS);
    dxch(ADDRESS_OF(SAMPLSUM));
    continue_at(ROADBACK);
}

/* ---- RADAREAD, the RADARUPT ---- */

/* RADAREAD -- p. 555: "THIS ROUTINE STARTS FROM A RADARUPT.  IT READS THE
 * DATA & LOTS MORE." A, with the superbank bits ("MUST SAVE SBANK BECAUSE
 * OF RUPT EXITS VIA TASKOVER (BADEND OR GOODEND)."), goes to BANKRUPT and
 * Q to QRUPT. The radar's select bits (channel 13 bits 1-3, 1 to 7) pick
 * the downlink word for the count RNRAD: "IF RADAR SELECT BITS ZERO, DO NOT
 * STORE DATA FOR DOWNLIST (ERASABLE PROBLEMS)". Then TRYCOUNT: tries left
 * (PLENTY), none (NOMORE), or a reading not asked for (SAMPLIM -1): alarm
 * 520. Not modelled: SAMPLIM -0. */
void radaread(void)
{
    ror_superbnk();
    ts(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    set_A(SEVEN);
    rand_channel(013);
    ts(ADDRESS_OF(DNINDEX));
    if (!A_is_zero()) {
        ca(ADDRESS_OF(RNRAD));
        ts(indexed(ADDRESS_OF(DNRRANGE) - 1u, fetch(ADDRESS_OF(DNINDEX))));
    }
    switch (ccs_at(ADDRESS_OF(SAMPLIM))) { /* TRYCOUNT */
    case POSITIVE: continue_at(PLENTY); break;
    case IS_PLUS_ZERO: continue_at(NOMORE); break;
    case NEGATIVE: tc_to(TRYCOUNT + 3u, ALARM); break; /* OCT 520 */
    case IS_MINUS_ZERO: not_modelled();
    }
}

void radaread_after_alarm(void)
{
    tc_to(TRYCOUNT + 5u, RESUME);
}

/* NOMORE: no tries left. "IS LRBYPASS SET?" "NO.  R12 IS ON -- BYPASS 521
 * ALARM." Nor while R04 runs ("IF 1, R04 IS RUNNING.  DO NOT ALARM"):
 * else "P20 WANTS THE ALARM." */
void nomore(void)
{
    ca(ADDRESS_OF(FLGWRD11));
    mask(LRBYBIT);
    if (A_is_zero()) {
        continue_at(BADRAD);
        return;
    }
    cs(ADDRESS_OF(FLAGWRD3)); /* "CHECK R04FLAG." */
    mask(R04FLBIT);
    if (A_is_zero()) {
        continue_at(BADRAD);
        return;
    }
    tc_to(NOMORE + 010u, ALARM); /* OCT 521 */
}

/* RDBADEND -2 and RGOODEND -2 -- p. 563: the ends "WHEN NOT UNDER
 * WAITLIST CONTROL" (RUPTAGN -0: the end runs from an interrupt, not a
 * task). */
static void rdbadend_from_interrupt(void)
{
    set_A(MINUS_ZERO); /* CS ZERO */
    ts(ADDRESS_OF(RUPTAGN));
    rdbadend();
}

static void rgoodend_from_interrupt(void)
{
    set_A(MINUS_ZERO); /* CS ZERO */
    ts(ADDRESS_OF(RUPTAGN));
    continue_at(RGOODEND);
}

/* BADRAD: SAMPLIM -1 (no more readings wanted), and the bad end. */
void badrad(void)
{
    set_A(negate(ONE));
    ts(ADDRESS_OF(SAMPLIM));
    tc_at(BADRAD + 2u);
    rdbadend_from_interrupt();
}

static void r77check(void);

/* PLENTY: SAMPLIM the tries left, less one (a TS that overflows skips the
 * CAF BIT3). Channel 13 bit 3 "TO FIND OUT WHICH RADAR": the rendezvous
 * radar's (RENDRAD), else the landing radar's, which "R77 QUITS HERE." */
void plenty(void)
{
    if (!ts(ADDRESS_OF(SAMPLIM))) set_A(BIT3);
    rand_channel(013);
    if (A_is_zero()) {
        continue_at(RENDRAD);
        return;
    }
    tc_at(PLENTY + 6u);
    r77check();
}

/* LRPOSCHK: "SEE IF LR IN DESIRED POSITION" (RADMODES bit 6, channel 33
 * bit 6), else alarm 522 and BADRAD. */
void lrposchk(void)
{
    ca(ADDRESS_OF(RADMODES));
    rxor_channel(033);
    mask(BIT6);
    if (A_is_zero()) {
        continue_at(VELCHK);
        return;
    }
    tc_to(LRPOSCHK + 6u, ALARM); /* OCT 522 */
}

void lrposchk_after_alarm(void)
{
    tc_to(LRPOSCHK + 010u, BADRAD);
}

static void dgcheck(void);

/* VELCHK: channel 13 bits 1-2 both set (LRALT's select bits): "TAKE A LR
 * RANGE READING" (LRHEIGHT). Else a velocity beam's: the count less its
 * bias, LVELBIAS ("LANDING RADAR BIAS FOR 153.6 KC."), to ITEMP3 +1, and
 * its bit 15 to ITEMP3; then DGCHECK for the velocity's data good bit
 * (channel 33 bit 8): "DATA GOOD ISN'T CHECKED UNTIL AFTER READ- ING DATA
 * SO SOME RADAR TESTS WILL WORK INDEPENDENT OF DATA GOOD." */
void velchk(void)
{
    set_A(THREE); /* BIN3 */
    rxor_channel(013);
    mask(THREE);
    if (A_is_zero()) {
        continue_at(LRHEIGHT);
        return;
    }
    set_A(POSMAX);
    mask_at(ADDRESS_OF(RNRAD));
    ad(LVELBIAS);
    ts(L_ADDRESS);
    ca(ADDRESS_OF(RNRAD));
    double_A();
    mask(BIT1);
    dxch(ADDRESS_OF(ITEMP3));
    set_A(BIT8);
    tc_at(VELCHK + 017u);
    dgcheck();
}

static void goodrad_tries_done(void);
static void goodrad_fail_bit_off(void);

/* After DGCHECK: samples still wanted (NOEND), or GOODRAD. (A negative
 * NSAMP, or -0, runs into GOODRAD past its CS ONE, or past its TS.) */
void velchk_after_dgcheck(void)
{
    switch (ccs_at(ADDRESS_OF(NSAMP))) {
    case POSITIVE: tc_to(VELCHK + 021u, NOEND); break;
    case IS_PLUS_ZERO: continue_at(GOODRAD); break;
    case NEGATIVE: goodrad_tries_done(); break;
    case IS_MINUS_ZERO: goodrad_fail_bit_off(); break;
    }
}

static void radlites(void);

/* GOODRAD: SAMPLIM -1, and "WHEN ENOUGH GOOD DATA HAS BEEN GATHERED, RESET
 * DATA FAIL FLAGS FOR SETTRKF." (ITEMP1's bit in RADMODES); "LAMPS MAY GO
 * OFF IF DATA JUST GOOD." Then the good end. */
void goodrad(void)
{
    set_A(negate(ONE));
    goodrad_tries_done();
}

static void goodrad_tries_done(void)
{
    ts(ADDRESS_OF(SAMPLIM));
    goodrad_fail_bit_off();
}

static void goodrad_fail_bit_off(void)
{
    cs(ADDRESS_OF(ITEMP1));
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    tc_at(GOODRAD + 5u);
    radlites();
}

void goodrad_after_radlites(void)
{
    tc_at(GOODRAD + 6u);
    rgoodend_from_interrupt();
}

static void datafail(void);

/* After RESAMPLE's CCS: the activity bit (channel 13 bit 4) back on
 * ("RESET ACTIVITY BIT.") for the next reading, and the interrupt's end. */
static void resample_resume(void)
{
    wor_channel(013);
    tc_to(RESAMPLE + 6u, RESUME);
}

/* RESAMPLE: "SEE IF ANY MORE TRIES SHOULD BE MADE." None left: DATAFAIL,
 * "N SAMPLES NOT AVAILABLE." (SAMPLIM -0 puts A +0 in the channel.) */
static void resample(void)
{
    switch (ccs_at(ADDRESS_OF(SAMPLIM))) {
    case POSITIVE:
    case NEGATIVE: set_A(BIT4); break;
    case IS_PLUS_ZERO: datafail(); return;
    case IS_MINUS_ZERO: break;
    }
    resample_resume();
}

/* NOEND: one sample fewer still wanted. (A TS that overflows skips
 * RESAMPLE's CCS, to its TCF +2.) */
void noend(void)
{
    if (ts(ADDRESS_OF(NSAMP))) {
        set_A(BIT4);
        resample_resume();
        return;
    }
    resample();
}

static void scalechk_with_bit(void);

/* LRHEIGHT -- p. 557: the altitude. Its data good bit is channel 33 bit 5
 * ("POSITION OF DATA GOOD BIT IN CHAN 33"), its scale bit bit 9. (The TC
 * leaves Q positive, which RADIN reads as a reading that is not the RR
 * RDOT.) */
void lrheight(void)
{
    set_A(BIT5);
    ts(ADDRESS_OF(ITEMP1));
    set_A(BIT9);
    tc_at(LRHEIGHT + 3u);
    scalechk_with_bit();
}

/* RENDRAD: the rendezvous radar's reading. "MAKE SURE ANTENNA HAS NOT GONE
 * OUT OF LIMITS." (a reposition, REPOSBIT) and "BE SURE RR CDU HASN'T
 * FAILED." (RCDUFBIT), else BADRAD. Its data good bit is channel 33 bit 4;
 * Q gets channel 13 bit 1 "FOR LATER TESTING.": set for the range, which
 * has a scale bit (channel 33 bit 3), clear for the range rate: "NO SCALE
 * CHECK FOR RR RDOT." */
void rendrad(void)
{
    set_A(REPOSBIT);
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) {
        continue_at(BADRAD);
        return;
    }
    cs(ADDRESS_OF(RADMODES));
    mask(RCDUFBIT);
    if (bit_set_in_A()) {
        continue_at(BADRAD);
        return;
    }
    set_A(BIT4); /* "SEE IF DATA HAS BEEN GOOD." */
    ts(ADDRESS_OF(ITEMP1));
    set_A(BIT1); /* "SEE IF RR RDOT." */
    rand_channel(013);
    ts(Q_ADDRESS);
    if (!bit_set_in_A()) {
        continue_at(RADIN);
        return;
    }
    set_A(BIT3);
    scalechk_with_bit();
}

/* SCALECHK -1: L gets the radar's scale bit (A); SCALECHK compares the
 * "SCALE STATUS NOW" (channel 33) with the "SCALE STATUS BEFORE"
 * (RADMODES): "SEE IF THEY DIFFER". "THEY DIFFER.": SCALCHNG. */
static void scalechk_with_bit(void)
{
    ts(L_ADDRESS);
    rand_channel(033);
    xch(L_ADDRESS);
    mask_at(ADDRESS_OF(RADMODES));
    rxor_lchan();
    if (bit_set_in_A()) {
        tc_to(SCALECHK + 7u, SCALCHNG);
        return;
    }
    continue_at(RADIN);
}

static void scaladj(void);
static void dgcheck_plus1(void);

/* DGCHECK2: "SEE THAT DATA HAS BEEN GOOD BEFORE AND AFTER TAKING SAMPLE." */
static void dgcheck2(void)
{
    ca(ADDRESS_OF(ITEMP1));
    tc_at(DGCHECK2 + 1u);
    dgcheck_plus1();
}

/* DASAMPL: a correction (A, L) added to the sample in ITEMP3. */
static void dasampl(void)
{
    das(ADDRESS_OF(ITEMP3));
    dgcheck2();
}

/* RADIN: the count, bits 1-14 to ITEMP4 and bit 15 to ITEMP3 (the sample,
 * double precision). Not the RR RDOT (Q positive): SCALADJ, "NO, BUT SCALE
 * CHANGING MAY BE NEEDED."; else "IF RR RANGE RATE, THROW OUT BIAS"
 * (RDOTBIAS, "BIAS COUNT FOR RR RANGE RATE."). (Q -0 adds +0; not
 * modelled: a negative Q.) */
void radin(void)
{
    set_A(POSMAX);
    mask_at(ADDRESS_OF(RNRAD));
    ts(ADDRESS_OF(ITEMP4));
    ca(ADDRESS_OF(RNRAD));
    double_A();
    mask(BIT1);
    ts(ADDRESS_OF(ITEMP3));
    switch (ccs_at(Q_ADDRESS)) { /* "SEE IF RR RDOT." */
    case POSITIVE: scaladj(); return;
    case IS_PLUS_ZERO: dcs(RDOTBIAS); break;
    case IS_MINUS_ZERO: break;
    case NEGATIVE: not_modelled();
    }
    dasampl();
}

void dgcheck2_after_dgcheck(void)
{
    tc_to(DGCHECK2 + 2u, GOODRAD);
}

/* SCALCHNG: RADMODES's scale bit (A, the bit that differs, less one)
 * changed to the new scale, the data good bits updated ("UPDATE LAST VALUE
 * OF DATA GOOD BITS."), and RNGSCFLG set "FOR LRS24.1"; this reading is
 * bad. (A TS that overflows skips the CAF DGBITS.) */
void scalchng(void)
{
    lxch(ADDRESS_OF(RADMODES));
    ad(BIT1);
    rxor_lchan();
    if (!ts(ADDRESS_OF(RADMODES))) set_A(DGBITS);
    rand_channel(033);
    ts(ADDRESS_OF(OLDATAGD));
    tc_to(SCALCHNG + 011u, UPFLAG); /* ADRES RNGSCFLG */
}

void scalchng_after_upflag(void)
{
    continue_at(BADRAD);
}

/* R77CHECK: "R77 MUST IGNORE DATA FAILS SO AS NOT TO DISTURB THE
 * ASTRONAUT." "NOT R77": back to LRPOSCHK. In R77, "UPDATE LR DATA GOOD
 * BITS IN RADMODES" (bits 5 and 8, from channel 33) and end the reading
 * at RGOODEND. */
static void r77check(void)
{
    cs(ADDRESS_OF(FLAGWRD5));
    mask(R77FLBIT);
    if (bit_set_in_A()) {
        tc_q();
        return;
    }
    set_A(negate(BITS5_8));
    mask_at(ADDRESS_OF(RADMODES));
    ts(L_ADDRESS);
    set_A(BITS5_8);
    rand_channel(033);
    ad_at(L_ADDRESS);
    ts(ADDRESS_OF(RADMODES));
    tc_at(R77CHECK + 014u);
    rgoodend_from_interrupt();
}

static void lrsck(void);

/* SCALADJ -- p. 559: "THE FOLLOWING ROUTINE INCORPORATES RR RANGE AND LR
 * ALT SCALE INFORMATION AND LEAVES DATA AT LO SCALE." "L HAS SCALE INBIT
 * FOR THIS RADAR.": +0 on the low scale (DGCHECK2), else "ON HIGH SCALE."
 * The rendezvous radar's range (DNINDEX bit 3 clear) times 8; the altitude
 * LRSCK's. (L -0 skips the CA DNINDEX.) */
static void scaladj(void)
{
    switch (ccs_at(L_ADDRESS)) {
    case POSITIVE:
    case NEGATIVE: ca(ADDRESS_OF(DNINDEX)); break;
    case IS_PLUS_ZERO: dgcheck2(); return;
    case IS_MINUS_ZERO: break;
    }
    mask(BIT3);
    if (bit_set_in_A()) {
        lrsck();
        return;
    }
    dxch(ADDRESS_OF(ITEMP3));
    ddoubl();
    ddoubl();
    ddoubl();
    dxch(ADDRESS_OF(ITEMP3));
    dgcheck2();
}

/* LRSCK: the altitude on the high scale. A count of 2^14 or more (ITEMP3
 * 1), or one over HISCALIM ("2481.7 FT"), clears the scale-bad flag
 * (FLGWRD11 bit 3, SCABBIT, set when the LR's low scale discrete is not
 * present when it should be), a lower one sets it. The sample times 5 (DASAMPL adds it
 * times 4). Not modelled: a negative ITEMP3, or -0 (RADIN sets 0 or 1). */
static void lrsck(void)
{
    bool scale_bad;

    switch (ccs_at(ADDRESS_OF(ITEMP3))) {
    case POSITIVE: scale_bad = false; break;
    case IS_PLUS_ZERO:
        cs(ADDRESS_OF(ITEMP4));
        ad(HISCALIM);
        scale_bad = !A_is_zero_or_minus(); /* EXTEND; BZMF +5 */
        break;
    default: not_modelled(); return;
    }
    if (scale_bad) {
        cs(ADDRESS_OF(FLGWRD11));
        mask(SCABBIT);
        ads(ADDRESS_OF(FLGWRD11));
    } else {
        set_A(negate(SCABBIT));
        mask_at(ADDRESS_OF(FLGWRD11));
        ts(ADDRESS_OF(FLGWRD11));
    }
    dca(ADDRESS_OF(ITEMP3));
    ddoubl();
    ddoubl();
    dasampl();
}

/* DGCHECK -- p. 560: "UPDATE DATA GOOD BIT IN OLDATAGD AND MAKE SURE IT
 * WAS ON BEFORE AND AFTER THE SAMPLE WAS TAKEN BEFORE RETURNING.  IF NOT,
 * GOES TO RESAMPLE TO TRY AGAIN.  IF MAX NUMBER OF TRIES HAS BEEN REACHED,
 * THE BIT CORRESPONDING TO THE DATA GOOD WHICH FAILED TO APPEAR IS IN
 * ITEMP1 AND CAN BE USED TO SET RADMODES WHICH VIA SETTRKF SETS THE
 * TRACKER FAIL LAMP." The bit (A) to ITEMP1; DGCHECK +1 takes it there.
 * The bit before (OLDATAGD) and now (channel 33, which reads 0 while the
 * data is good): "SHOULD BOTH BE ZERO." Then DGCHECK +014 adds the sample.
 * (A negative sum, from an ITEMP1 with bit 15, adds A and L instead.) */
static void dgcheck(void)
{
    ts(ADDRESS_OF(ITEMP1));
    dgcheck_plus1();
}

static void dgcheck_plus1(void)
{
    rand_channel(033);
    ts(L_ADDRESS);
    cs(ADDRESS_OF(ITEMP1));
    mask_at(ADDRESS_OF(OLDATAGD));
    ad_at(L_ADDRESS);
    xch(ADDRESS_OF(OLDATAGD));
    mask_at(ADDRESS_OF(ITEMP1));
    ad_at(L_ADDRESS);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        tc_at(DGCHECK + 013u);
        resample();
        break;
    case IS_PLUS_ZERO: continue_at(DGCHECK + 014u); break;
    case NEGATIVE: /* past DGCHECK +014's DXCH */
        das(ADDRESS_OF(SAMPLSUM));
        tc_q();
        break;
    case IS_MINUS_ZERO: tc_q(); break;
    }
}

/* "IF DATA GOOD BEFORE AND AFTER, ADD TO ACCUMULATION." */
void dgcheck_good(void)
{
    dxch(ADDRESS_OF(ITEMP3));
    das(ADDRESS_OF(SAMPLSUM));
    tc_q();
}

/* DATAFAIL: "IN THE ABOVE CASE, SET RADMODES BIT SHOWING SOME RADAR DATA
 * FAILED." "IF WE HAVE BEEN UNABLE TO GATHER N SAMPLES, USE LAST ONE
 * ONLY." The lamps, and NOMORE. */
static void datafail(void)
{
    cs(ADDRESS_OF(ITEMP1));
    mask_at(ADDRESS_OF(RADMODES));
    ad_at(ADDRESS_OF(ITEMP1));
    ts(ADDRESS_OF(RADMODES));
    dxch(ADDRESS_OF(ITEMP3));
    dxch(ADDRESS_OF(SAMPLSUM));
    tc_at(DATAFAIL + 6u);
    radlites();
}

void datafail_after_radlites(void)
{
    continue_at(NOMORE);
}

/* ---- LRPOS2 ---- */

/* LRPOS2 -- p. 561: "THIS ROUTINE CHANGES THE LR POSITION, AND CHECKS THAT
 * IT GOT THERE." "SHOW DESIRED LR POSITION IS 2" (RADMODES bit 6); "SEE
 * IF ALREADY THERE." (channel 33 bit 7 +0): RADNOOP. Else the "COMMAND TO
 * POSITION 2" (channel 12 bit 13), and the task LRPOSCAN in 6 seconds:
 * "START SCANNING FOR INBIT AFTER 7 SECS." */
void lrpos2(void)
{
    inhint();
    cs(ADDRESS_OF(RADMODES));
    mask(LRPOSBIT);
    ads(ADDRESS_OF(RADMODES));
    set_A(BIT7);
    rand_channel(033);
    if (A_is_zero()) {
        radnoop();
        return;
    }
    set_A(BIT13);
    wor_channel(012);
    set_A(SIX_SECS);
    tc_to(LRPOS2 + 015u, WAITLIST); /* 2CADR LRPOSCAN */
}

void lrpos2_after_waitlist(void)
{
    tc_to(LRPOS2 + 020u, ROADBACK);
}

void radnoop_after_waitlist(void)
{
    tc_to(RADNOOP + 4u, ROADBACK);
}

/* LRPOSNXT: "SCAN ONCE PER SECOND 15 TIMES MAX AFTER INITIAL DELAY OF 7
 * SECONDS." SAMPLIM counts the scans. Not modelled: an A with overflow. */
void lrposnxt(void)
{
    if (ts(ADDRESS_OF(SAMPLIM))) not_modelled();
    tc_to(LRPOSNXT + 1u, FIXDELAY); /* DEC 100 */
}

/* "SEE IF LR POS2 IS ON": "IF THERE, WAIT FINAL SECOND FOR BOUNCE."
 * (LASTLRDT). "SEE IF MAX TIME UP.": "IF TIME UP, DISABLE COMMAND AND
 * ALARM." (A negative SAMPLIM takes the WAND with A as CCS leaves it; not
 * modelled: -0.) */
void lrposnxt_after_fixdelay(void)
{
    set_A(BIT7);
    rand_channel(033);
    if (A_is_zero()) {
        continue_at(LASTLRDT);
        return;
    }
    switch (ccs_at(ADDRESS_OF(SAMPLIM))) {
    case POSITIVE: continue_at(LRPOSNXT); return;
    case IS_PLUS_ZERO: set_A(negate(BIT13)); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    wand_channel(012);
    rdbadend();
}

/* LASTLRDT -- p. 562: "WAIT TWO SECONDS AFTER RECEIPT OF INBIT TO WAIT FOR
 * ANTENNA BOUNCE TO DIE OUT."; then "REMOVE COMMAND" and the good end. */
void lastlrdt(void)
{
    set_A(TWO_SECS);
    tc_to(LASTLRDT + 1u, VARDELAY);
}

void lastlrdt_after_vardelay(void)
{
    set_A(negate(BIT13));
    wand_channel(012);
    continue_at(RGOODEND);
}

/* LRPOSCAN: the task. "SET UP FOR 15 SAMPLES." */
void lrposcan(void)
{
    set_A(FOURTEEN);
    continue_at(LRPOSNXT);
}

/* RGOODEND -- p. 563: the radar job's good end (POSTJUMP, CADR GOODEND). */
void rgoodend(void)
{
    set_A(TWO);
    tc_to(RGOODEND + 1u, POSTJUMP);
}

/* ---- The landing radar's lamps (pp. 612-613) ---- */

static void hlight(void);
static void vlight(void);

/* R12LITES -- p. 612: both lamps, H and V, then ISWRETRN; nothing during a
 * lamp test (IMODES33 bit 1). */
void r12lites(void)
{
    set_A(ONE);
    mask_at(ADDRESS_OF(IMODES33));
    if (bit_set_in_A()) {
        continue_at(ISWRETRN);
        return;
    }
    tc_at(R12LITES + 4u);
    hlight();
}

void r12lites_after_hlight(void)
{
    tc_at(R12LITES + 5u);
    set_A(negate(ONE)); /* HLIGHT -3: CS ONE */
    vlight();
}

void r12lites_after_vlight(void)
{
    continue_at(ISWRETRN);
}

/* RADLITES: after a reading, the lamp of the data good bit in ITEMP1
 * (nothing during a lamp test): channel 33 bit 8 the V lamp (VLIGHT, ITEMP5
 * -1), bit 5 (the -0 of ITEMP1 less BIT5) the H lamp (HLIGHT, ITEMP5 +0),
 * bit 4 the rendezvous radar's tracker lamp only (RRTRKF). */
static void radlites(void)
{
    set_A(BIT1);
    mask_at(ADDRESS_OF(IMODES33));
    if (bit_set_in_A()) {
        tc_q();
        return;
    }
    set_A(negate(BIT5));
    ad_at(ADDRESS_OF(ITEMP1));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        set_A(negate(ONE));
        vlight();
        break;
    case IS_PLUS_ZERO: vlight(); break;
    case NEGATIVE: rrtrkf(); break;
    case IS_MINUS_ZERO: hlight(); break;
    }
}

static void bothlits(void);
static void onlites(void);

/* HLIGHT: "ZERO ITEMP5 FOR H INDEX"; the H lamp (DSPTAB +11D bit 5, HLITE)
 * on when the scale is bad (SCABBIT) or the altitude failed (RADMODES bit
 * 5, LRALTBIT). */
static void hlight(void)
{
    ts(ADDRESS_OF(ITEMP5));
    set_A(BIT5); /* HLITE */
    ts(L_ADDRESS);
    ca(ADDRESS_OF(FLGWRD11));
    mask(SCABBIT);
    if (bit_set_in_A()) {
        onlites();
        return;
    }
    set_A(LRALTBIT);
    bothlits();
}

/* VLIGHT: ITEMP5 the V index (-1); the V lamp (DSPTAB +11D bit 3, VLITE)
 * on when the velocity failed (RADMODES bit 8, LRVELBIT). */
static void vlight(void)
{
    ts(ADDRESS_OF(ITEMP5));
    set_A(BIT3); /* VLITE */
    ts(L_ADDRESS);
    set_A(BIT8);
    bothlits();
}

static void litit(void);

/* BOTHLITS: A the lamp's fail bit in RADMODES. Failed: ONLITES. Else the
 * lamp's flash bit in FLGWRD11 (INDEX ITEMP5; MASK HFLSHBIT: bit 1 for H,
 * bit 2 for V): a flashing lamp is left as it is (RRTRKF); else LITIT puts
 * it off (A +0). */
static void bothlits(void)
{
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) {
        onlites();
        return;
    }
    ca(ADDRESS_OF(FLGWRD11));
    mask_at(indexed(ADDRESS_OF(HFLSHBIT), fetch(ADDRESS_OF(ITEMP5))));
    if (bit_set_in_A()) {
        rrtrkf();
        return;
    }
    litit();
}

/* LITIT -- p. 613: the lamp to the state in A (L, or +0) by TRKFLON +1,
 * with the caller's return kept in ITEMP6; then the tracker lamp. */
static void litit(void)
{
    qxch(ADDRESS_OF(ITEMP6));
    tc_at(LITIT + 2u);
    trkflon_plus1();
}

void litit_after_trkflon(void)
{
    qxch(ADDRESS_OF(ITEMP6));
    rrtrkf();
}

/* ONLITES: the lamp's flash bit off, and the lamp on (A, L). */
static void onlites(void)
{
    cs(indexed(ADDRESS_OF(HFLSHBIT), fetch(ADDRESS_OF(ITEMP5))));
    mask_at(ADDRESS_OF(FLGWRD11));
    ts(ADDRESS_OF(FLGWRD11));
    ca(L_ADDRESS);
    litit();
}

#endif
