/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 2b: ENTER and the verb fan --
 * LM pp. 406-412, CM pp. 323-329. The faithful C is the port's Pinball
 * modules (c64-port/src/lanec_pw_*.c, lanec_pin_head_*.c).
 *
 * "ENTER PASS 0 IS THE EXECUTE FUNCTION. HIGHER ORDER ENTERS ARE TO LOAD
 * DATA. THE SIGN OF REQRET DETERMINES THE PASS, + FOR PASS 0, - FOR HIGHER
 * PASSES." Pass 0 looks the noun up (LODNNTAB, in the noun tables' bank)
 * unless the verb is 28 or more, and goes to the verb's routine through
 * VERBTAB (VERBFAN); verbs 40 and up are the extended verbs, in their own
 * bank (GOEXTVB). A mixed noun's components are fetched into MIXTEMP for
 * the display verbs (MIXNOUN). A noun whose address the operator must
 * give ("MACHINE CADR TO BE SPECIFIED") asks for it in R3 (REQADD). A load
 * verb asks for its data with REQDATX, REQDATY or REQDATZ: the register
 * blanked, the verb and noun flashing, and REQRET made negative with the
 * place to come back to. The operator's ENTER then takes the word
 * (ENTPASHI, ACCEPTWD) and goes back there. UPDATNN and UPDATVB put the
 * noun or the verb on the DSKY.
 *
 * Bank 41, the same code in both missions but for DPTEST's table, which
 * is read from the rope. As in part 2a, each function runs from a label,
 * or from the word a call returns to, to a call or jump to another
 * routine, which ends it with Q and Z set; a TC that skips within the
 * routine (TC +2, TC MIXNN2, the MIXNN1 loop) is its control flow. The
 * subroutines that come straight back (SETNCADR, FLASHON, RELDSP,
 * SFRUTMIX, DPTEST, LODNNTAB through DXCH Z) are C calls. */
#include "pinball.h"
#include "../interpreter/interpreter.h"
#include "../pinball_noun_tables/pinball_noun_tables.h"
#include "../agc/names.h"

/* ---- The standard lead-ins ---- */

/* NVSUBB and LOADLV1 -- LM p. 406, CM p. 323. "STANDARD LEAD INS. DONT
 * MOVE.": the bank's first two words, which other banks reach by
 * BANKCALL. */
void nvsubb(void)
{
    tc(NVSUBB, NVSUB1);
}

void loadlv1(void)
{
    tc(LOADLV1, LOADLV);
}

/* ENTERJMP -- LM p. 399, CM p. 316 (bank 40). The ENTER key's word in
 * CHARIN's table: to ENTER in bank 41. */
void enterjmp(void)
{
    tc(ENTERJMP, POSTJUMP); /* CADR ENTER */
}

/* ---- ENTER ---- */

static void entpashi(void);
static void acceptwd(void);
static void testnn(void);
static void useadd_on(void);
static void intmctbs_plus2(void);
static void reqcom(void);
static void updat1(void);

/* ENTER -- LM p. 406, CM p. 323. CLPASS +0, ENTRET TC ENDOFJOB (ENDINST:
 * the end of a keyboard job), and the pass REQRET's sign gives. */
void enter(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(CLPASS));
    set_A(ENDINST);
    ts(ADDRESS_OF(ENTRET));
    switch (ccs_at(ADDRESS_OF(REQRET))) {
    case POSITIVE: tc(ENTER + 5u, ENTPAS0); return;     /* "IF +, PASS 0" */
    case IS_PLUS_ZERO: tc(ENTER + 6u, ENTPAS0); return; /* "IF +, PASS 0" */
    case NEGATIVE: tc_at(ENTER + 7u); break;            /* TC +1: "IF -, NOT PASS 0" */
    case IS_MINUS_ZERO: break;
    }
    entpashi();
}

/* ENTPASHI -- LM p. 406, CM p. 323. A higher pass, the loading of a word.
 * REQRET is -(MMCHANG +1) when the word is a new major mode: "IF L/ 2 CHAR
 * IN FOR MM CODE, ALARM AND RECYCLE (DECIDE AT MMCHANG+1)." "IF DEC, ALARM
 * IF L/ 5 CHAR IN FOR DATA, BUT LEAVE REQRET - AND FLASH ON, SO OPERATOR
 * CAN SUPPLY MISSING NUMERICAL CHARACTERS AND CONTINUE." DSPCOUNT goes
 * negative once the fifth character is in. */
