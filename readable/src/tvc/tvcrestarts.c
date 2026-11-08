/* SPDX-License-Identifier: MIT */
/* TVCRESTARTS.agc (Comanche055 pp. 956-960, bank 16): the "TVC RESTART
 * PACKAGE", REDOTVC, which a restart during an SPS burn calls in a T5
 * interrupt. It restarts TVCEXECUTIVE where its phase TVCEXPHS shows it
 * stopped (EXRSTRT), finishes a DAP copycycle cut short (FINCOPY), or
 * repeats the TVC initialization or the CSM/LM switch-over; then brings
 * up the TVC and optics outcounters again in T5 interrupts (ENABL1,
 * ENABL2, CMDSOUT), stops a stroke test, and goes on with the DAP at
 * TVCINIT4. The faithful C is c64-port/src/cm_tvc_restarts.c. */
#include "tvc.h"
#include "../agc/names.h"

#if COMANCHE055

static void enabl1(void);
static void fincopy(void);
static void trim_cmd(void);

/* TC (or TCR) to routine at fixed address at. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* The words in fixed memory the file reads under INDEX: TVCCADR, "TABLE
 * OF CADRS" (+0 to +3), and TVCEXADR's "TVCEXECUTIVE RESTART POINTS"
 * (+1 to +6). */
static bool a_constant(unsigned address)
{
    return (address >= ADDRESS_OF(TVCCADR) && address <= ADDRESS_OF(TVCCADR) + 3u) ||
           (address >= ADDRESS_OF(TVCEXADR) + 1u && address <= ADDRESS_OF(TVCEXADR) + 6u);
}

/* INDEX, then an order that reads the word at field: the field plus the
 * index, a 12-bit address. Not modelled (the faithful port refuses them
 * too): a sum outside 0-07777 or at Z, or a word of fixed memory outside
 * the tables. */
static unsigned read_field(unsigned field, unsigned index_address)
{
    int sum = (int)field + value_of(fetch(index_address));

    if (sum < 0 || sum > 07777 || sum == (int)Z_ADDRESS) not_modelled();
    if (sum >= 02000 && !a_constant((unsigned)sum)) not_modelled();
    return (unsigned)sum;
}

/* ---- REDOTVC -- p. 957 ---- */

/* REDOTVC, the T5 interrupt: BANKRUPT and QRUPT kept. EXECPHS: "CHECK
 * TVCEXECUTIVE PHASE". TVCEXPHS +0, "NO NEED TO RESTART TVCEXECUTIVE":
 * TVCDAPHS. Otherwise EXRSTRT by WAITLIST in nine centiseconds, the "9CS
 * DELAY TO FORCE EXRSTRT TO OCCUR" "BEFORE PITCHDAP, AFTER CMDSOUT"
 * (TVCEXPHS -0 skips the CAF NINE: the delay the CCS's +0). */
void redotvc(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    switch (ccs_at(ADDRESS_OF(TVCEXPHS))) {
    case IS_PLUS_ZERO: tvcdaphs(); return;
    case POSITIVE:
    case NEGATIVE: set_A(NINE); break;
    case IS_MINUS_ZERO: break;
    }
    tc_to(REDOTVC + 7u, WAITLIST); /* 2CADR EXRSTRT */
}

/* TVCDAPHS, after the WAITLIST: "CHECK BITS 15 AND 1 OF TVCPHASE TO SEE"
 * the "DAP RESTART LOCATION (-1,1,2,3)". Bit 1 alone (1 or 3), "FINISH
 * THE COPYCYCLE FIRST": FINCOPY. Neither (2, or +0), "JUST PREPARE THE
 * OUTCOUNTERS AND GO": ENABL1. Bit 15 (negative): "TEST FOR TVCPHASE =
 * -2" (bit 2 of its complement +0), a restart "DURING CSM/LM V46
 * SWITCH-OVER": ENABL1; -1, "RSTRT WAS IN TVCINIT": TRIM/CMD. (The MASK
 * leaves bits 15 and 1 at most, never -0.) */
void tvcdaphs(void)
{
    set_A(negate(OCT37776));
    mask_at(ADDRESS_OF(TVCPHASE));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: fincopy(); return;
    case IS_PLUS_ZERO: enabl1(); return;
    case NEGATIVE: cs(ADDRESS_OF(TVCPHASE)); break;
    case IS_MINUS_ZERO: break;
    }
    mask(BIT2);
    if (A_is_zero()) /* EXTEND; BZF TRIM/CMD */
        trim_cmd();
    else
        enabl1();
}

/* FINCOPY: "PICK UP THE APPROPRIATE COPYCYCLE" (TVCCADR + TVCPHASE: PCOPY
 * +1 or YCOPY +1) and "RE-ENTER THE COPYCYCLE, RETURN AT END" (TCR
 * ISWCALL). */
static void fincopy(void)
{
    ca(read_field(ADDRESS_OF(TVCCADR), ADDRESS_OF(TVCPHASE)));
    tc_to(FINCOPY + 2u, ISWCALL);
}

/* FINCOPY +3, after the copycycle: "NOW PREPARE THE OUTCOUNTERS". */
void fincopy_after_copycycle(void)
{
    enabl1();
}

/* TRIM/CMD: "TVCDAPON INITIALIZATION NOT COMPLETED," "EG.  P,YCMD MAY NOT
 * BE SET": PCMD and YCMD from PACTOFF and YACTOFF; then ENABL1. */
static void trim_cmd(void)
{
    dca(ADDRESS_OF(PACTOFF));
    dxch(ADDRESS_OF(PCMD));
    enabl1();
}

