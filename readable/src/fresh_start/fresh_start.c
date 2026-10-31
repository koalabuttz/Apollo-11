/* SPDX-License-Identifier: MIT */
/* Fresh start and restart: FRESH_START_AND_RESTART.agc of Luminary099 (LM,
 * listing pages 211-224) and Comanche055 (CM, pages 183-194), its part in
 * bank 05 and GOTOPOOH. The faithful C is c64-port/src/lm_fresh_start.c
 * and cm_fresh_start.c. (The rest of the file, V37 and P00, is in bank 04
 * in the LM and bank 10 in the CM.)
 *
 * The two missions share the plan and much of the code, but each sets its
 * own vehicle's words: the LM its descent engine, DAP and landing radar,
 * the CM its optics, VHF ranging and T5RUPT owner. Where they differ the
 * code says so in place.
 *
 * How the routines leave. Every routine ends by setting Z:
 *   - a TC to a routine elsewhere (MMDSPLAY, SWCALL, ALARM, WAITLIST,
 *     BANKCALL...): Q gets the return, Z the routine. Where that routine
 *     returns is a routine of this file too (goprog3_after_mmdsplay(),
 *     pinact()...);
 *   - TC POSTJUMP with a CADR after it: ENDRSTRT's DUMMYJOB +2 (the
 *     Executive's idle loop, "PICKS UP AT RELINT") and GOTOPOOH's GOP00FIX;
 *   - TC Q (RETURN): MR.KLEAN, P00KLEAN and V37KLEAN, which other banks
 *     call with BANKCALL.
 * Inside the file, a TC to a subroutine (STARTSUB, LIGHTSET, MR.KLEAN) is
 * a C call: the subroutine's TC Q sets Z to the word after the TC, and the
 * caller goes on from there. LIGHTSET does not always return: with the
 * MARK REJECT and ERROR RESET keys held it does a fresh start instead.
 *
 * A CCS continues at one of the four words after it, for a positive, +0,
 * negative or -0 value. Where the listing's word cannot be negative or -0
 * (a MASK result, a count), those cases are written out all the same, as
 * the AGC would run them, and marked "never". */
#include "fresh_start.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099
static void goprog3(void);
#endif

/* The words some TCs and CCS cases land on, by the listing's labels. */
#if LUMINARY099
#define TC_MR_KLEAN     (DOFSTRT1 + 041u) /* +4 TC MR.KLEAN */
#define WOR_CHAN12      (DOFSTRT1 + 040u) /* DOFSTRT1's EXTEND; WOR CHAN12 */
#define WOR_ENGINE_ON   (SETINFL + 026u)  /* EXTEND; WOR DSALMOUT (BIT13) */
#define TC_MMDSPLAY     (PCLOOP + 016u)
#define TC_SWCALL       (PINACT - 1u)
#define TC_STARTSUB_ELR (NONAVKEY + 1u) /* TC STARTSUB; TCF DOFSTART */
#define TC_STARTSUB_EM  (NONAVKEY + 3u) /* TC STARTSUB; TCF DOFSTRT1 */
#else
#define TC_MR_KLEAN     (DOFSTART + 037u)
#define WOR_CHAN12      (DOFSTART + 036u)
#define TC_VAC5STOR     (GOPROG + 5u)     /* TC BANKCALL; CADR VAC5STOR */
#define TC_STARTSUB_ER  (ELRSKIP - 1u)    /* TC STARTSUB after an E memory restore */
#define TC_SETCOARS     (NOCOARSE - 6u)   /* TC IBNKCALL; CADR SETCOARS */
#define TC_WAITLIST_CA  (NOCOARSE - 3u)   /* TC WAITLIST; 2CADR CA+ECE */
#define WOR_ENGINE_ON   (NOCOARSE + 012u) /* EXTEND; WOR DSALMOUT (BIT13) */
#define TC_WAITLIST_TVC (GOPROG3 - 3u)    /* TC WAITLIST; 2CADR TVCEXEC */
#define TC_MMDSPLAY     (PCLOOP + 016u)
#define TC_STOPRATE     (NXTRST - 4u)
#define TC_SWCALL       (PINACT - 1u)
#define TC_STARTSUB_ELR (NONAVKEY + 1u) /* TC STARTSUB; TCF DOFSTART */
#endif

/* INDEX K; DXCH 0000 runs the word 52001 + C(K): DXCH 0000 is the word
 * 52001, DXCH's code with the address of the pair's second word. */
#define DXCH_0000      052001u
#define DXCH_CODE_MASK 076000u
#define DXCH_CODE      052000u
/* INDEX A; CCS PHASE1: the word 10000 + PHASE1, + C(A). */
#define CCS_CODE_MASK 076000u
#define CCS_CODE      010000u
/* The registers and counters, below 061: ERASCHK never tests them, and
 * the phase table is above them. */
#define FIRST_ERASABLE 061u

#define MPAC5 (ADDRESS_OF(MPAC) + 5u)
#define MPAC6 (ADDRESS_OF(MPAC) + 6u)

static void skipsim(void);
static void dofstart(void);
#if LUMINARY099
static void dofstrt1(void);
static void dorstart(void);
static void setinfl(void);
static void goprog2a(void);
static void startsb1(void);
#else
static void buttons(void);
static void elrskip(void);
#endif
static void endrstrt(void);
static void pcloop(void);
static void nxtrst(void);
static void pactive_plus1(void);
static void pinact_end(enum sign mpac5);
static bool lightset(void);
static void startsub(void);
static void startsb2(void);

/* ---- Small steps the listing repeats ---- */

/* CAF k; TS x (or CA k): x gets the constant, which stays in A. */
static void store_constant(unsigned address, word k)
{
    set_A(k);
    ts(address);
}

/* A run of TS: each word gets A (which holds no overflow here, so no TS
 * skips). */
static void ts_each(const unsigned *addresses, unsigned count)
{
    unsigned i;
    for (i = 0; i < count; ++i) ts(addresses[i]);
}

/* CS k; MASK x; TS x: clear k's bits in x. */
static void clear_bits(unsigned address, word k)
{
    set_A(negate(k));
    mask_at(address);
    ts(address);
}

/* EXTEND; DCA of a two-word constant. */
static void dca_constant(const word *pair)
{
    set_L(pair[1]);
    set_A(pair[0]);
}

/* EXTEND; DCA NEG0; DXCH -PHASEn: end restart group n. DCA NEG0 reads
 * NEG0 and the word after it, ZERO, so -PHASEn gets -0 and PHASEn +0, the
 * pair of a group that is not running. */