static void entpashi(void)
{
    set_A(MMADREF);
    ad_at(ADDRESS_OF(REQRET));
    if (A_is_zero()) { /* EXTEND; BZF ACCEPTWD */
        acceptwd();
        return;
    }
    set_A(THREE);
    mask_at(ADDRESS_OF(DECBRNCH)); /* CCS A: MASK leaves 0 to 3 */
    if (ccs_at(A_ADDRESS) != POSITIVE) {
        tc_at(ENTPASHI + 8u); /* TC ACCEPTWD: "OCTAL. ANY NUMBER OF CHAR OK." */
        acceptwd();
        return;
    }
    tc_at(ENTPASHI + 7u); /* TC +2 */
    switch (ccs_at(ADDRESS_OF(DSPCOUNT))) {
    case POSITIVE: tc(ENTPASHI + 10u, GODSPALM); return; /* "LESS THAN 5 CHAR DEC" */
    case IS_PLUS_ZERO: tc(ENTPASHI + 11u, GODSPALM); return;
    case NEGATIVE: tc_at(ENTPASHI + 12u); break; /* TC +1: "5 CHAR IN" */
    case IS_MINUS_ZERO: break;
    }
    acceptwd();
}

/* ACCEPTWD -- LM p. 406, CM p. 323. "5 CHAR IN": "SET REQRET +.", the
 * flash off, and back to the routine that asked, at the word REQRET
 * holds. */
static void acceptwd(void)
{
    cs(ADDRESS_OF(REQRET));
    ts(ADDRESS_OF(REQRET));
    tc_at(ACCEPTWD + 2u);
    flashoff();
    tc(ACCEPTWD + 3u, ADDRESS_OF(REQRET));
}

/* ENTPAS0 and TESTVB -- LM p. 407, CM p. 324. "NOUN VERB SUB ENTERS
 * HERE". Pass 0: DECBRNCH +0, "BLOCK FURTHER NUM CHAR, SO THAT STRAY CHAR
 * DO NOT GET INTO VERB OR NOUN LTS.", the verb kept for a recycle
 * (VERBSAVE, complemented), and "IF VERB IS G/E LOWVB, SKIP NOUN TEST." */
void entpas0(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DECBRNCH));
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    /* TESTVB */
    cs(ADDRESS_OF(VERBREG));
    ts(ADDRESS_OF(VERBSAVE)); /* "SAVE VERB FOR POSSIBLE RECYCLE." */
    ad(LOWVERB);              /* "LOWVERB - VB" */
    if (A_is_zero_or_minus()) {
        continue_at(VERBFAN); /* EXTEND; BZMF VERBFAN: "VERB G/E LOWVERB" */
        return;
    }
    testnn();
}

/* The noun tables' reader, in their own bank: EXTEND; DCA LODNNLOC; DXCH
 * Z at at ("SWITCH BANKS TO NOUN TABLE READING ROUTINE."), back at at +1
 * with NNADTEM, NNTYPTEM, MIXBR and for a mixed noun RUTMXTEM and
 * IDAD1TEM-IDAD3TEM loaded. */
static void read_noun_tables(word at)
{
    word noun = fetch(ADDRESS_OF(NOUNREG));

    /* A noun past the tables would have LODNNTAB's INDEX change the
     * instruction it indexes. */
    if (noun > 99u && noun != MINUS_ZERO) not_modelled();
    dca(LODNNLOC);
    dtcb(at);
    lodnntab();
}

/* TESTNN -- LM p. 407, CM p. 324. "VERB L/ LOWVERB": the noun looked up,
 * then by MIXBR (INDEX MIXBR; TC +0) on to MIXNOUN for a mixed noun, or
 * for a normal one by NNADTEM's sign: "NORMAL IF +" (its ECADR, one less,
 * into NOUNCADR at VERBFAN -2), "NOT IN USE IF +0", "SPECIFY MACHINE CADR
 * IF -" (REQADD), "AUGMENT MACHINE CADR IF -0" (the address the operator
 * gave last time, one on). */