/* ---- The outcounters -- pp. 957-958 ---- */

/* ENABL1: "TVC ENABLE, FOLLOWED BY 40 MS (MIN) WAIT" (bit 8 of channel
 * 12), with the "BIT FOR OPTICS-DAC-ENABLE ALSO" (bit 11); then ENABL2 in
 * a T5 interrupt in 60 ms (T5LOC TVCADDR, the GENADR ENABL2; TIME5
 * TVCADDR +4, OCT 37772), and RESUME. */
static void enabl1(void)
{
    set_A(BIT8);
    ad(BIT11);
    wor_channel(CHAN12);
    set_A(fetch_fixed(ADDRESS_OF(TVCCADR)));
    ts(ADDRESS_OF(T5LOC));
    set_A(fetch_fixed(ADDRESS_OF(TVCEXADR)));
    ts(ADDRESS_OF(TIME5));
    continue_at(RESUME);
}

/* ENABL2, the T5 interrupt: BANKRUPT kept; the "OPTICS ERROR CNTR
 * ENABLE" (bit 2 of channel 12), "4MS MIN WAIT"; then CMDSOUT in a T5
 * interrupt in 20 ms (T5LOC TVCADDR +2, the GENADR CMDSOUT; TIME5
 * OCT37776), and NOQRSM. */
void enabl2(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    set_A(BIT2);
    wor_channel(CHAN12);
    set_A(fetch_fixed(ADDRESS_OF(TVCCADR) + 2u));
    ts(ADDRESS_OF(T5LOC));
    set_A(OCT37776);
    ts(ADDRESS_OF(TIME5));
    continue_at(NOQRSM);
}

/* CMDSOUT, the T5 interrupt: BANKRUPT and QRUPT kept; the "MOST RECENT
 * ACTUATOR COMMANDS", PCMD and YCMD less -0 ("AVOID +0"), to the
 * outcounters TVCPITCH and TVCYAW; "RELEASE THE COUNTERS (BITS 11,12)"
 * of channel 14 (PRIO6). (A sum with -0 never overflows, so neither TS
 * skips.) Then PHSCHK2: TVCPHASE positive or +0, CHKSTRK. Negative, "A
 * CONTAINS THE DIMINISHED ABSOLUTE OF" "TVCPHASE (-2 BECOMES +1, -1
 * BECOMES +0)": -1, PHSCHK2 +5; -2 or less, and -0, TC +3 to "REPEAT
 * CSM/LM V46 SWITCH-OVER" (IBNKCALL, CADR SWICHOVR +5, whose TC sets Q
 * again), "RETURN TO CHECK FOR STROKE TEST". */
void cmdsout(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    set_A(negate(ZERO));
    ad_at(ADDRESS_OF(PCMD));
    ts(ADDRESS_OF(TVCPITCH));
    set_A(negate(ZERO));
    ad_at(ADDRESS_OF(YCMD));
    ts(ADDRESS_OF(TVCYAW));
    set_A(PRIO6);
    wor_channel(CHAN14);
    switch (ccs_at(ADDRESS_OF(TVCPHASE))) { /* PHSCHK2 */
    case POSITIVE:
    case IS_PLUS_ZERO: chkstrk(); return;
    case NEGATIVE:
        if (ccs_register(A_ADDRESS) == IS_PLUS_ZERO) { /* (A +0 or positive) */
            phschk2_mrclean();
            return;
        }
        break;
    case IS_MINUS_ZERO: break;
    }
    tc_to(CMDSOUT + 023u, IBNKCALL);
}

/* PHSCHK2 +5: "REPEAT TVC INITIALIZATION" (POSTJUMP, CADR MRCLEAN; "DO
 * NOT RETURN"). */
void phschk2_mrclean(void)
{
    tc_to(CMDSOUT + 021u, POSTJUMP);
}

/* ---- The stroke test and EXRSTRT -- pp. 958-959 ---- */

/* CHKSTRK: "CHECK FOR STROKE TEST IN PROGRESS" (STROKER positive or
 * negative): "YES, KILL IT", TSTINITJ, STROKER +0 to "DISABLE STROKE
 * TEST" ("+0 MEANS NEW V68 REQUIRED FOR STARTUP"). Then (+0, or -0,
 * which "SHOWS PRIOR V68", as they are) "RESTART AT" "TVCINIT4 (ZEROS
 * TVCPHASE, AND" "CALLS TVC DAPS VIA DAPINIT)" (POSTJUMP). */
void chkstrk(void)
{
    enum sign stroker = ccs_at(ADDRESS_OF(STROKER));

    if (stroker == POSITIVE || stroker == NEGATIVE) { /* TSTINITJ */
        set_A(ZERO);
        ts(ADDRESS_OF(STROKER));
    }
    tc_to(CHKSTRK + 4u, POSTJUMP); /* CADR TVCINIT4 */
}

/* EXRSTRT, the WAITLIST task: "TVCEXECUTIVE RESTARTS....GO TO" the
 * "APPROPRIATE RESTART POINT", TVCEXADR + TVCEXPHS (INDEX A; TCF 0: on
 * at the word A names). Not modelled (the faithful port refuses them
 * too): a word outside 02000-07777, which the INDEX makes another order
 * than TCF. The binding goes on at a word of the file that is a key, as
 * the faithful port does. */
void exrstrt(void)
{
    int to;

    ca(read_field(ADDRESS_OF(TVCEXADR), ADDRESS_OF(TVCEXPHS)));
    to = value_of(overflow_corrected(A));
    if (to < 02000 || to > 07777) not_modelled();
    Z = (word)to;
}

#endif