static void end_group(unsigned minus_phase)
{
    set_L(ZERO);
    set_A(NEG0);
    dxch(minus_phase);
}

/* CS DSPTAB +11D; MASK BITS4&6; CCS A; TC +4; CA BITS4&6; EXTEND; WOR
 * CHAN12. With the GIMBAL LOCK and NO ATT lamps both lit (DSPTAB +11D bits
 * 6 and 4): "THE IMU WAS IN COARSE ALIGN IN GIMBAL LOCK, SO PUT IT BACK
 * INTO COARSE ALIGN." False if it left the routine (never: a MASK leaves
 * no -0, on which the AGC would run the WOR's word alone, as TC 5012). */
static bool back_into_coarse_align(void)
{
    cs(ADDRESS_OF(DSPTAB) + 11u);
    mask(BITS4_6);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: return true;        /* TC +4 (whose Q the TC MR.KLEAN there replaces) */
    case IS_PLUS_ZERO: set_A(BITS4_6); /* fall through */
    case NEGATIVE: wor_channel(CHAN12); return true;
    case IS_MINUS_ZERO: break;
    }
    tc_at(WOR_CHAN12);
    continue_at(05012u);
    return false;
}

/* INDEX SKEEP7; DXCH 0000 (with SKEEP5,6 in A,L): put back the two words
 * ERASCHK, the self-check of erasable memory, was testing when the restart
 * came. Not modelled: a sum that is no longer a DXCH, or a pair among the
 * registers and counters. */
static void restore_e_memory(void)
{
    word instruction = indexed_instruction(DXCH_0000, fetch(ADDRESS_OF(SKEEP7)));
    if ((instruction & DXCH_CODE_MASK) != DXCH_CODE || (instruction & 01777u) < FIRST_ERASABLE + 1u)
        not_modelled();
    dxch((instruction & 01777u) - 1u);
}

/* ---- The fresh start ---- */

/* SLAP1 -- LM p. 211, CM p. 183.
 * The fresh start: "COMES HERE FROM PINBALL" (V36). STARTSUB "DOES MOST OF
 * THE WORK"; then STARTSW, TCF SKIPSIM ("PATCH....TCF STARTSIM FOR
 * SIMULATION": STARTSIM runs only in a patched program). */
void slap1(void)
{
    inhint();
    tc_at(SLAP1 + 1u);
    startsub();
    skipsim();
}

/* SKIPSIM -- LM p. 211, CM p. 183.
 * "TURN OFF ALL DSPTAB +11D LAMPS EXCEPT THE GIMBAL LOCK & NO ATT" (bit 15
 * tells the display program the word has changed). The LM sets the
 * erasable dump (V74) to one pass. Then the alarm and restart counts and
 * the failure registers are zeroed, and DSRUPTSW set to -12000. */
static void skipsim(void)
{
    static const unsigned zeroed[] = {ADDRESS_OF(ERCOUNT), ADDRESS_OF(FAILREG),
                                      ADDRESS_OF(FAILREG) + 1u, ADDRESS_OF(FAILREG) + 2u,
                                      ADDRESS_OF(REDOCTR)};

    ca(ADDRESS_OF(DSPTAB) + 11u);
    mask(BITS4_6);
    ad(BIT15);
    ts(ADDRESS_OF(DSPTAB) + 11u);
#if LUMINARY099
    store_constant(ADDRESS_OF(DUMPCNT), BIT12); /* "DUMP FOR ONE PASS" */
#endif
    set_A(ZERO);
    ts_each(zeroed, COUNT(zeroed));
    store_constant(ADDRESS_OF(DSRUPTSW), negate(PRIO12_WORD));
    dofstart();
}

#if LUMINARY099

/* DOFSTART -- LM p. 211.
 * "INSURE ENGINE IS OFF" (channel 11 bit 14), and zero THRUST. */
static void dofstart(void)
{
    set_A(BIT14);
    write_channel(DSALMOUT);
    store_constant(ADDRESS_OF(THRUST), NEG0);
    dofstrt1();
}

/* DOFSTRT1 -- LM pp. 211-213.
 * The fresh start proper, without touching the engine: zero the DAP's and
 * displays' words and the output channels ("TURN OFF RCS JETS"), end every
 * restart group (MR.KLEAN), set MODREG to -0 (no program), and give the
 * IMU, the DAP and the flag words their starting values. SWINIT +n is
 * flag word n's, but for the flags a fresh start keeps: REFSMFLG, SURFFLAG,
 * CMOONFLG, LMOONFLG and APSFLAG. */
