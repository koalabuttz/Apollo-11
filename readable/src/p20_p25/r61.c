/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 4 (Luminary099 pp. 517-520): R61 and R65.
 *
 * R61, "PREFERRED TRACKING ATTITUDE ROUTINE R61LEM": "TO COMPUTE THE
 * PREFERRED TRACKING ATTITUDE OF THE LM TO ENABLE RR TRACKING OF THE CSM
 * AND TO PERFORM THE MANEUVER TO THE PREFERRED ATTITUDE." While the track
 * flag is set: the line of sight to the CSM three seconds ahead (LPS20.1),
 * and from it, with the desired CDU angles (READCDUD), the final gimbal
 * angles that point the LM's +Z axis along it (VECPNT1). If the guidance
 * system has the attitude (G+N,AUTO) and the line of sight is within 15
 * degrees of +Z, those angles go straight to the DAP's desired CDUs;
 * further off, R60 maneuvers there (a priority display, with the minimum
 * deadband). Out of auto, BALLANGS shows the angles on the FDAI. "GENRET
 * USED TO SAVE Q FOR RETURN": R61 then returns to its caller.
 *
 * R65 is the same routine with the R61 flag clear: run by R22 and P25, it
 * does it again every 6 seconds, R65CNTR times, before it returns. Both go
 * to P20LEMWT (R65WAIT) when the track flag is not set.
 *
 * As in parts 1 to 3, each call out of the file leaves Z at the routine
 * and Q at the word after the TC, and a TC or a branch to a label the
 * faithful port keys a routine at stops there. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* R61LEM -- p. 518. The caller's return, as a CADR, to GENRET. */
void r61lem(void)
{
    tc_to(R61LEM, MAKECADR);
}

void r61lem_after_makecadr(void)
{
    if (ts(ADDRESS_OF(GENRET))) not_modelled(); /* (A as MAKECADR leaves it does not overflow) */
    tc_to(R61LEM + 2u, UPFLAG);                 /* "SET R61 FLAG" */
}

void r61lem_after_upflag(void)
{
    tc_to(R61LEM + 4u, R61C_L01);
}

/* R65LEM -- the same, with the R61 flag reset. */
void r65lem(void)
{
    tc_to(R65LEM, MAKECADR);
}

void r65lem_after_makecadr(void)
{
    if (ts(ADDRESS_OF(GENRET))) not_modelled();
    tc_to(R65LEM + 2u, DOWNFLAG); /* "RESET R61 FLAG" */
}

/* R61C+L01 -- "TRACKFLAG": "NOT SET", P20LEMWT (R65WAIT). Else the LOS
 * and the angles (R61C_L03_WORDS). */
void r61c_l01(void)
{
    set_A(TRACKBIT);
    mask_at(ADDRESS_OF(STATE) + 1u);
    if (A_is_zero()) { /* EXTEND; BZF R65WAIT */
        continue_at(R65WAIT);
        return;
    }
    tc_at(R61C_L03);
    intpret();
}

/* R61C+L03 +1 -- the track axis is +Z; R61LEM1: "EXTRAPOLATE FORWARD TO
 * CENTER SIX SECOND PERIOD." by 3 seconds, "LOS DETERMINATION + VEH
 * ATTITUDE" (LPS20.1); then the angles: "GET DESIRED CDU'S FOR VECPNT1"
 * (READCDUD), and VECPNT1 "COMPUTES FINAL ANGLES FROM PRESENT CDUDS":
 * "STORE FINAL ANGLES -- CPHI, CTHETA, CPSI". */
static const word r61c_l03_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(HIUNITZ),
    STORE(SCAXIS), /* "TRACK AXIS UNIT VECTOR" */
    /* R61LEM1 */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(3SECONDS),
    STCALL(TDEC1),
    ADDRESS(LPS20_1),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RRTARGET),
    STORE(POINTVSM),
    ORDERS(RTB, CALL),
    ADDRESS(READCDUD),
    ADDRESS(VECPNT1),
    STORE(CPHI),
    ORDER(EXIT),
};

/* R61LEM1 +015 -- restart group 2 at P20LEM1. */
void r61lem1_after_exit(void)
{
    tc_to(R61LEM1 + 015u, PHASCHNG); /* OCT 04022 */
}

/* R61LEM1 +017 -- "IS TRACK FLAG SET": not, R65WAIT. Else "CHECK FOR AUTO
 * MODE" (G+N,AUTO). */