static void testnn(void)
{
    word mixbr;

    read_noun_tables(TESTNN + 2u);
    mixbr = fetch(ADDRESS_OF(MIXBR));
    if (mixbr == 2u) {
        tc_at(TESTNN + 4u);
        tc(TESTNN + 6u, MIXNOUN); /* "MIXED" */
        return;
    }
    if (mixbr != 1u) not_modelled(); /* the TC would land on another word */
    tc_at(TESTNN + 4u);
    tc_at(TESTNN + 5u); /* TC +2: "NORMAL" */
    switch (ccs_at(ADDRESS_OF(NNADTEM))) {
    case POSITIVE: tc(TESTNN + 8u, VERBFAN - 2u); return;
    case IS_PLUS_ZERO: tc(TESTNN + 9u, GODSPALM); return;
    case NEGATIVE: tc(TESTNN + 10u, REQADD); return;
    case IS_MINUS_ZERO: break;
    }
    incr(ADDRESS_OF(NOUNCADR));
    tc_at(TESTNN + 12u);
    setnadd(); /* "ECADR FROM NOUNCADR, SETS EB, NOUNADD." */
    tc(TESTNN + 13u, INTMCTBS + 2u);
}

/* ---- A machine address the operator gives ---- */

/* REQADD -- LM p. 407, CM p. 324. "SET CLPASS FOR PASS 0 ONLY". "TEST IF
 * REACHED HERE FROM INTERNAL OR FROM EXTERNAL": a keyboard ENTER left
 * ENTRET TC ENDOFJOB; NVSUB, internal, left another word there and the
 * address in MPAC +2 (INTMCTBS). From the keyboard, ask for it in R3
 * (REQDATZ). */
void reqadd(void)
{
    set_A(BIT15);
    ts(ADDRESS_OF(CLPASS));
    set_A(negate(ENDINST));
    ad_at(ADDRESS_OF(ENTRET));    /* ENTEXIT */
    if (A_is_zero()) {            /* EXTEND; BZF +2 */
        tc(REQADD + 7u, REQDATZ); /* "EXTERNAL MACH CADR TO BE SPECIFIED" */
        return;
    }
    tc(REQADD + 6u, INTMCTBS);
}

/* REQADD +8, where the loaded address comes back (ACCEPTWD's TC REQRET):
 * "ALARM AND RECYCLE IF DECIMAL USED FOR MCTBS." "OCTAL USED OK", "BLOCK
 * NUM CHAR IN"; "EXTERNAL MCTBS DISPLAY WILL LEAVE FLASH ON IF ENDIDLE
 * NOT = +0." (CADRSTOR), then USEADD. */
void reqadd_after_reqdatz(void)
{
    switch (ccs_at(ADDRESS_OF(DECBRNCH))) {
    case POSITIVE: tc(REQADD + 9u, ALMCYCLE); return;
    case IS_PLUS_ZERO:
        set_A(negate(VD1)); /* CS VD1 */
        /* fall through */
    case NEGATIVE:
        ts(ADDRESS_OF(DSPCOUNT));
        /* fall through */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE: tc_at(REQADD + 13u); break; /* TC +3 */
    case IS_PLUS_ZERO: tc(REQADD + 14u, USEADD); return;
    case NEGATIVE: tc_at(REQADD + 15u); break; /* TC +1 */
    case IS_MINUS_ZERO: break;
    }
    tc_at(REQADD + 16u);
    flashon();
    useadd_on();
}

/* USEADD -- LM p. 407, CM p. 324. The address the operator loaded, from
 * ZREG: "ECADR INTO NOUNCADR. SET EB, NOUNADD.", the noun tables read
 * again, and on to the verb. */
void useadd(void)
{
    useadd_on();
}

static void useadd_on(void)
{
    xch(ADDRESS_OF(ZREG));
    tc_at(USEADD + 1u);
    setncadr();
    read_noun_tables(USEADD + 4u);
    tc(USEADD + 5u, VERBFAN);
}

/* INTMCTBS -- LM p. 408, CM p. 325. "INTERNAL MACH CADR TO BE SPECIFIED."
 * "NVSUB CALL LEFT CADR IN MPAC+2 FOR MACH CADR TO BE SPECIFIED." "DONT
 * DISPLAY CADR IF VB = 05." Otherwise the address in octal in R3
 * (DSPOCTWD). INTMCTBS +2 is where TESTNN's augmented address joins. */