static void dofstrt1(void)
{
    static const unsigned zeroed[] = {
        ADDRESS_OF(ABDELV),   ADDRESS_OF(NVSAVE),   ADDRESS_OF(EBANKTEM), ADDRESS_OF(CH5MASK),
        ADDRESS_OF(CH6MASK),  ADDRESS_OF(PVALVEST), ADDRESS_OF(ERESTORE), ADDRESS_OF(SMODE),
        ADDRESS_OF(DNLSTCOD), ADDRESS_OF(AGSWORD),  ADDRESS_OF(UPSVFLAG)};

    store_constant(ADDRESS_OF(RCSFLAGS), FOUR); /* "INITIALIZE ATTITUDE ERROR DISPLAYS." */
    store_constant(ADDRESS_OF(RESTREG), PRIO30);
    set_A(ZERO);
    ts_each(zeroed, COUNT(zeroed)); /* DNLSTCOD +0: "SELECT P00 DOWNLIST" */
    write_channel(CHAN5);
    write_channel(CHAN6);
    write_channel(CHAN12);
    write_channel(CHAN13);
    write_channel(CHAN14);
    if (!back_into_coarse_align()) return;
    tc_at(TC_MR_KLEAN);
    mr_klean();

    store_constant(ADDRESS_OF(MODREG), NEG0);
    store_constant(ADDRESS_OF(IMODES30), IM30INIF);
    store_constant(ADDRESS_OF(DB), MAXDB); /* 5 degree attitude deadband */
    store_constant(ADDRESS_OF(RATEINDX), FOUR);
    store_constant(ADDRESS_OF(DAPBOOLS), BOOLSTRT);
    store_constant(EBANK_ADDRESS, EBANK6);
    store_constant(ADDRESS_OF(STIKSENS), STIKSTRT);
    store_constant(ADDRESS_OF(MINUS_RATEDB), RATESTRT);
    store_constant(ADDRESS_OF(HIASCENT), FULLAPS); /* until the pad load */
    store_constant(ADDRESS_OF(DKTRAP), OCT_77001); /* "DAP FILTER GAINS" */
    ts(ADDRESS_OF(LMTRAP));
    store_constant(ADDRESS_OF(DKKAOSN), DEC_60);
    ts(ADDRESS_OF(LMKAOSN));
    store_constant(ADDRESS_OF(LMOMEGAN), ZERO);
    store_constant(ADDRESS_OF(DKOMEGAN), TEN);
    store_constant(ADDRESS_OF(DKDB), BIT8); /* docked deadband 1.4 degrees */
    set_A(IM33INIT);
    ad(BIT6); /* "KEEP BOTH DAP AND ERROR-NEEDLES DISPLAY OFF" */
    ts(ADDRESS_OF(IMODES33));

    /* "INITIALIZE SWITCHES ONLY ON FRESH START." */
    dca_constant(&SWINIT[0]);
    dxch(ADDRESS_OF(STATE));
    store_constant(ADDRESS_OF(STATE) + 2u, SWINIT[2]);
    set_A(REFSMBIT);
    mask_at(ADDRESS_OF(STATE) + 3u);
    ad(SWINIT[3]);
    ts(ADDRESS_OF(STATE) + 3u);
    dca_constant(&SWINIT[4]);
    dxch(ADDRESS_OF(STATE) + 4u);
    dca_constant(&SWINIT[6]);
    dxch(ADDRESS_OF(STATE) + 6u);
    set_A(SURFFBIT);
    ad(CMOONBIT);
    ad(LMOONBIT);
    mask_at(ADDRESS_OF(STATE) + 8u);
    ad(SWINIT[8]);
    ts(ADDRESS_OF(STATE) + 8u);
    store_constant(ADDRESS_OF(STATE) + 9u, SWINIT[9]);
    set_A(APSFLBIT);
    mask_at(ADDRESS_OF(STATE) + 10u);
    ad(SWINIT[10]);
    ts(ADDRESS_OF(STATE) + 10u);
    store_constant(ADDRESS_OF(STATE) + 11u, SWINIT[11]);
    endrstrt();
}

#else

/* DOFSTART -- CM pp. 183-185.
 * The fresh start proper: zero the output channels ("TURN OFF RCS JETS",
 * channels 11-14) and the words the CM's programs start from, end every
 * restart group (MR.KLEAN), set MODREG to -0 (no program), kill the coarse
 * optics, let T5RUPT idle (T5IDLOC), and give the flag words their
 * starting values: SWINIT +n is flag word n's, but for the flags a fresh
 * start keeps (NODOP01, REFSMFLG, CMOONFLG, LMOONFLG and SURFFLAG). */
static void dofstart(void)
{
    static const unsigned zeroed[] = {ADDRESS_OF(ERESTORE), ADDRESS_OF(SMODE),
                                      ADDRESS_OF(UPSVFLAG)};
    static const unsigned zeroed_too[] = {
        ADDRESS_OF(WTOPTION), ADDRESS_OF(DNLSTCOD), ADDRESS_OF(NVSAVE), ADDRESS_OF(EBANKTEM),
        ADDRESS_OF(RATEINDX), ADDRESS_OF(TRKMKCNT), ADDRESS_OF(VHFCNT), ADDRESS_OF(EXTVBACT)};

    set_A(ZERO);
    ts_each(zeroed, COUNT(zeroed));
    write_channel(CHAN5);
    write_channel(CHAN6);
    write_channel(DSALMOUT);
    write_channel(CHAN12);
    write_channel(CHAN13);
    write_channel(CHAN14);
    ts_each(zeroed_too, COUNT(zeroed_too));
    if (!back_into_coarse_align()) return;
    tc_at(TC_MR_KLEAN);
    mr_klean();

    store_constant(ADDRESS_OF(MODREG), NEG0);
    store_constant(ADDRESS_OF(RESTREG), PRIO30);
    store_constant(ADDRESS_OF(IMODES30), IM30INIF);
    store_constant(ADDRESS_OF(OPTIND), NEGONE); /* "KILL COARSE OPTICS" */
    store_constant(ADDRESS_OF(OPTMODES), OPTINITF);
    store_constant(ADDRESS_OF(IMODES33), IM33INIT);
    dca_constant(T5IDLER[0]); /* "LET T5 IDLE." */
    dxch(ADDRESS_OF(T5LOC));

    store_constant(ADDRESS_OF(STATE), SWINIT[0]);
    ca(ADDRESS_OF(FLAGWRD1));
    mask(NOP01BIT);
    ad(SWINIT[1]);
    ts(ADDRESS_OF(FLAGWRD1));
    store_constant(ADDRESS_OF(STATE) + 2u, SWINIT[2]);
    ca(ADDRESS_OF(FLAGWRD3));
    mask(BIT13); /* "REFSMMAT FLAG" */
    ad(SWINIT[3]);
    ts(ADDRESS_OF(FLAGWRD3));
    dca_constant(&SWINIT[4]);
    dxch(ADDRESS_OF(STATE) + 4u);
    dca_constant(&SWINIT[6]);
    dxch(ADDRESS_OF(STATE) + 6u);
    ca(ADDRESS_OF(FLAGWRD8));
    mask(OCT6200);
    ad(SWINIT[8]);
    ts(ADDRESS_OF(FLAGWRD8));
    store_constant(ADDRESS_OF(STATE) + 9u, SWINIT[9]);
    dca_constant(&SWINIT[10]);
    dxch(ADDRESS_OF(STATE) + 10u);
    endrstrt();
}

#endif

/* ENDRSTRT -- LM p. 213, CM p. 185.
 * "NOW IN ANOTHER BANK": POSTJUMP to DUMMYJOB +2, the Executive's idle
 * loop, which "PICKS UP AT RELINT". */
static void endrstrt(void)
{
    tc_at(ENDRSTRT);
    continue_at(POSTJUMP);
}

/* MR.KLEAN -- LM pp. 213-214, CM p. 185.
 * End every restart group, with interrupts inhibited: MR.KLEAN all six,
 * P00KLEAN all but group 2, V37KLEAN groups 1, 3, 5 and 6 (CM: 3, 5 and
 * 6). Each returns (TC Q) to its caller. */
void mr_klean(void)
{
    inhint();
    end_group(ADDRESS_OF(MINUS_PHASE2));
    p00klean();
}

