/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part A (Comanche055 pp. 1066-1069, bank
 * 20): "THESE ARE CALLED FOR THE VARIOUS INITIALIZATIONS NEEDED."
 *
 * CM/DAPON, from P61-P67, turns the service module's RCS DAP off (its T5
 * and T6 programs idled, the jets zeroed, the DAP's configuration bits
 * cleared "FOR ENTRY"), waits half a second at a time until GAMDIFSW is
 * set, then starts the entry DAP: the needles kept zero, attitude hold
 * "UNTIL KEYBOARD ESTABLISHES HEADSUP", CMDAPARM set and NODOFLAG set "TO
 * PREVENT FURTHER V 37 ENTRIES", and on to P62.1.
 *
 * CM/DAPIC: "INITIALIZE CM/DAP.  WAITLIST CALL FOR READGYMB.  SET SWITCH
 * CM/DSTBY =1 SO READACCS WILL ENTER A WTLST CALL FOR  SETJTAG ."
 * ("CMDAPARM  = 0, SO ONLY BODY RATE AND ATTITUDE CALCULATIONS ARE
 * DONE."), and on to P62.2. */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- CM/DAPON -- pp. 1066-1067 ---- */

/* CM/DAPON: E bank 6 (EBAOG), "RESET DAPBIT1.  T5 RESTART IDENTIFIER."
 * (DOWNFLAG, ADRES DAPBIT1), */
void cm_dapon(void)
{
    set_A(EBAOG);
    ts(EBANK_ADDRESS);
    tc_to(CM_DAPON + 2u, DOWNFLAG); /* ADRES DAPBIT1 */
}

/* CM/DAPON +4: "RESET DAPBIT2", */
void cm_dapon_after_dapbit1(void)
{
    tc_to(CM_DAPON + 4u, DOWNFLAG); /* ADRES DAPBIT2 */
}

/* CM/DAPON +6: "DISABLE RCS CALCULATION" (T5LOC to T5IDLOC, the 2CADR
 * T5IDLER1), */
void cm_dapon_after_dapbit2(void)
{
    dca(T5IDLER1); /* EXTEND; DCA */
    cm_dapon_t5loc();
}

/* CM/DAPON +010: "DISABLE RCS JET CALLS" (T6LOC too), and "JETS DEPARTED
 * ON SM. ZERO JET BITS." (FLUSHJET). */
void cm_dapon_t5loc(void)
{
    dxch(ADDRESS_OF(T5LOC));
    dca(T5IDLER1); /* EXTEND; DCA */
    dxch(ADDRESS_OF(T6LOC));
    tc_to(CM_DAPON + 014u, FLUSHJET);
}

/* CM/DAPON +015: "SET CONFIG BITS =0 FOR ENTRY" (bits 13-15 of DAPDATR1;
 * a MASK leaves no overflow for the TS to skip on), */
void cm_dapon_after_flushjet(void)
{
    set_A(negate(BITS13_14_15)); /* CS */
    mask_at(ADDRESS_OF(DAPDATR1));
    ts(ADDRESS_OF(DAPDATR1));
    cm_dapon_plus4_tc();
}

/* CM/DAPON +020: on at the word the listing labels +4 (TC +4). */
void cm_dapon_plus4_tc(void)
{
    tc_at(CM_DAPON + 020u);
    notyet_plus4();
}

/* NOTYET: half a second's wait (BANKCALL, CADR DELAYJOB: "(DELAYJOB DOES
 * INHINT)"), */
void notyet(void)
{
    set_A(HALF_SEC);
    notyet_bankcall();
}

void notyet_bankcall(void)
{
    tc_to(NOTYET + 1u, BANKCALL); /* CADR DELAYJOB */
}

/* NOTYET +3 (+4): GAMDIFSW ("GAMDIFSW = 94D BIT11, INITLY=0"): "IF ZERO,
 * WAIT UNTIL CM/POSE UPDATE." (EXTEND; BZF NOTYET). Set: "ACTIVATE
 * CM/DAP" (RCSFLAGS -1: "USE BIT3 TO INITIALIZE NEEDLER ON NEXT PASS."),
 * P63FLAG -1 too, "SO WAKEP62 WILL NOT BE INITIATED UNTIL HEADSUP IS SET
 * IN P62."; JETAG and PAXERR1 +0 (CA 7): "KEEP NEEDLES ZERO UNTIL DAP
 * UPDATE". Then, with interrupts inhibited, ALFA/180 (EXTEND; DCA), */
void notyet_plus4(void)
{
    set_A(BIT11);
    mask_at(ADDRESS_OF(CM_FLAGS));
    if (A_is_zero()) {
        notyet();
        return;
    }
    set_A(negate(ONE)); /* CS */
    ts(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(P63FLAG));
    ca(ZERO_ADDRESS);
    ts(ADDRESS_OF(JETAG));
    ts(ADDRESS_OF(PAXERR1));
    inhint();
    dca(ADDRESS_OF(ALFA_180));
    notyet_alfacom();
}

/* NOTYET +020: to ALFACOM, "DO ATTITUDE HOLD UNTIL KEYBOARD ESTABLISHES
 * HEADSUP."; ROLL/180 to ROLLHOLD, "FOR ATTITUDE HOLD IN MODE +1.", and
 * half of it to ROLLC ("NOT INTERESTED IN LO WORD."); CMDAPARM set
 * ("CMDAPARM =93D BIT12  INITLY=0", "SET BIT TO 1."), NODOFLAG set (bit 1
 * of FLAGWRD2), and on to P62.1 (POSTJUMP). A from a CA or an MP has no
 * overflow for a TS to skip on. */
void notyet_alfacom(void)
{
    dxch(ADDRESS_OF(ALFACOM));
    ca(ADDRESS_OF(ROLL_180));
    ts(ADDRESS_OF(ROLLHOLD));
    mp(HALF); /* EXTEND; MP */
    ts(ADDRESS_OF(ROLLC));
    cs(ADDRESS_OF(CM_FLAGS));
    mask(BIT12);
    ads(ADDRESS_OF(CM_FLAGS));
    cs(ADDRESS_OF(FLAGWRD2));
    mask(BIT1);
    ads(ADDRESS_OF(FLAGWRD2));
    relint();
    tc_to(NOTYET + 035u, POSTJUMP); /* CADR P62.1 */
}

/* ---- CM/DAPIC -- pp. 1068-1069 ---- */

/* CM/DAPIC: E bank 6, interrupts inhibited; CM/DAP2C: the time since the
 * last PIPA reading, "DELTA TIME SINCE PIPUP" (TIME1 less PIPTIME +1, the
 * sum of POS1/2 and POS1/2 there to make the "OVFL GUARANTEED"), */
void cm_dapic(void)
{
    set_A(EBAOG);
    ts(EBANK_ADDRESS);
    inhint();
    cs(ADDRESS_OF(PIPTIME) + 1u);
    ts(ADDRESS_OF(JETEM)); /* "PRIO OF P62 L PRIO AVG.:PIPTM=PIPTM1" */
    set_A(POS1_2);
    ad(POS1_2);
    ad_at(ADDRESS_OF(TIME1));
    cm_dap2c_jetem();
}

/* CM/DAP2C +5: into JETEM; then less 5, then less 10 at a time (CCS A,
 * AD -CDUT+1, TCF -2) until CCS finds it negative or -0, then one more
 * ("SEND NO ZERO TO WTLST"): READGYMB's first wait, kept in CM/GYMDT "FOR
 * RESTART". (TCF -2 would take a +0 round again for ever: the sums never
 * give one, a ones' complement sum being -0, so it is not modelled.) */
void cm_dap2c_jetem(void)
{
    ads(ADDRESS_OF(JETEM));
    set_A(negate(FIVE)); /* CS */
    ad_at(ADDRESS_OF(JETEM));
    for (;;) {
        enum sign sign = ccs_register(A_ADDRESS);

        if (sign == POSITIVE) {
            ad(MINUS_CDUT1);
            continue;
        }
        if (sign == IS_PLUS_ZERO) not_modelled();
        break;
    }
    ad(ONE);
    if (ts(ADDRESS_OF(CM_GYMDT))) {
        /* An overflow skips the TC WAITLIST: the 2CADR's first word,
         * READGYMB's address 2434, runs as TC 2434, in this bank. */
        tc_to(CM_DAP2C + 017u, READGYMB);
        return;
    }
    tc_to(CM_DAP2C + 016u, WAITLIST); /* 2CADR READGYMB */
}

/* CM/DAP2C +020: TC 2434's return, the 2CADR's second word (6, its
 * E bank). The faithful port runs it as CAF 6 (on the AGC the word 00006
 * is EXTEND); A gets 6, which the CS next replaces. */
void cm_dap2c_2cadr(void)
{
    set_A(6u);
    cm_dap2c_after_waitlist();
}

/* CM/DAP2C +021: CM/FLAGS' GAMDIFSW, GYMDIFSW, CM/DSTBY, DAPARM, .05GSW,
 * LATSW and ENTRYDSP cleared (CM/SWIC1, OCT 16017), then "SET CM/DSTBY,
 * LATSW" (CM/SWIC2, ten); and "DISABLE ENTRY DISPLAY, SINCE DES. GIMB.
 * CALC. (P62.3) GOES TO ENDEXIT." An overflow skips to CM/DAP2C +026. */
void cm_dap2c_after_waitlist(void)
{
    set_A(negate(CM_SWIC1)); /* CS */
    mask_at(ADDRESS_OF(CM_FLAGS));
    ad(CM_SWIC2);
    if (ts(ADDRESS_OF(CM_FLAGS)))
        continue_at(CM_DAP2C + 026u);
    else
        continue_at(CM_DAP2C + 025u);
}

/* CM/DAP2C +025: +0 (CA 7) to BETA/180, */
void cm_dap2c_beta(void)
{
    ca(ZERO_ADDRESS);
    cm_dap2c_beta_ts();
}

/* CM/DAP2C +026: "NECESSARY:  NO OVFL CORRECTION" (an overflow skips to
 * SW/NDX, A +1 or -1); 1 to SW/NDX, "INITIALIZE THE TM OF BODY RATES VIA"
 * "UPBUFF.", */
void cm_dap2c_beta_ts(void)
{
    if (!ts(ADDRESS_OF(BETA_180))) set_A(ONE);
    ts(ADDRESS_OF(SW_NDX));
    cm_dap2c_2phschng();
}

/* CM/DAP2C +031: restart groups (2PHSCHNG: OCT 40116, "SAVE TBASE6", OCT
 * 05024, OCT 13000; "DOES INHINT/RELINT"), */
void cm_dap2c_2phschng(void)
{
    tc_to(CM_DAP2C + 031u, TWO_PHSCHNG);
}

/* CM/DAP2C +035: and on to P62.2 (POSTJUMP). */
void cm_dap2c_after_2phschng(void)
{
    tc_to(CM_DAP2C + 035u, POSTJUMP); /* CADR P62.2 */
}

#endif