void intmctbs(void)
{
    ca(ADDRESS_OF(MPAC) + 2u);
    tc_at(INTMCTBS + 1u);
    setncadr(); /* "ECADR INTO NOUNCADR. SET EB. NOUNADD." */
    intmctbs_plus2();
}

void intmctbs_plus2_entry(void)
{
    intmctbs_plus2();
}

static void intmctbs_plus2(void)
{
    set_A(negate(FIVE));
    ad_at(ADDRESS_OF(VERBREG));
    if (A_is_zero()) { /* EXTEND; BZF VERBFAN */
        continue_at(VERBFAN);
        return;
    }
    set_A(R3D1); /* "VB NOT = 05. DISPLAY CADR." */
    ts(ADDRESS_OF(DSPCOUNT));
    ca(ADDRESS_OF(NOUNCADR));
    tc(INTMCTBS + 9u, DSPOCTWD);
}

void intmctbs_after_dspoctwd(void)
{
    tc(INTMCTBS + 10u, VERBFAN);
}

/* VERBFAN -2: TESTNN's normal noun, its ECADR from NOUNCADR (NNADTEM, one
 * less) into NOUNCADR. SETNCADR comes back to VERBFAN, where the faithful
 * port's step ends. */
void verbfan_minus2(void)
{
    ad(ONE);
    tc_at(VERBFAN - 1u);
    setncadr(); /* "ECADR INTO NOUNCADR. SETS EB, NOUNADD." */
}

/* ---- The verb fan ---- */

/* VERBFAN -- LM p. 408, CM p. 325. "VERB L/ LST2CON": through VERBTAB
 * (VBFANDIR). "VERB G/ LST2CON" or equal: the verb less 40 into MPAC,
 * "RELEASE DISPLAY SYST" (RELDSP), and "GO TO GOEXTVB WITH VB=40 IN MPAC"
 * (a verb of 40 leaves +0). */
void verbfan(void)
{
    set_A(negate(LST2CON));     /* CS LST2CON */
    ad_at(ADDRESS_OF(VERBREG)); /* "VERB = LST2CON" */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        ad(ONE); /* "VERB G/ LST2CON" */
        /* fall through */
    case IS_PLUS_ZERO: tc_at(VERBFAN + 4u); break;     /* TC +2 */
    case NEGATIVE: tc(VERBFAN + 5u, VBFANDIR); return; /* "VERB L/ LST2CON" */
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(MPAC));
    tc(VERBFAN + 7u, RELDSP); /* "RELEASE DISPLAY SYST" */
}

/* VERBFAN +8, where RELDSP returns. */
void verbfan_after_reldsp(void)
{
    tc(VERBFAN + 8u, POSTJUMP); /* CADR GOEXTVB */
}

/* VBFANDIR -- LM p. 408, CM p. 325. VERBTAB's CADR for the verb, by
 * BANKJUMP: the display verbs 01-07, MONITOR (11-17), the load verbs
 * 21-25, DSPFMEM (27), and the verbs that make no noun test, 30-37. The
 * spares go to GODSPALM. */
void vbfandir(void)
{
    ca(indexed_by(ADDRESS_OF(VERBTAB), fetch(ADDRESS_OF(VERBREG))));
    tc(VBFANDIR + 2u, BANKJUMP);
}

/* ---- A mixed noun ---- */

/* DPTEST -- LM pp. 410-411, CM pp. 327-328. "ENTER WITH SF ROUT NUMBER IN
 * A. RETURNS TO L+1 IF NO DP. RETURNS TO L+2 IF DP." INDEX A; TCF +1 runs
 * the routine's word of the table: TC Q, or TCF DPTEST1 (the table starts at DPTEST +2, where TCF +1 goes), whose INDEX Q;
 * TC 1 is "RETURN TO L+2". True for a double-precision routine. */