/* P00KLEAN -- LM p. 214, CM p. 185. */
void p00klean(void)
{
    end_group(ADDRESS_OF(MINUS_PHASE4));
#if COMANCHE055
    end_group(ADDRESS_OF(MINUS_PHASE1));
#endif
    v37klean();
}

/* V37KLEAN -- LM pp. 214-215, CM p. 185. */
void v37klean(void)
{
#if LUMINARY099
    end_group(ADDRESS_OF(MINUS_PHASE1));
#endif
    end_group(ADDRESS_OF(MINUS_PHASE3));
    end_group(ADDRESS_OF(MINUS_PHASE5));
    end_group(ADDRESS_OF(MINUS_PHASE6));
    tc_through(Q_ADDRESS);
}

/* ---- The hardware restart ---- */

/* GOPROG -- LM p. 215, CM p. 186.
 * The hardware restart. "ADVANCE RESTART COUNTER", and keep where the
 * restart came: the interrupted BBANK (with SUPERBNK's bits) and Q in
 * RSBBQ. The LM puts the ISS back into coarse align if the NO ATT lamp is
 * lit; the CM first stores its erasables for debugging (VAC5STOR). */
void goprog(void)
{
    incr(ADDRESS_OF(REDOCTR));
    lxch(Q_ADDRESS);
    ror_superbnk();
    dxch(ADDRESS_OF(RSBBQ));
#if LUMINARY099
    ca(ADDRESS_OF(DSPTAB) + 11u);
    mask(BIT4);
    if (!A_is_zero()) {      /* BZF +4 not taken */
        ad(BIT6);            /* "SET ERROR COUNTER ENABLE" */
        wor_channel(CHAN12); /* "ISS WAS IN COARSE ALIGN SO GO BACK TO" it */
    }
    tc_at(BUTTONS);
    if (!lightset()) return;

    /* BUTTONS +1: ERESTORE is +0, or the address ERASCHK was testing when
     * the restart came (SKEEP7): then put its two words back. Anything
     * else and "E MEMORY MIGHT BE BAD": a fresh start, engine untouched. */
    set_A(HI5);
    mask_at(ADDRESS_OF(ERESTORE));
    if (!A_is_zero()) {
        tc_at(TC_STARTSUB_EM);
        startsub();
        dofstrt1();
        return;
    }
    cs(ADDRESS_OF(ERESTORE));
    if (A_is_zero()) {
        dorstart();
        return;
    }
    ad_at(ADDRESS_OF(SKEEP7));
    if (!A_is_zero()) {
        tc_at(TC_STARTSUB_EM);
        startsub();
        dofstrt1();
        return;
    }
    ca(ADDRESS_OF(SKEEP4));
    ts(EBANK_ADDRESS); /* "EBANK OF E MEMORY THAT WAS UNDER TEST." */
    dca(ADDRESS_OF(SKEEP5));
    restore_e_memory();
    store_constant(ADDRESS_OF(ERESTORE), ZERO);
    dorstart();
#else
    tc_at(TC_VAC5STOR);
    continue_at(BANKCALL);
#endif
}

#if LUMINARY099

/* DORSTART -- LM p. 215.
 * "DO INITIALIZATION AFTER ERASE RESTORE." */
static void dorstart(void)
{
    tc_at(DORSTART);
    startsub();
    setinfl();
}

/* SETINFL -- LM pp. 215-216.
 * Clear the integration flag; leave the PROG ALARM, GIMBAL LOCK and NO ATT
 * lamps as they were ("ON HARDWARE RESTART"), the IMU failure inhibits
 * too, and reset all failure codes; send the downlist the AGS word asks
 * for; turn on the throttle counter, and the engine on or off as
 * FLAGWRD5's ENGONFLG says. */
static void setinfl(void)
{
    clear_bits(ADDRESS_OF(FLGWRD10), INTFLBIT);
    set_A(BITS_9_6_4);
    mask_at(ADDRESS_OF(DSPTAB) + 11u);
    ad(BIT15);
    xch(ADDRESS_OF(DSPTAB) + 11u);
    set_A(IFAILINH);
    mask_at(ADDRESS_OF(IMODES30));
    ad(IM30INIR);
    ts(ADDRESS_OF(IMODES30));
    ca(ADDRESS_OF(AGSWORD)); /* "BE SURE OF CORRECT DOWNLIST" */
    ts(ADDRESS_OF(DNLSTCOD));
    set_A(BIT4);
    wor_channel(CHAN14); /* "TURN ON THRUST DRIVE" */

    cs(ADDRESS_OF(FLAGWRD5));
    mask(ENGONBIT);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: /* TCF +5 */
        set_A(BIT14);
        wor_channel(DSALMOUT); /* "TURN ENGINE OFF" */
        break;
    case IS_PLUS_ZERO: set_A(BIT13);             /* fall through */
    case NEGATIVE: wor_channel(DSALMOUT); break; /* "TURN ENGINE ON" (never negative) */
    case IS_MINUS_ZERO:                          /* never: the WOR's word alone, TC 5011 */
        tc_at(WOR_ENGINE_ON);
        continue_at(05011u);
        return;
    }
    goprog3();
}

#else

/* GOPROG -- CM p. 186, after VAC5STOR.
 * Channel 33: "TEST OSC FAIL BIT TO SEE IF WE HAVE HAD A POWER TRANSIENT.
 * IF SO, ATTEMPT A RESTART. IF NOT, ... IF AGC WARNING ON (BIT = 0), DO A
 * FRESH START ON THE ASSUMPTION THAT WE ARE IN A RESTART LOOP." */
void goprog_after_vac5stor(void)
{
    set_A(BIT15);
    wand_channel(CHAN33);
    if (!A_is_zero()) {
        set_A(BIT14);
        rand_channel(CHAN33);
        if (A_is_zero()) {
            tc_at(TC_STARTSUB_ELR);
            startsub();
            dofstart();
            return;
        }
    }
    buttons();
}

/* BUTTONS -- CM pp. 186-187.
 * "MAKE FRESH START CHECKS BEFORE ERESTORE" (LIGHTSET). ERESTORE is +0, or
 * the address ERASCHK was testing when the restart came (SKEEP7): then put
 * its two words back. Anything else: "DOUBT E MEMORY AND DO A FRESH
 * START". */
