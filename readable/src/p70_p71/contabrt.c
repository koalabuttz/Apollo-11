/* SPDX-License-Identifier: MIT */
/* P70-P71.agc, pp. 830-832 (Luminary099 only, bank 5): CONTABRT, which
 * leaves the interrupt R10,R11 may run in, and ABRTJASK, which turns the
 * landing into the abort: the program number, the flags and the restart
 * groups, the engine on, and the landing's jobs and tasks cleared by ENEMA
 * (POSTJUMP), with GOABORT to start after it. The faithful C is
 * c64-port/src/lm_p70_p71_file.c. */
#include "p70_p71.h"
#include "../agc/names.h"

#if LUMINARY099

/* CONTABRT: the instruction an interrupt's RESUME runs first (BRUPT) made
 * TCF ABRTJASK (ABRTJADR's word), and RESUME: the interrupt ends, and
 * ABRTJASK runs. (Out of an interrupt too, RESUME runs the word in BRUPT.)
 * The faithful port takes a TCF to fixed memory in BRUPT, as here. */
void contabrt(void)
{
    set_A(ABRTJADR);
    ts(ADDRESS_OF(BRUPT)); /* (A is a constant: TS does not skip) */
    if (OPERATION_CODE(BRUPT) != 1u || ADDRESS_FIELD(BRUPT) < 02000u) not_modelled();
    if (agc.in_interrupt) *agc.in_interrupt = 0;
    continue_at(ADDRESS_FIELD(BRUPT));
}

/* Each restart group's pair is its phase and the phase complemented. */
static void end_group(unsigned minus_phase)
{
    set_A(NEG0); /* EXTEND; DCA NEG0: -0, and ZERO, the word after it */
    set_L(ZERO);
    dxch(minus_phase);
}

/* ABRTJASK: with Q -0 for P70 or two for P71 (from P70A or P71A),
 *
 *     PHASE4 = 27 + Q          (the abort's restart point, 4.27 or 4.31)
 *     MODREG = DISPDEX = MODE70 or MODE71 (INDEX Q; "INSURE DISPDEX IS
 *                                          POSITIVE")
 *
 * then for P71 (Q positive) "SET APSFLAG ... PRIOR TO THE ENEMA", and DAPBOOLS
 * without DAPBITS ("TURN OFF: ULLAGE, DRIFT, AND XOVINHIB"), "SET
 * ENGONFLG", "INSURE THAT THE ENGINE IS ON, IF ARMED" (channel 11: the
 * engine-on bit 13 set, PRIO30's bits 13 and 14 cleared first), "TERMINATE
 * R12" (FLGWRD11 only LRBYBIT), "SET R10FLAG TO SUPPRESS OUTPUTS TO THE
 * CROSS-POINTER DISPLAY", and CLRADMOD, "INSURE RADMODES PROPERLY SET FOR
 * R29". From any other Q the AGC runs the same words: an overflow in AD Q
 * skips the COM, an index past MODE71 reads the word there (one in
 * erasable memory, or MODE70's neighbours; others the faithful port does
 * not take), and CCS Q of a negative Q adds its magnitude less one to
 * FLGWRD10. */
void abrtjask(void)
{
    word address;

    set_A(OCTAL27);
    ad_at(Q_ADDRESS);
    if (!ts(L_ADDRESS)) com();
    dxch(ADDRESS_OF(MINUS_PHASE4));
    address = (word)indexed(MODE70, overflow_corrected(Q));
    if (address < 02000u && address != Z_ADDRESS)
        ca(address);
    else if (address >= MODE70 && address <= MODE70 + 2u)
        set_A(fetch_fixed(address));
    else
        not_modelled();
    ts(ADDRESS_OF(MODREG));
    ts(ADDRESS_OF(DISPDEX));
    switch (ccs_at(Q_ADDRESS)) {             /* "SET APSFLAG IF P71." */
    case POSITIVE: cs(ADDRESS_OF(FLGWRD10)); /* fall through */
    case IS_PLUS_ZERO: mask(APSFLBIT);       /* fall through */
    case NEGATIVE: ads(ADDRESS_OF(FLGWRD10)); break;
    case IS_MINUS_ZERO: break;
    }
    set_A(negate(DAPBITS));
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(DAPBOOLS));
    cs(ADDRESS_OF(FLAGWRD5)); /* "SET ENGONFLG." */
    mask(ENGONBIT);
    ads(ADDRESS_OF(FLAGWRD5));
    set_A(negate(PRIO30));
    rand_channel(DSALMOUT);
    ad(BIT13);
    write_channel(DSALMOUT);
    set_A(LRBYBIT);
    ts(ADDRESS_OF(FLGWRD11));
    cs(ADDRESS_OF(FLAGWRD0));
    mask(R10FLBIT);
    ads(ADDRESS_OF(FLAGWRD0));
    tc_to(ABRTJASK + 036u, CLRADMOD);
}

/* CLRADMOD's return: "LOAD TEVENT FOR THE DOWNLINK" (TIME2), the average-G
 * exit SERVEXIT (AVGEXIT), restart groups 1, 3 and 6 ended (-0 and +0),
 * group 4 at 4.3 ("SET UP 4.3SPOT FOR GOABORT"), "SET T5RUPT TO CALL
 * DAPIDLER IN 40 MILLISECONDS" (TIME5), and ENEMA (POSTJUMP). (Every TS
 * stores a constant: none skips.) */
void abrtjask_after_clradmod(void)
{
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(TEVENT));
    set_A(SVEXITAD[0]);
    set_L(SVEXITAD[1]);
    dxch(ADDRESS_OF(AVGEXIT));
    end_group(ADDRESS_OF(MINUS_PHASE1));
    end_group(ADDRESS_OF(MINUS_PHASE3));
    end_group(ADDRESS_OF(MINUS_PHASE6));
    set_A(THREE);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE4));
    set_A(OCT37774);
    ts(ADDRESS_OF(TIME5));
    tc_to(ABRTJASK + 064u, POSTJUMP); /* CADR ENEMA */
}

#endif