bool dptest(void)
{
    word routine = a_word();
    word back;

    if (routine > 13u) not_modelled(); /* past the table */
    if (fetch_fixed((word)(DPTEST + 2u + routine)) == Q_ADDRESS) {
        tc_return(Q_ADDRESS); /* TC Q: "NO DP" */
        return false;
    }
    back = (word)(Q + 1u);
    tc(DPTEST1 + 1u, back); /* TCF DPTEST1; INDEX Q; TC 1 */
    return true;
}

/* MIXNOUN, MIXNN1 and MIXNN2 -- LM p. 410, CM p. 327.
 * "MIXNOUN GETS DATA AND STORES IN MIXTEMP,+1,+2. IT SETS NOUNADD FOR
 * MIXTEMP." Only for the display verbs ("AVOID MIXNOUN SWAP IF VB NOT =
 * DISPLAY"). For each component, the third first (DECOUNT 2, 1, 0), its
 * address from IDAD1TEM-IDAD3TEM (an ECADR: SETEBANK), "TEST FOR DP (FOR
 * OCT DISPLAY). IF SO, GET MINOR PART ONLY.", and its word into MIXTEMP
 * +K. NOUNADD is left at MIXTEMP. */
void mixnoun(void)
{
    switch (ccs_at(ADDRESS_OF(NNADTEM))) {
    case POSITIVE: tc_at(MIXNOUN + 1u); break;             /* TC +4: "+ IN USE" */
    case IS_PLUS_ZERO: tc(MIXNOUN + 2u, GODSPALM); return; /* "+0 NOT IN USE" */
    case NEGATIVE: tc_at(MIXNOUN + 3u); break;             /* TC +2: "- IN USE" */
    case IS_MINUS_ZERO: tc_at(MIXNOUN + 4u); break;        /* TC +1: "-0 IN USE" */
    }
    set_A(negate(SIX));
    ad_at(ADDRESS_OF(VERBREG));
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +2: "VERB L/E 6" */
        tc(MIXNOUN + 9u, VERBFAN);
        return;
    }
    set_A(TWO);
    for (;;) {
        /* MIXNN1 */
        word decount;

        ts(ADDRESS_OF(DECOUNT));
        ad(MIXAD);
        ts(ADDRESS_OF(NOUNADD)); /* "SET NOUNADD TO MIXTEMP + K" */
        decount = fetch(ADDRESS_OF(DECOUNT));
        ca(indexed_by(ADDRESS_OF(IDAD1TEM), decount)); /* "GET IDADDTAB ENTRY FOR COMPONENT K" */
        ts(ADDRESS_OF(NOUNTEM));
        tc_at(MIXNN1 + 6u);
        sfrutmix(); /* "GET SF ROUT NUMBER IN A" */
        tc_at(MIXNN1 + 7u);
        if (dptest())
            incr(ADDRESS_OF(NOUNTEM)); /* "DP GET MINOR PART" */
        else
            tc_at(MIXNN1 + 8u); /* TC MIXNN2: "NO DP" */
        /* MIXNN2 */
        ca(ADDRESS_OF(NOUNTEM));
        mask(LOW11); /* "ESUBK (NO DP) OR (ESUBK)+1 FOR DP" */
        tc_at(MIXNN2 + 2u);
        setebank();                   /* "SET EBANK, LEAVE EADRES IN A." */
        ca(indexed_by(0u, a_word())); /* INDEX A; CA 0: "PICK UP C(ESUBK)" */
        xch(indexed_erasable(0u, fetch(ADDRESS_OF(NOUNADD)))); /* "STORE IN MIXTEM + K" */
        switch (ccs_at(ADDRESS_OF(DECOUNT))) {
        case POSITIVE: tc_at(MIXNN2 + 8u); continue; /* TC MIXNN1 */
        case IS_PLUS_ZERO: tc(MIXNN2 + 9u, VERBFAN); return;
        case NEGATIVE:
        case IS_MINUS_ZERO: not_modelled(); /* the CCS would run MIXAD */
        }
    }
}

/* ---- Asking for data ---- */

/* REQDATX, REQDATY, REQDATZ and REQCOM -- LM p. 411, CM p. 328. Ask for a
 * word in R1, R2 or R3: DSPCOUNT at its first digit, REQRET the caller's
 * return complemented (ENTER's higher pass goes back there), the register
 * blanked (5BLANK, by BANKCALL), the verb and noun flashing, and the job
 * ends (ENTEXIT: ENTRET holds TC ENDOFJOB for the keyboard). */