static void buttons(void)
{
    tc_at(BUTTONS);
    if (!lightset()) return;
    set_A(HI5);
    mask_at(ADDRESS_OF(ERESTORE));
    if (A_is_zero()) {
        cs(ADDRESS_OF(ERESTORE));
        if (A_is_zero()) {
            elrskip();
            return;
        }
        ad_at(ADDRESS_OF(SKEEP7));
        if (A_is_zero()) {
            ca(ADDRESS_OF(SKEEP4));
            ts(EBANK_ADDRESS); /* "EBANK OF E MEMORY THAT WAS UNDER TEST." */
            dca(ADDRESS_OF(SKEEP5));
            restore_e_memory();
            store_constant(ADDRESS_OF(ERESTORE), ZERO);
            elrskip();
            return;
        }
    }
    tc_at(TC_STARTSUB_ELR);
    startsub();
    dofstart();
}

/* ELRSKIP -1 and ELRSKIP -- CM p. 187.
 * "DO INITIALIZATION AFTER ERASE RESTORE" (STARTSUB), then "RESTART
 * AUTOPILOTS": FLAGWRD6's bits 15 and 14 pick T5RUPT's owner from T5IDLER
 * (T5IDLOC, REDORCS, REDOTVC or REDOSAT). Clear the integration flag,
 * restart the optics, keep the PROG ALARM, GIMBAL LOCK and NO ATT lamps
 * ("ON HARDWARE RESTART"), and with NO ATT lit return the ISS to coarse
 * align (SETCOARS, then CA+ECE in 60 ms). */
static void elrskip(void)
{
    word owner;

    tc_at(TC_STARTSUB_ER);
    startsub();
    ca(ADDRESS_OF(FLAGWRD6));
    mp(BIT3);
    mask(SIX);
    owner = a_word(); /* 0, 2, 4 or 6: INDEX A; DCA T5IDLER */
    dca_constant(T5IDLER[owner / 2u]);
    dxch(ADDRESS_OF(T5LOC));
    clear_bits(ADDRESS_OF(RASFLAG), INTFLBIT);
    ca(ADDRESS_OF(OPTMODES));
    mask(OPTINITR);
    ad(BIT7);
    ts(ADDRESS_OF(OPTMODES));
    set_A(BIT6);
    mask_at(ADDRESS_OF(IMODES33));
    ad(IM33INIT);
    ts(ADDRESS_OF(IMODES33));
    set_A(BITS_9_6_4);
    mask_at(ADDRESS_OF(DSPTAB) + 11u);
    ad(BIT15);
    xch(ADDRESS_OF(DSPTAB) + 11u);
    mask(BIT4);
    if (A_is_zero()) { /* "IF NO ATT LAMP WAS ON, LEAVE ISS IN COURSE ALIGN" */
        nocoarse();
        return;
    }
    tc_at(TC_SETCOARS);
    continue_at(IBNKCALL);
}

/* ELRSKIP, after SETCOARS -- CM p. 187.
 * WAITLIST CA+ECE in 60 ms; WAITLIST returns to NOCOARSE. */
void elrskip_after_setcoars(void)
{
    set_A(SIX);
    tc_at(TC_WAITLIST_CA);
    continue_at(WAITLIST);
}

/* NOCOARSE -- CM pp. 187-188.
 * "LEAVE FAILURE INHIBITS INTACT ON HARDWARE RESTART. RESET ALL FAILURE
 * CODES." Turn the engine back on if FLAGWRD5's ENGONFLG says it was. */
void nocoarse(void)
{
    set_A(IFAILINH);
    mask_at(ADDRESS_OF(IMODES30));
    ad(IM30INIR);
    ts(ADDRESS_OF(IMODES30));
    cs(ADDRESS_OF(FLAGWRD5));
    mask(ENGONBIT);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: break;                        /* TCF GOPROG3 */
    case IS_PLUS_ZERO: set_A(BIT13);             /* fall through */
    case NEGATIVE: wor_channel(DSALMOUT); break; /* "TURN ENGINE ON" (never negative) */
    case IS_MINUS_ZERO:                          /* never: the WOR's word alone, TC 5011 */
        tc_at(WOR_ENGINE_ON);
        continue_at(05011u);
        return;
    }
    goprog3();
}

#endif

/* ---- The software restart ---- */

#if LUMINARY099

/* ENEMA -- LM p. 216.
 * The software restart: STARTSB1 (no STARTSUB: the downlist goes on), then
 * as GOPROG2 from GOPROG2A. */
void enema(void)
{
    inhint();
    tc_at(ENEMA + 1u);
    startsb1();
    goprog2a();
}

/* GOPROG2 -- LM p. 216.
 * The software restart from STARTSB2 on. */
void goprog2(void)
{
    tc_at(GOPROG2);
    startsb2();
    goprog2a();
}

/* GOPROG2A -- LM p. 216.
 * The fresh start checks (LIGHTSET), then "CLEAR BITS 7 AND 14" of
 * FLGWRD10 and check the phase table. */
static void goprog2a(void)
{
    tc_at(GOPROG2A);
    if (!lightset()) return;
    clear_bits(ADDRESS_OF(FLGWRD10), RSFLGBTS);
    goprog3();
}

#else

/* ENEMA (GOPROG2) -- CM p. 188.
 * The software restart: LIGHTSET ("EXIT TO DOFSTART IF ERROR RESET AND
 * MARK REJECT DEPRESSED SIMULTANEOUSLY"), STARTSB2, "RESET INTEGRATION
 * BITS", and if TVC is on, call again the TVCEXEC task that STARTSB2
 * killed (WAITLIST returns to GOPROG3). */
void enema(void)
{
    inhint();
    tc_at(ENEMA + 1u);
    if (!lightset()) return;
    tc_at(ENEMA + 2u);
    startsb2();
    clear_bits(ADDRESS_OF(RASFLAG), INTMASK);
    cs(ADDRESS_OF(FLAGWRD6));
    mask(OCT60000);
    if (A_is_zero_or_minus()) { /* "IS TVC ON ... NO" */
        goprog3();
        return;
    }
    set_A(HALF_SEC);
    tc_at(TC_WAITLIST_TVC);
    continue_at(WAITLIST);
}

#endif

/* ---- The restart groups ---- */

/* GOPROG3 -- LM p. 216, CM p. 188.
 * "VERIFY PHASE TABLE AGREEMENTS" (PCLOOP). */
#if LUMINARY099
static void goprog3(void)
#else
void goprog3(void)
#endif
{
    set_A(NUMGRPS);
    pcloop();
}

/* PCLOOP -- LM pp. 216-217, CM pp. 188-189.
 * Each restart group's pair, from group 6 down to group 1: "COMPLEMENT INTO
 * A, DIRECT INTO L", whose exclusive or "MUST BE -0 FOR AGREEMENT"; else
 * PTBAD. All agree: show the major mode (MMDSPLAY), with MPAC +6 +0. */
