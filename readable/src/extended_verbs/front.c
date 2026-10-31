/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc, part 1: the front end -- LM pp. 262-264, CM pp.
 * 236-238. The faithful C is c64-port/src/lanec_ev_front.c.
 *
 * GOEXTVB fans out to the verb's routine through LST2FAN, a table of one
 * TC per verb (TCF for the two that never come back). A routine that
 * shows things on the DSKY first calls TESTXACT, which claims the extended
 * verb display system: if EXTVBACT says another extended verb has it, or
 * FLAGWRD4 says a priority display (OC24100's bits) has the DSKY, the
 * operator error light goes on and PINBALL takes over again (ALM/END,
 * GOPIN). Else SETXTACT sets EXTVBACT (bits 3 and 5, OCT24), blanks the
 * DSKY but for the program and verb lights (NVSUB with -2) and returns to
 * the caller, through the return address it kept in MPAC +1. XACT0 does
 * the same with EXTVBACT +0. XACTALM is the operator error light and the
 * release of the display system (ENDEXT) together.
 *
 * All of it is in bank 43, at the same addresses in both missions. Each
 * call out of the file (NVSUB, FALTON, ENDEXT, POSTJUMP, the verbs'
 * routines) ends a function with Q and Z set, and the word the call
 * returns to is an entry of its own. */
#include "extended_verbs.h"
#include "../agc/names.h"

/* ---- The fan ---- */

/* One word of LST2FAN: a TC to the verb's routine, or a TCF. */
struct fan_word {
    word to;
    bool tcf;
};
#define TC(label)  {(word)K_##label, false}
#define TCF(label) {(word)K_##label, true}

/* LST2FAN -- LM pp. 262-263, CM pp. 236-237: VB40 to VB99, the verb less
 * 40 being the index. ALM_END is ALM/END, a spare verb. */
static const struct fan_word lst2fan[] = {
#if LUMINARY099
    TC(VBZERO),    /* VB40 ZERO (USED WITH NOUN 20 OR 72 ONLY) */
    TC(VBCOARK),   /* VB41 COARSE ALIGN (USED WITH NOUN 20 OR 72 ONLY) */
    TC(IMUFINEK),  /* VB42 FINE ALIGN IMU */
    TC(IMUATTCK),  /* VB43 LOAD IMU ATTITUDE ERROR METERS. */
    TC(RRDESEND),  /* VB44 TERMINATE CONTINUOUS DESIGNATE */
    TC(ALM_END),   /* VB45 SPARE */
    TC(ALM_END),   /* VB46 SPARE */
    TC(V47TXACT),  /* VB47 AGS INITIALIZATION */
    TC(DAPDISP),   /* VB48 LOAD A/P DATA */
    TCF(CREWMANU), /* VB49 START AUTOMATIC ATTITUDE MANEUVER */
    TC(GOLOADLV),  /* VB50 PLEASE PERFORM */
    TC(ALM_END),   /* VB51 SPARE */
    TC(GOLOADLV),  /* VB52 PLEASE MARK X -- RETICLE. */
    TC(GOLOADLV),  /* VB53 PLEASE MARK Y -- RETICLE. */
    TC(GOLOADLV),  /* VB54 PLEASE MARK X OR Y RETICLE */
    TC(ALINTIME),  /* VB55 ALIGN TIME */
    TC(TRMTRACK),  /* VB56 TERMINATE TRACKING -- P20 + P25 */
    TC(LRON),      /* VB57 PERMIT LANDING RADAR UPDATES */
    TC(LROFF),     /* VB58 INHIBIT LANDING RADAR UPDATES */
    TC(ALM_END),   /* VB59 SPARE */
    TC(LRPOS2K),   /* VB60 COMMAND LR TO POSITION 2. */
    TC(DAPATTER),  /* VB61 DISPLAY DAP ATTITUDE ERROR */
    TC(TOTATTER),  /* VB62 DISPLAY TOTAL ATTITUDE ERROR */
    TC(R04),       /* VB63 SAMPLE RADAR ONCE PER SECOND */
    TC(VB64),      /* VB64 CALCULATE, DISPLAY S-BAND ANT ANGLES */
    TC(SNUFFOUT),  /* VB65 DISABLE U,V JETS DURING DPS BURNS. */
    TC(ATTACHED),  /* VB66 ATTACHED MOVE THIS TO OTHER STATE */
    TC(V67),       /* VB67 W MATRIX MONITOR */
    TC(ALM_END),   /* VB68 SPARE */
    TC(VERB69),    /* VB69 FORCE A HARDWARE RESTART (VERB69 is this word) */
    TC(V70UPDAT),  /* VB70 UPDATE LIFTOFF TIME. */
    TC(V71UPDAT),  /* VB71 UNIVERSAL UPDATE -- BLOCK ADDRESS. */
    TC(V72UPDAT),  /* VB72 UNIVERSAL UPDATE -- SINGLE ADDRESS. */
    TC(V73UPDAT),  /* VB73 UPDATE AGC TIME (OCTAL). */
    TC(DNEDUMP),   /* VB74 INITIALIZE DOWN-TELEMETRY PROGRAM FOR ERASABLE DUMP. */
    TC(OUTSNUFF),  /* VB75 ENABLE U,V JETS DURING DPS BURNS. */
    TC(MINIMP),    /* VB76 MINIMUM IMPULSE MODE */
    TC(NOMINIMP),  /* VB77 RATE COMMAND MODE */
    TC(R77),       /* VB78 START LR SPURIOUS RETURN TEST */
    TC(R77END),    /* VB79 TERMINATE LR SPURIOUS RETURN TEST */
    TC(LEMVEC),    /* VB80 UPDATE LEM STATE VECTOR */
    TC(CSMVEC),    /* VB81 UPDATE CSM STATE VECTOR */
    TC(V82PERF),   /* VB82 REQUEST ORBIT PARAM DISPLAY (R30) */
    TC(V83PERF),   /* VB83 REQUEST REND PARAM DISPLAY (R31) */
    TC(ALM_END),   /* VB84 SPARE */
    TC(VERB85),    /* VB85 DISPLAY RR LOS AZ AND ELEV */
    TC(ALM_END),   /* VB86 SPARE */
    TC(ALM_END),   /* VB87 SPARE */
    TC(ALM_END),   /* VB88 SPARE */
    TC(V89PERF),   /* VB89 ALIGN XORZ LEM AXIS ALONG LOS (R63) */
    TC(V90PERF),   /* VB90 OUT OF PLANE RENDEZVOUS DISPLAY */
    TC(GOSHOSUM),  /* VB91 DISPLAY BANK SUM. */
    TC(SYSTEST),   /* VB92 OPERAT IMU PERFORMANCE TEST. */
    TC(WMATRXNG),  /* VB93 CLEAR RENDWFLG */
    TC(ALM_END),   /* VB94 SPARE */
    TC(UPDATOFF),  /* VB95 NO STATE VECTOR UPDATE ALLOWED */
    TC(VERB96),    /* VB96 INTERRUPT INTEGRATION AND GO TO P00 */
    TC(GOLOADLV),  /* VB97 PLEASE VERIFY ENGINE FAILURE */
    TC(ALM_END),   /* VB98 SPARE */
    TC(GOLOADLV),  /* VB99 PLEASE ENABLE ENGINE */
#else
    TC(VBZERO),    /* VB40 ZERO (USED WITH NOUN 20 ONLY). */
    TC(VBCOARK),   /* VB41 COARSE ALIGN (USED WITH NOUN 20 OR 91 ONLY) */
    TC(IMUFINEK),  /* VB42 FINE ALIGN IMU */
    TC(IMUATTCK),  /* VB43 LOAD IMU ATTITUDE ERROR METERS. */
    TC(SETSURF),   /* VB44 SET SURFACE FLAG */
    TC(RESTSRF),   /* VB45 RESET SURFACE FLAG */
    TC(STABLISH),  /* VB46 ESTABLISH G+C CONTROL. */
    TC(LMTOCMSV),  /* VB47 MOVE LM STATE VECTOR INTO CM */
    TC(DAPDISP),   /* VB48 LOAD A/P DATA. */
    TCF(CREWMANU), /* VB 49 START AUTOMATIC ATTITUDE MANEUVER */
    TC(GOLOADLV),  /* VB50 PLEASE PERFORM */
    TC(GOLOADLV),  /* VB51 PLEASE MARK */
    TC(V52),       /* VB52 SET OFFSET NO. FOR P22 */
    TC(GOLOADLV),  /* VB 53 PLEASE PERFORM COAS MARK */
    TC(GOTOR23),   /* VB54 PLEASE MARK (R-21-BACKUP) */
    TC(ALINTIME),  /* VB55 ALIGN TIME */
    TC(TRACKTRM),  /* VB56 TERMINATE TRACKING (P20 +P25) */
    TC(GOTOR21),   /* V57 START R21 REND TRACK SIGHT MARK ROUT */
    TC(ENATMA),    /* VB58 ENABLE AUTOMATIC ATTITUDE MANEUVER */
    TC(GOLOADLV),  /* VB59 PLEASE CALIBRATE */
    TC(V60),       /* VB60 SET CPHIX (N17) EQUAL TO CDU */
    TC(V61),       /* VB61 SELECT MODE I */
    TC(V62),       /* VB62 SELECT MODE II, ERROR WRT N22 */
    TC(V63),       /* VB63 SELECT MODE III, ERROR WRT N17 */
    TC(VB64),      /* VB64 CALCULATE,DISPLAY S-BAND ANT ANGLES */
    TC(CKOPTVB),   /* V 65 E OPTICAL VERIFICATION FOR PRELAUNC */
    TC(ATTACHED),  /* VB 66 ATTACHED. MOVE THIS TO OTHER STATE */
    TC(V67),       /* VB67 WMATRIX MONITOR */
    TC(STROKON),   /* VB68 CSM STROKE TEST ON. */
    TC(VERB69),    /* VB 69 CAUSE RESTART (VERB69 is this word) */
    TC(V70UPDAT),  /* VB70 UPDATE LIFTOFF TIME. */
    TC(V71UPDAT),  /* VB71 UNIVERSAL UPDATE - BLOCK ADDRESS. */
    TC(V72UPDAT),  /* VB72 UNIVERSAL UPDATE - SINGLE ADDRESS. */
    TC(V73UPDAT),  /* VB73 UPDATE AGC TIME (OCTAL). */
    TC(DNEDUMP),   /* VB74 INITIALIZE DOWN-TELEMETRY PROGRAM FOR ERASABLE DUMP. */
    TC(LFTFLGON),  /* VB75 SET LIFTOFF FLAG. */
    TC(SETPRFLG),  /* VB76 SET PREFERRED ATTITUDE FLAG */
    TC(RESETPRF),  /* VB77 RESET PREFERRED ATT. FLAG */
    TC(CHAZFOGC),  /* CHANGE GYROCOMPASS LAUNCH AZIMUTH V78 */
    TC(ALM_END),   /* V79 SPARE */
    TC(LEMVEC),    /* VB80 UPDATE LEM STATE VECTOR */
    TC(CSMVEC),    /* VB81 UPDATE CSM STATE VECTOR */
    TC(V82PERF),   /* VB82 REQUEST ORBIT PARAM DISPLAY (R30) */
    TC(V83PERF),   /* VB83 RANGE, RANGE RATE, +X AXIS (R31) */
    TC(ALM_END),   /* V84 SPARE */
    TC(V85PERF),   /* VB85 RANGE, RANGE RATE, SLOS (R32) */
    TC(V86PERF),   /* VB86 BACKUP MARK REJECT */
    TC(SETVHFLG),  /* VB87 SET VHF RANGE FLAG */
    TC(RESETVHF),  /* VB88 RESET VHF RANGE FLAG */
    TC(V89PERF),   /* V89-ALIGN X OR PRF CSM AXIS TO LOS (R63) */
    TC(V90PERF),   /* VB90-OUT OF PLAN PARAMETERS (R36) */
    TC(GOSHOSUM),  /* VB91 TEMP FOR HYBRID AND STG. */
    TC(SYSTEST),   /* VB92 OPERATE IMU PERFORMANCE TEST */
    TC(WMATRXNG),  /* VB93 CLEAR RENDWFLG */
    TC(VERB94),    /* VB94 DO R64 */
    TC(ALM_END),   /* VB95 SPARE */
    TCF(VERB96),   /* VB96 SET QUITFLAG TO STOP INTEGRATION */
    TC(GOLOADLV),  /* VB97 PLEASE PERFORM ENGINE-FAIL (R41) */
    TC(ALM_END),   /* VB98 SPARE */
    TC(GOLOADLV),  /* VB99 PLEASE ENABLE ENGINE */
#endif
};
#define FAN_WORDS (sizeof lst2fan / sizeof lst2fan[0])

/* GOEXTVB -- LM p. 262, CM p. 236.
 * INDEX MPAC; TC LST2FAN: the verb less 40 in MPAC picks the fan's word,
 * which goes on to the verb's routine. An index beyond the fan's 60 words
 * (PINBALL passes none) is not modelled, as in the faithful port. */
void goextvb(void)
{
    const struct fan_word *fan;

    if (MPAC(0) >= FAN_WORDS) not_modelled();
    tc_at(GOEXTVB + 1u);
    continue_at(LST2FAN + MPAC(0));
    fan = &lst2fan[MPAC(0)];
    /* VERB69's word is TC VERB69, a TC to itself: the AGC runs it until
     * the hardware's TC trap restarts the computer. The routine stops at
     * it, as the faithful port does. */
    if (fan->to == Z) return;
    if (!fan->tcf) tc_at(Z);
    continue_at(fan->to);
}

/* LST2FAN +n -- one word of the fan run on its own, at Z: where a verb's
 * routine that returns to its TC's Q goes on, the next verb's word (the
 * port's compiled copy of the CM's file enters at the words after V70UPDAT
 * to V73UPDAT's). */
void lst2fan_word(void)
{
    const struct fan_word *fan;

    if (Z < LST2FAN || Z >= LST2FAN + FAN_WORDS) not_modelled();
    fan = &lst2fan[Z - LST2FAN];
    if (!fan->tcf) tc_at(Z);
    continue_at(fan->to);
}

/* ---- Claiming the display system ---- */

static void setxtact_plus1(void);
static void setxtact_plus2(void);

/* TESTXACT -- LM p. 263, CM p. 237.
 * "ARE EXTENDED VERBS BUSY": they are if EXTVBACT is positive, and the
 * DSKY is if a priority display has it (FLAGWRD4 & OC24100); either way,
 * the operator error light (ALM/END). Else SETXTACT with OCT24. Each CCS
 * goes on at the word its outcome picks: a negative EXTVBACT skips the CA
 * with its DABS in A, a -0 skips the MASK too with +0 in A. The masked
 * word is never negative, but those two outcomes are followed anyway. */
void testxact(void)
{
    switch (ccs_at(ADDRESS_OF(EXTVBACT))) {
    case POSITIVE:
        tc_at(TESTXACT + 1u); /* "YES, TURN ON OPERATOR LIGHT" */
        alm_end();
        return;
    case IS_PLUS_ZERO: ca(ADDRESS_OF(FLAGWRD4)); /* fall through */
    case NEGATIVE: mask(OC24100);                /* fall through */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        tc_at(TESTXACT + 5u); /* "YES" */
        alm_end();
        return;
    case IS_PLUS_ZERO:
        set_A(OCT24); /* "SET BITS 3 AND 5" */
        setxtact();
        return;
    case NEGATIVE: setxtact(); return;
    case IS_MINUS_ZERO: setxtact_plus1(); return;
    }
}

/* SETXTACT -- LM p. 263, CM p. 237.
 * "SET FLAG TO SHOW EXT VERB DISPLAY SYSTEM BUSY" with A. An overflow in
 * A (the TS keeps it corrected) skips CA Q. */
void setxtact(void)
{
    if (ts(ADDRESS_OF(EXTVBACT)))
        setxtact_plus2();
    else
        setxtact_plus1();
}

/* SETXTACT +1: CA Q; TS MPAC +1, the caller's return, for SETXTACT +6. */
static void setxtact_plus1(void)
{
    ca(Q_ADDRESS);
    setxtact_plus2();
}

/* SETXTACT +2: keep A in MPAC +1 and NVSUB with -2, "BLANK EVERYTHING
 * EXCEPT MM AND VERB". An overflow in A skips CS TWO. */
static void setxtact_plus2(void)
{
    if (!ts(ADDRESS_OF(MPAC) + 1u)) set_A(negate(TWO));
    tc_at(SETXTACT + 4u);
    continue_at(NVSUB);
}

/* SETXTACT +5 -- LM p. 263, CM p. 237.
 * NVSUB returns here when the DSKY was busy: TC +1, on as if it was not. */
void setxtact_after_nvsub_busy(void)
{
    tc_at(SETXTACT + 5u);
    setxtact_after_nvsub();
}

/* SETXTACT +6 -- LM p. 263, CM p. 237.
 * TC MPAC +1: the AGC runs the word kept there, the caller's return, as
 * the instruction TC <return>. Any word but a TC into fixed memory is not
 * modelled, as in the faithful port. */
void setxtact_after_nvsub(void)
{
    word back = MPAC(1);

    if (back < 02000u || back > 07777u) not_modelled();
    tc_at(SETXTACT + 6u);
    tc_through(ADDRESS_OF(MPAC) + 1u);
}

/* XACTALM -- LM p. 263, CM p. 237.
 * "TURN ON OPERATOR ERROR LIGHT." (FALTON), then release the display
 * system. */
void xactalm(void)
{
    tc_at(XACTALM);
    continue_at(FALTON);
}

/* XACTALM +1 -- LM p. 263, CM p. 237.
 * "RELEASE MARK AND EXT. VERB DISPLAY SYS." (ENDEXT). */
void xactalm_after_falton(void)
{
    tc_at(XACTALM + 1u);
    continue_at(ENDEXT);
}

/* XACT0 -- LM p. 264, CM p. 238.
 * SETXTACT with EXTVBACT +0. */
void xact0(void)
{
    set_A(ZERO);
    tc_at(XACT0 + 1u); /* TC SETXTACT */
    setxtact();
}

/* ---- Back to PINBALL ---- */

/* ALM/END -- LM p. 264, CM p. 238.
 * "TURN ON OPERATOR ERROR LIGHT" (FALTON, which returns to GOPIN). */
void alm_end(void)
{
    tc_at(ALM_END);
    continue_at(FALTON);
}

/* GOPIN -- LM p. 264, CM p. 238.
 * TC POSTJUMP; CADR PINBRNCH: back to PINBALL in its own bank. */
void gopin(void)
{
    tc_at(GOPIN);
    continue_at(POSTJUMP);
}

#if LUMINARY099
/* CHKPOOH -- LM p. 264.
 * "CHECK FOR P00 OR P00-.": with MODREG +0 or -0, return to the caller
 * (TCQ, a TC Q); else the operator error light. */
void chkpooh(void)
{
    ca(ADDRESS_OF(MODREG));
    if (A_is_zero()) { /* EXTEND; BZF TCQ */
        continue_at(TCQ);
        return;
    }
    tc_at(CHKPOOH + 3u); /* TC ALM/END */
    alm_end();
}
#endif