void reqdatx(void)
{
    set_A(R1D1);
    reqcom(); /* TCF REQCOM */
}

void reqdaty(void)
{
    set_A(R2D1);
    reqcom(); /* TCF REQCOM */
}

void reqdatz(void)
{
    set_A(R3D1);
    reqcom();
}

static void reqcom(void)
{
    ts(ADDRESS_OF(DSPCOUNT));
    cs(Q_ADDRESS);
    if (ts(ADDRESS_OF(REQRET))) not_modelled(); /* an overflow in Q would skip the TC */
    tc(REQCOM + 3u, BANKCALL);                  /* CADR 5BLANK */
}

void reqcom_after_5blank(void)
{
    tc_at(ENDRQDAT - 1u);
    flashon();
    tc(ENDRQDAT, ADDRESS_OF(ENTRET)); /* TC ENTEXIT */
}

/* ---- The noun and the verb on the DSKY ---- */

/* UPDATNN and PUTADD -- LM p. 411, CM p. 328. UPDATNN -1 stores A as the
 * noun first. The noun looked up; a normal noun's ECADR into NOUNCADR
 * (NNADTEM +1, and a +0 NNADTEM +0); for "MCTBS" and "MCTBI" (negative)
 * "DONT CHANGE NOUNADD". The noun on the DSKY at ND1. The return in
 * UPDATRET. */
static void updatnn_on(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(UPDATRET))) not_modelled(); /* an overflow in Q would skip the EXTEND */
    read_noun_tables(UPDATNN + 4u);
    switch (ccs_at(ADDRESS_OF(NNADTEM))) {
    case POSITIVE:
        ad(ONE); /* "NORMAL" */
        /* fall through */
    case IS_PLUS_ZERO:
        tc_at(PUTADD);
        setncadr(); /* PUTADD: "ECADR INTO NOUNCADR. SETS EB. NOUNADD." */
        break;
    case NEGATIVE: /* "MCTBS DONT CHANGE NOUNADD" */
    case IS_MINUS_ZERO: /* "MCTBI DONT CHANGE NOUNADD" */ break;
    }
    set_A(ND1);
    ts(ADDRESS_OF(DSPCOUNT));
    ca(ADDRESS_OF(NOUNREG));
    updat1(); /* TCF UPDAT1 */
}

void updatnn_minus1(void)
{
    ts(ADDRESS_OF(NOUNREG));
    updatnn_on();
}

void updatnn(void)
{
    updatnn_on();
}

/* UPDATVB and UPDAT1 -- LM pp. 411-412, CM pp. 328-329. UPDATVB -1 stores
 * A as the verb first. The verb on the DSKY at VD1. "CANT USE SWCALL TO
 * GO TO DSPDECVN, SINCE UPDATVB CAN ITSELF BE CALLED BY SWCALL.": POSTJUMP
 * to GOVNUPDT in bank 40, which comes back to UPDAT1 +2 the same way. */
static void updatvb_on(void)
{
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(UPDATRET))) set_A(VD1); /* an overflow in Q skips CAF VD1 */
    ts(ADDRESS_OF(DSPCOUNT));
    ca(ADDRESS_OF(VERBREG));
    updat1();
}

void updatvb_minus1(void)
{
    ts(ADDRESS_OF(VERBREG));
    updatvb_on();
}

void updatvb(void)
{
    updatvb_on();
}

static void updat1(void)
{
    tc(UPDAT1, POSTJUMP); /* CADR GOVNUPDT */
}

/* UPDAT1 +2, where GOVNUPDT comes back: the return in UPDATRET. */
void updat1_plus2(void)
{
    tc(UPDAT1 + 2u, ADDRESS_OF(UPDATRET));
}

/* GOALMCYC and GODSPALM -- LM p. 412, CM p. 329. GOALMCYC: "NEEDED
 * BECAUSE BANKJUMP CANT HANDLE F/F." GODSPALM: to DSPALARM in bank 40. */
void goalmcyc(void)
{
    tc(GOALMCYC, ALMCYCLE);
}

void godspalm(void)
{
    tc(GODSPALM, POSTJUMP); /* CADR DSPALARM */
}