static void pcloop(void)
{
    do {
        ts(MPAC5);
        double_A();
        dca(indexed(ADDRESS_OF(MINUS_PHASE1), a_word())); /* EXTEND; INDEX A; DCA -PHASE1 */
        rxor_lchan();
        if (ccs_register(A_ADDRESS) != IS_MINUS_ZERO) {
            /* PTBAD -- LM p. 217, CM p. 189. "SET ALARM TO SHOW PHASE TABLE
             * FAILURE": alarm 1107. */
            tc_at(PTBAD);
            continue_at(ALARM);
            return;
        }
    } while (ccs_at(MPAC5) == POSITIVE); /* "PROCESS ALL RESTART GROUPS." */
    ts(MPAC6); /* "SET TO +0." */
    tc_at(TC_MMDSPLAY);
    continue_at(MMDSPLAY);
}

/* PTBAD, after ALARM -- LM p. 217, CM p. 189.
 * The phase table is bad: a fresh start (the LM's leaves the engine). */
void ptbad_after_alarm(void)
{
#if LUMINARY099
    dofstrt1();
#else
    dofstart();
#endif
}

/* PCLOOP, after MMDSPLAY -- LM p. 217, CM pp. 188-189.
 * "RELINT DONE IN MMDSPLAY". The LM clears DIDFLAG ("TO FORCE R10 TO
 * RE-INITIALIZE ITSELF"), RODFLAG (for P66) and P21FLAG ("SO THAT P21 WILL
 * COMPUTE NEW BASE STATE VECTORS"); with the RCS DAP running the CM stops
 * automatic maneuvers (STOPRATE, in EBANK 6). Then NXTRST: "SEE IF ANY
 * GROUPS RUNNING." */
void goprog3_after_mmdsplay(void)
{
    inhint();
#if LUMINARY099
    clear_bits(ADDRESS_OF(FLAGWRD1), DIDFLBIT);
    clear_bits(ADDRESS_OF(FLAGWRD1), RODFLBIT);
    clear_bits(ADDRESS_OF(FLAGWRD0), P21FLBIT);
#else
    ca(ADDRESS_OF(FLAGWRD6)); /* CAE */
    mask(OCT60000);
    if (!A_is_zero_or_minus()) { /* "IS RCS DAP RUNNING" */
        store_constant(EBANK_ADDRESS, EBANK6);
        tc_at(TC_STOPRATE);
        continue_at(STOPRATE);
        return;
    }
#endif
    set_A(NUMGRPS);
    nxtrst();
}

#if COMANCHE055
/* PCLOOP, after STOPRATE -- CM p. 189. */
void goprog3_after_stoprate(void)
{
    store_constant(EBANK_ADDRESS, EBANK3);
    set_A(NUMGRPS);
    nxtrst();
}
#endif

/* NXTRST -- LM p. 217, CM p. 189.
 * Each restart group from 6 down to 1: a group whose phase is not +0 is
 * running (PACTIVE). The first CCS tells; PINACT's goes on to the next. */
static void nxtrst(void)
{
    enum sign more;
    do {
        word instruction;
        ts(MPAC5);
        double_A();
        /* INDEX A; CCS PHASE1. Not modelled: MPAC +5 that is not a group
         * (only after a PINACT entry no program makes). */
        instruction = indexed_instruction(CCS_CODE | ADDRESS_OF(PHASE1), a_word());
        if ((instruction & CCS_CODE_MASK) != CCS_CODE || (instruction & 01777u) < FIRST_ERASABLE)
            not_modelled();
        switch (ccs_at(instruction & 01777u)) {
        case POSITIVE:                       /* TCF PACTIVE: "PNZ -- GROUP ACTIVE." */
        case NEGATIVE: ts(ADDRESS_OF(MPAC)); /* fall through */
        case IS_MINUS_ZERO: pactive_plus1(); return;
        case IS_PLUS_ZERO: break; /* TCF PINACT: "GROUP NOT RUNNING." */
        }
        more = ccs_at(MPAC5); /* PINACT */
    } while (more == POSITIVE);
    pinact_end(more);
}

/* PACTIVE -- LM p. 217, CM p. 189.
 * MPAC gets the phase ("ABS OF PHASE"), MPAC +6 counts the groups running
 * ("GROUP DEMANDS PRESENT"), and RESTARTS restarts the group through
 * SWCALL: "MUST RETURN TO SWRETURN", that is to PINACT. */
static void pactive_plus1(void)
{
    incr(ADDRESS_OF(MPAC));
    incr(MPAC6);
    set_A(RACTCADR);
    tc_at(TC_SWCALL);
    continue_at(SWCALL);
}

/* PINACT -- LM p. 217, CM p. 189.
 * After a group: the next one (NXTRST), or the end. */
void pinact(void)
{
    enum sign more = ccs_at(MPAC5); /* "PROCESS ALL RESTART GROUPS." */
    if (more == POSITIVE)
        nxtrst();
    else
        pinact_end(more);
}

/* PINACT +2 on: all groups seen. With a group running ("PHASE ACTIVE"),
 * or a major mode, back to the Executive (ENDRSTRT); with neither ("IS
 * MODE -0", no program and nothing to restart) go to P00 (GOTOPOOH).
 * mpac5 is how the CCS of MPAC +5 came out: +0, or (never, as MPAC +5
 * counts down) a negative or -0, which land one and two words on. */
static void pinact_end(enum sign mpac5)
{
    enum sign groups = POSITIVE;

    switch (mpac5) {
    case IS_PLUS_ZERO: groups = ccs_at(MPAC6); break;
    case NEGATIVE: groups = POSITIVE; break;          /* on TCF ENDRSTRT */
    case IS_MINUS_ZERO: groups = IS_PLUS_ZERO; break; /* on CAF BIT15 */
    case POSITIVE: break;
    }
    switch (groups) {
    case POSITIVE: endrstrt(); return;
    case IS_PLUS_ZERO: set_A(BIT15);            /* fall through */
    case NEGATIVE: mask_at(ADDRESS_OF(MODREG)); /* fall through */
    case IS_MINUS_ZERO: break;
    }
    if (A_is_zero())
        gotopooh();
    else
        endrstrt();
}

/* ---- The fresh start checks ---- */