void r61lem1_after_phaschng(void)
{
    set_A(TRACKBIT);
    mask_at(ADDRESS_OF(FLAGWRD1));
    if (A_is_zero()) { /* EXTEND; BZF R65WAIT */
        continue_at(R65WAIT);
        return;
    }
    tc_to(R61LEM1 + 023u, BANKCALL); /* CADR G+N,AUTO */
}

/* R61LEM1 +025 -- G+N,AUTO's answer in A: positive, "NOT IN AUTO", the
 * angles on the FDAI (R61C+L04); +0, in auto (R61LEM1_AUTO_WORDS). Not
 * modelled: a negative answer, which runs interpretive words as basic
 * code. */
void r61lem1_after_auto_check(void)
{
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        tc_at(R61LEM1 + 026u);
        tc_to(R61C_L04, BANKCALL); /* "TO CONVERT ANGLES TO FDAI": CADR BALLANGS */
        return;
    case IS_PLUS_ZERO: continue_at(R61LEM1 + 027u); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

void r61lem1_in_auto(void)
{
    tc_at(R61LEM1 + 027u);
    intpret();
}

/* R61LEM1 +030 -- the LOS in navigation base axes (CDU*SMNB): "GET PHI --
 * ARCCOS OF Z-COMPONENT OF LOS", against cos 15 degrees. R61LEM2: "BRANCH
 * -- PHI > 15 DEGREES", R61C+L05. */
static const word r61lem1_auto_words[] = {
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(RRTARGET),
    ADDRESS(CDU_STARSMNB),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_AT(MPAC, 5),
    ADDRESS_PLUS_ONE(COS15DEG),
    /* R61LEM2 */ ORDERS(BMN, EXIT),
    ADDRESS(R61CPL05),
};

/* R61LEM2 +2 -- within 15 degrees: the final angles straight to the
 * desired CDUs (CDUXD, CDUYD, CDUZD in E bank 6), with interrupts held,
 * then E bank 7 again and R61C+L06. (A, an EBANK constant or a word from
 * memory, does not overflow: no TS skips.) */
void r61lem2_after_exit(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    inhint();
    dca(ADDRESS_OF(CPHI)); /* EXTEND; DCA CPHI */
    dxch(ADDRESS_OF(CDUXD));
    ca(ADDRESS_OF(CPSI));
    ts(ADDRESS_OF(CDUZD));
    relint();
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    tc_to(R61LEM2 + 015u, R61C_L06);
}

/* R61C+L05 -- "PHI GRE 10DEG": EXIT, and R60 maneuvers there. */
static const word r61c_l05_words[] = {
    ORDER(EXIT),
};

/* R61C+L05 +1 -- ZATTEROR, and SETMINDB: "REDUCE ATTITUDE ERROR". */
void r61c_l05_after_exit(void)
{
    inhint();
    tc_to(R61C_L05 + 2u, IBNKCALL); /* FCADR ZATTEROR */
}

void r61c_l05_after_zatteror(void)
{
    tc_to(R61C_L05 + 4u, IBNKCALL); /* FCADR SETMINDB */
}

/* R61C+L05 +6 -- 3AXISFLG reset; "SET PRIORITY DISPLAY FLAG"; R60. */
void r61c_l05_after_setmindb(void)
{
    tc_to(R61C_L05 + 6u, DOWNFLAG); /* ADRES 3AXISFLG */
}

void r61c_l05_after_downflag(void)
{
    tc_to(R61C_L05 + 010u, UPFLAG); /* ADRES PDSPFLAG */
}

void r61c_l05_after_upflag(void)
{
    tc_to(R61C_L05 + 012u, BANKCALL); /* CADR R60LEM */
}

/* R61C+L05 +014 -- the astronaut's deadband back (RESTORDB). */
void r61c_l05_after_r60lem(void)
{
    inhint();
    tc_to(R61C_L05 + 015u, IBNKCALL); /* FCADR RESTORDB */
}

/* R61C+L05 +017 -- restart group 2 at P20LEM1. */
void r61c_l05_after_restordb(void)
{
    tc_to(R61C_L05 + 017u, PHASCHNG); /* OCT 04022 */
}

/* R61C+L05 +021 -- "RESET PRIORITY DISPLAY FLAG". */
void r61c_l05_after_phaschng(void)
{
    tc_to(R61C_L05 + 021u, DOWNFLAG); /* ADRES PDSPFLAG */
}

/* R61C+L06 -- the R61 flag set: R61C+L4, return. Else (R65) count R65CNTR
 * down: at +0, "R65CNTR = 0 - EXIT ROUTINE"; else again in 6 seconds,
 * R61C+L2 by TWIDDLE. (After MASK, A is +0 or the flag's bit.) */
void r61c_l06(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(R61FLBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_to(R61C_L06 + 3u, R61C_L4); return;
    case IS_PLUS_ZERO: continue_at(R61C_L06 + 4u); return;
    case NEGATIVE: tc_to(R61C_L06 + 5u, R61C_L06 + 7u); return; /* TC +2 */
    case IS_MINUS_ZERO: continue_at(R61C_L06 + 6u); return;
    }
}

/* R61C+L06 +4 -- CCS R65CNTR. */
void r61c_l06_r65(void)
{
    switch (ccs_at(ADDRESS_OF(R65CNTR))) {
    case POSITIVE: tc_to(R61C_L06 + 5u, R61C_L06 + 7u); return; /* TC +2 */
    case IS_PLUS_ZERO: continue_at(R61C_L06 + 6u); return;
    case NEGATIVE: continue_at(R61C_L06 + 7u); return;
    case IS_MINUS_ZERO:
        set_A(SIX_SEC);
        tc_to(R61C_L06 + 011u, TWIDDLE); /* ADRES R61C+L2 */
        return;
    }
}

/* R61C+L06 +6 -- "R65CNTR = 0 - EXIT ROUTINE". */
void r61c_l06_count_out(void)
{
    tc_to(R61C_L06 + 6u, R61C_L4);
}

/* R61C+L06 +7 -- the count back to R65CNTR, and again in 6 seconds. */
void r61c_l06_again(void)
{
    if (!ts(ADDRESS_OF(R65CNTR))) set_A(SIX_SEC);
    tc_to(R61C_L06 + 011u, TWIDDLE); /* ADRES R61C+L2 */
}

void r61c_l06_after_twiddle(void)
{
    tc_to(R61C_L06 + 013u, ENDOFJOB);
}

/* R61C+L2 -- the task TWIDDLE starts: R61C+L01 as a job. */
void r61c_l2(void)
{
    set_A(PRIO26);
    tc_to(R61C_L2 + 1u, FINDVAC); /* 2CADR R61C+L01 */
}

void r61c_l2_after_findvac(void)
{
    tc_to(R61C_L2 + 4u, TASKOVER);
}

/* R61C+L04 +2 -- after BALLANGS. */
void r61c_l04_after_ballangs(void)
{
    tc_to(R61C_L04 + 2u, R61C_L06);
}

/* R61C+L4 -- "EXIT R61": to the CADR in GENRET. */
void r61c_l4(void)
{
    ca(ADDRESS_OF(GENRET)); /* CAE GENRET */
    continue_at(BANKJUMP);  /* TCF BANKJUMP */
}

/* R65WAIT -- P20LEMWT. */
void r65wait(void)
{
    tc_to(R65WAIT, POSTJUMP); /* CADR P20LEMWT */
}

/* READCDUD -- p. 520, R61LEM1's RTB: "READS DESIRED CDU'S AND STORES IN
 * MPAC TP EXITS WITH MODE SET TO TP", with E bank 6 selected for them and
 * interrupts held. (A, a word from memory, does not overflow: no TS
 * skips.) */
void readcdud(void)
{
    inhint();
    set_A(EBANK6);
    xch(EBANK_ADDRESS);
    ts(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(CDUXD));
    ts(ADDRESS_OF(MPAC));
    dca(ADDRESS_OF(CDUYD)); /* EXTEND; DCA CDUYD */
    dxch(ADDRESS_OF(MPAC) + 1u);
    ca(ADDRESS_OF(RUPTREG1));
    ts(EBANK_ADDRESS);
    relint();
    continue_at(TMODE); /* TCF TMODE */
}

/* ---- The interpretive code ---- */

const struct interpretive_code R61C_L03_CODE = {023u, R61C_L03 + 1u, COUNT(r61c_l03_words),
                                                r61c_l03_words};
const struct interpretive_code R61LEM1_AUTO_CODE = {023u, R61LEM1 + 030u, COUNT(r61lem1_auto_words),
                                                    r61lem1_auto_words};
const struct interpretive_code R61C_L05_CODE = {023u, R61C_L05, COUNT(r61c_l05_words),
                                                r61c_l05_words};

#endif