/* LIGHTSET -- LM p. 218, CM pp. 189-190.
 * The astronaut can ask for a fresh start at a restart: MARK REJECT
 * (navigation DSKY, channel 16) held with ERROR LIGHT RESET (the CM
 * takes it from either DSKY). Then a fresh start (STARTSUB, DOFSTART), and
 * false; else NONAVKEY returns (TC Q), true. */
static bool lightset(void)
{
#if LUMINARY099
    set_A(BIT5);
    rand_channel(NAVKEYIN);
    if (!A_is_zero()) {
        read_channel(MNKEYIN);
        ad(MINUS_ELR);
        if (A_is_zero()) {
            tc_at(TC_STARTSUB_ELR);
            startsub();
            dofstart();
            return false;
        }
    }
#else
    set_A(BIT7);
    rand_channel(NAVKEYIN);
    if (!A_is_zero()) {
        set_A(OCT37);
        rand_channel(NAVKEYIN); /* "NAV DSKY KEYCODES,MARK,MARK REJECT" */
        ad(MINUS_ELR);
        if (!A_is_zero()) {
            read_channel(MNKEYIN); /* "MAIN DSKY KEYCODES" */
            ad(MINUS_ELR);
        }
        if (A_is_zero()) {
            tc_at(TC_STARTSUB_ELR);
            startsub();
            dofstart();
            return false;
        }
    }
#endif
    tc_through(Q_ADDRESS); /* NONAVKEY */
    return true;
}

/* ---- STARTSUB ---- */

/* STARTSUB -- LM p. 219, CM p. 190.
 * The work fresh start and restart share. "SET POINTER SO NEXT 20MS
 * DOWNRUPT WILL CAUSE THE CURRENT DOWNLIST TO BE INTERRUPTED AND START
 * SENDING FROM THE BEGINNING OF THE CURRENT DOWNLIST." The LM resets the
 * rendezvous radar's modes (RADMODES), keeping channel 33's bit 6. */
static void startsub(void)
{
    store_constant(ADDRESS_OF(DNTMGOTO), LDNPHAS1);
#if LUMINARY099
    set_A(BIT6);
    rand_channel(CHAN33);
    ad(RMODINIT);
    ts(ADDRESS_OF(RADMODES));
    startsb1();
#else
    store_constant(ADDRESS_OF(TIME3), POSMAX); /* "37777 TO TIME3." */
    ad(MINUS2);
    ts(ADDRESS_OF(TIME4));
    ad(NEGONE);
    ts(ADDRESS_OF(TIME5));
    startsb2();
#endif
}

#if LUMINARY099
/* STARTSB1 -- LM p. 219.
 * The clocks' interrupts as far off as can be (TIME3 37777, TIME4 37775,
 * TIME5 37774), TIME6 disabled ("JUST IN CASE A T6 RUPT IS ALREADY IN THE
 * PRIORITY CHAIN, ENSURE THAT ITS INPUTS WILL RENDER IT INEFFECTUAL"),
 * "CAUSE DAPIDLER TO CALL 1/ACCS", and "SET T5RUPT FOR DAPIDLER PROGRAM." */
static void startsb1(void)
{
    store_constant(ADDRESS_OF(TIME3), POSMAX);
    ad(MINUS2);
    ts(ADDRESS_OF(TIME4));
    ad(NEGONE);
    ts(ADDRESS_OF(TIME5));
    store_constant(EBANK_ADDRESS, EBANK6);
    clear_bits(ADDRESS_OF(RCSFLAGS), BIT13);
    store_constant(ADDRESS_OF(T6NEXT), POSMAX);
    wand_channel(CHAN13);
    store_constant(ADDRESS_OF(NXT6ADR), ZERO);
    ts(ADDRESS_OF(NEXTP));
    clear_bits(ADDRESS_OF(DAPBOOLS), ACCSOKAY);
    dca_constant(IDLEADR);
    dxch(ADDRESS_OF(T5ADR));
    startsb2();
}
#endif

/* STARTSB2 -- LM pp. 219-221, CM pp. 190-191.
 * What every restart clears:
 *   - the output channels' bits, but for those "DURING SOFTWARE RESTART"
 *     must not change (the LM's engine and ISS warnings, IMU and RR
 *     flags; the CM's lamps the restart leaves);
 *   - the LM's R29 and R25 and R77 flags, the CM's SKIPVHF flag set;
 *   - "INITIALIZE WAITLIST DELTA-TS": every task slot empty (NEG1/2, and
 *     the 2CADR of ENDTASK, SVCT3's);
 *   - "MAKE ALL EXECUTIVE REGISTER SETS AVAILABLE": every core set's
 *     PRIORITY -0, NEWJOB +0 ("SHOWS NO ACTIVE JOBS"), every VAC area free;
 *   - the DSKY: its registers blank (DSPOFF), the display and monitor
 *     words zeroed ("KILL MONITOR", "KILL INTERFACE DISPLAYS"). */
static void startsb2(void)
{
#if LUMINARY099
    static const unsigned display_words[] = {
        ADDRESS_OF(DELAYLOC), ADDRESS_OF(DELAYLOC) + 1u, ADDRESS_OF(DELAYLOC) + 2u,
        ADDRESS_OF(R1SAVE),   ADDRESS_OF(INLINK),        ADDRESS_OF(DSPCNT),
        ADDRESS_OF(CADRSTOR), ADDRESS_OF(REQRET),        ADDRESS_OF(CLPASS),
        ADDRESS_OF(DSPLOCK),  ADDRESS_OF(MONSAVE),       ADDRESS_OF(MONSAVE1),
        ADDRESS_OF(VERBREG),  ADDRESS_OF(NOUNREG),       ADDRESS_OF(DSPLIST),
        ADDRESS_OF(MARKSTAT), ADDRESS_OF(EXTVBACT),      ADDRESS_OF(IMUCADR),
        ADDRESS_OF(OPTCADR),  ADDRESS_OF(RADCADR),       ADDRESS_OF(ATTCADR),
        ADDRESS_OF(LGYRO),    ADDRESS_OF(FLAGWRD4)};
#else
    static const unsigned display_words[] = {
        ADDRESS_OF(DELAYLOC),      ADDRESS_OF(DELAYLOC) + 1u, ADDRESS_OF(DELAYLOC) + 2u,
        ADDRESS_OF(DELAYLOC) + 3u, ADDRESS_OF(R1SAVE),        ADDRESS_OF(INLINK),
        ADDRESS_OF(DSPCNT),        ADDRESS_OF(CADRSTOR),      ADDRESS_OF(REQRET),
        ADDRESS_OF(CLPASS),        ADDRESS_OF(DSPLOCK),       ADDRESS_OF(MONSAVE),
        ADDRESS_OF(MONSAVE1),      ADDRESS_OF(VERBREG),       ADDRESS_OF(NOUNREG),
        ADDRESS_OF(DSPLIST),       ADDRESS_OF(MARKSTAT),      ADDRESS_OF(IMUCADR),
        ADDRESS_OF(OPTCADR),       ADDRESS_OF(RADCADR),       ADDRESS_OF(ATTCADR),
        ADDRESS_OF(LGYRO),         ADDRESS_OF(FLAGWRD4)};
#endif
    unsigned i;

#if LUMINARY099
    set_A(OCT30001); /* "DO NOT DISTURB ENGINE ON, OFF AND ISS WARNING." */
    wand_channel(DSALMOUT);
    clear_bits(ADDRESS_OF(FLAGWRD3), READRBIT); /* "CLEAR READRFLG FOR R29" */
    /* "CLEAR TURNON, REPOSITION, CDU ZERO AND REMODE BITS IN RADMODES ...
     * ALSO IF R29 HAD BEEN REQUESTED" (NOR29FLG = 0): bit 10. */
    cs(ADDRESS_OF(FLAGWRD3));
    mask(NR29FBIT);
    if (!A_is_zero()) set_A(BIT10);
    ad(OCT32001);
    com();
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    set_A(OCT27470); /* "DO NOT DISTURB IMU FLAGS" ... "LEAVE RR LOCKON ENABLE ALONE." */
    wand_channel(CHAN12);
    clear_bits(ADDRESS_OF(FLAGWRD5), NORRMBIT); /* "ENABLE R25." */
    clear_bits(ADDRESS_OF(FLAGWRD5), R77FLBIT);
    set_A(OCT74160); /* "DO NOT DISTURB TELEMETRY FLAGS, RESET TRAP FLAGS, AND ENABLE T6RUPT" */
    wand_channel(CHAN13);
    set_A(BIT12); /* "REENABLE RUPT10" */
    wor_channel(CHAN13);
    set_A(BIT6); /* "DO NOT DISTURB GYRO ENABLE FLAG." */
    wand_channel(CHAN14);
#else
    set_A(OCT77603); /* "TURN OFF UPLINK ACTY, TEMP CAUTION, KR, FLASH, OP. ERROR" */
    wand_channel(DSALMOUT);
    set_A(OCT74777); /* "TURN OFF TEST ALARMS, STANDBY ENABLE." */
    wand_channel(CHAN13);
    set_A(negate(PRIO25)); /* "CLEAR R21MARK, P21FLAG, AND SKIPVHF BIT." */
    mask_at(ADDRESS_OF(FLAGWRD2));
    ad(SKIPVBIT); /* "NOW SET SKIPVHF FLAG." */
    ts(ADDRESS_OF(FLAGWRD2));
#endif

    store_constant(EBANK_ADDRESS, STARTEB); /* E3 */
    set_A(NEG1_2);
    for (i = 8; i-- > 0;) ts(ADDRESS_OF(LST1) + i);
    set_A(negate(ENDTASK[0]));
    for (i = 0; i < 18u; i += 2u) ts(ADDRESS_OF(LST2) + i);
    set_A(negate(ENDTASK[1]));
    for (i = 1; i < 18u; i += 2u) ts(ADDRESS_OF(LST2) + i);

    set_A(NEG0);
    for (i = 0; i < CORE_SETS; ++i) ts(ADDRESS_OF(PRIORITY) + CORE_SET_WORDS * i);
    ts(ADDRESS_OF(DSRUPTSW));
    ts(ADDRESS_OF(NEWJOB));
    store_constant(VACNUSE_ADDRESS(1u), VAC1ADRC);
    for (i = 2; i <= VAC_AREAS; ++i) {
        ad(LTHVACA);
        ts(VACNUSE_ADDRESS(i));
    }

    /* DSPOFF: blank the DSKY's registers, DSPTAB +10D down to +0 ("R1, R2,
     * R3" and the program, verb and noun). */
    set_A(TEN);
    do {
        ts(ADDRESS_OF(MPAC));
        set_A(negate(BIT12));
        ts(indexed(ADDRESS_OF(DSPTAB), MPAC(0)));
    } while (ccs_at(ADDRESS_OF(MPAC)) == POSITIVE);
    ts_each(display_words, COUNT(display_words));
    store_constant(ADDRESS_OF(NOUT), NOUTCON);
#if LUMINARY099
    store_constant(ADDRESS_OF(SAMPLIM), negate(ONE));
    set_A(BIT6); /* "LEAVE BIT 6 UNCHANGED" */
    mask_at(ADDRESS_OF(IMODES33));
    ad(IM33INIT); /* "NO PIP OR TM FAILS." */
    ts(ADDRESS_OF(IMODES33));
#else
    set_A(BIT14);
    mask_at(ADDRESS_OF(EXTVBACT));
    ts(ADDRESS_OF(EXTVBACT));
#endif
    store_constant(ADDRESS_OF(SELFRET), LESCHK); /* "SELF CHECK GO-TO REGISTER." */
    store_constant(ADDRESS_OF(DSPCOUNT), negate(VD1));
    tc_through(Q_ADDRESS);
}

/* ---- The rest ---- */

#if COMANCHE055
/* T5IDLOC -- CM p. 192.
 * "T5RUPT COMES HERE EVERY 163.84 SECS WHEN NOBODY IS USING IT." */
void t5idloc(void)
{
    ca(L_ADDRESS);
    continue_at(NOQRSM + 1u);
}
#endif

/* GOTOPOOH -- LM p. 224, CM p. 194.
 * Go to P00. The LM sets restart group 4 to phase 33 (the "4.33 SPOT FOR
 * GOP00FIX"), the CM restarts there through PHASCHNG (OCT 14); then
 * POSTJUMP to GOP00FIX. */
void gotopooh(void)
{
#if LUMINARY099
    set_A(OCT33);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE4));
    tc_at(GOTOPOOH + 4u);
    continue_at(POSTJUMP);
#else
    tc_at(GOTOPOOH);
    continue_at(PHASCHNG);
#endif
}

#if COMANCHE055
/* GOTOPOOH, after PHASCHNG -- CM p. 194. */
void gotopooh_after_phaschng(void)
{
    tc_at(GOTOPOOH + 2u);
    continue_at(POSTJUMP);
}
#endif
