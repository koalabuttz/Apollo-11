/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc, part 2: the short verbs -- LM pp. 266-299, CM pp.
 * 243-267. The faithful C is c64-port/src/lanec_ev_short.c.
 *
 * Most extended verbs are short: they set or clear a flag and go back to
 * PINBALL (GOPIN), or claim the display system (TESTXACT) and start a job
 * (FINDVAC, NOVAC) that does the work, or check something and hand over to
 * a routine in another bank (POSTJUMP). Here are those, scattered through
 * the listing, and SHOWSUM, verb 91's display of the bank sums.
 *
 * Every TC and TCF out of a routine ends its C function with Q and Z set
 * (tc() below, or continue_at()), and the word a call returns to is an
 * entry of its own. The binding goes on from one entry of this part to the
 * next while Z is one, as the faithful module does; a call to another part
 * (TESTXACT, GOPIN, ALM/END and the LM's CHKPOOH are part 1's) or out of
 * the file stops there. A routine that falls into the next label calls it.
 * All of it is basic code in bank 43. */
#include "extended_verbs.h"
#include "../agc/names.h"

/* TC at fixed address at to routine: Q gets the return, at +1. */
static void tc(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- The flag verbs ---- */

/* Each is TC UPFLAG or TC DOWNFLAG, ADRES <flag> (the flag's number, which
 * the flag routine reads at Q and steps over), and back to PINBALL, by TC
 * or TCF GOPIN. */

#if LUMINARY099
/* DAPATTER -- LM p. 266.
 * "VERB 61, DISPLAY DAP ATTITUDE ERRORS ON FDAI ATTITUDE ERROR NEEDLES":
 * NEEDLFLG down. */
void dapatter(void)
{
    tc(DAPATTER, DOWNFLAG); /* ADRES NEEDLFLG */
}

/* DAPATTER +2 -- LM p. 266. */
void dapatter_after_downflag(void)
{
    tc(DAPATTER + 2u, GOPIN);
}

/* TOTATTER -- LM p. 266.
 * Verb 62, "DISPLAY TOTAL ATTITUDE ERRORS ON FDAI ATTITUDE ERROR NEEDLES":
 * NEEDLFLG up. */
void totatter(void)
{
    tc(TOTATTER, UPFLAG); /* ADRES NEEDLFLG */
}

/* TOTATTER +2 -- LM p. 266. */
void totatter_after_upflag(void)
{
    tc(TOTATTER + 2u, GOPIN);
}

/* LRON -- LM pp. 271-272.
 * "PERMIT INCORPORATION OF LR DATA" (verb 57): LRINH up. */
void lron(void)
{
    tc(LRON, UPFLAG); /* ADRES LRINH */
}

/* LRON +2 -- LM p. 272. */
void lron_after_upflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}

/* LROFF -- LM p. 272.
 * "INHIBIT INCORPORATION OF LR DATA" (verb 58): LRINH down. */
void lroff(void)
{
    tc(LROFF, DOWNFLAG); /* ADRES LRINH */
}

/* LROFF +2 -- LM p. 272. */
void lroff_after_downflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}
#endif

/* LEMVEC -- LM p. 288, CM p. 260.
 * Verb 80, update the LM's state vector: "VEHUPFLG DOWN INDICATES LEM".
 * The LM goes on to NOUPDOWN, which allows the update. */
void lemvec(void)
{
    tc(LEMVEC, DOWNFLAG); /* ADRES VEHUPFLG */
}

/* LEMVEC +2 -- LM p. 288, CM p. 260. */
void lemvec_after_downflag(void)
{
#if LUMINARY099
    tc(LEMVEC + 2u, NOUPDOWN);
#else
    continue_at(GOPIN); /* TCF GOPIN */
#endif
}

/* CSMVEC -- LM p. 289, CM p. 260.
 * Verb 81, update the CSM's state vector: VEHUPFLG up. The LM's UPFLAG
 * returns to NOUPDOWN, the next word. */
void csmvec(void)
{
    tc(CSMVEC, UPFLAG); /* ADRES VEHUPFLG */
}

#if LUMINARY099
/* NOUPDOWN -- LM p. 289.
 * NOUPFLAG down: state vector updates allowed. */
void noupdown(void)
{
    tc(NOUPDOWN, DOWNFLAG); /* ADRES NOUPFLAG */
}

/* NOUPDOWN +2 -- LM p. 289. */
void noupdown_after_downflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}

/* UPDATOFF -- LM p. 289.
 * Verb 95, "INHIBIT STATE VECTOR UPDATES BY INCORP": NOUPFLAG up. */
void updatoff(void)
{
    tc(UPDATOFF, UPFLAG); /* ADRES NOUPFLAG */
}

/* UPDATOFF +2 -- LM p. 289. */
void updatoff_after_upflag(void)
{
    tc(UPDATOFF + 2u, GOPIN);
}

/* SNUFFOUT -- LM p. 298.
 * Verb 65, "DISABLE U,V JETS DURING DPS BURNS": SNUFFER up. */
void snuffout(void)
{
    tc(SNUFFOUT, UPFLAG); /* ADRES SNUFFER */
}

/* SNUFFOUT +2 -- LM p. 298. */
void snuffout_after_upflag(void)
{
    tc(SNUFFOUT + 2u, GOPIN);
}

/* OUTSNUFF -- LM p. 298.
 * Verb 75, "ENABLE U,V JETS DURING DPS BURNS": SNUFFER down. */
void outsnuff(void)
{
    tc(OUTSNUFF, DOWNFLAG); /* ADRES SNUFFER */
}

/* OUTSNUFF +2 -- LM p. 298. */
void outsnuff_after_downflag(void)
{
    tc(OUTSNUFF + 2u, GOPIN);
}

/* WMATRXNG -- LM p. 290.
 * Verb 93, clear RENDWFLG: with interrupts inhibited, FLAGWRD5 and'ed with
 * RENDWBIT's complement. The TS cannot overflow: A holds a masked word. */
void wmatrxng(void)
{
    inhint();
    set_A(negate(RENDWBIT)); /* CS RENDWBIT */
    mask_at(ADDRESS_OF(FLAGWRD5));
    ts(ADDRESS_OF(FLAGWRD5));
    tc(WMATRXNG + 4u, GOPIN);
}

/* MINIMP -- LM p. 284.
 * Verb 76, "MINIMUM IMPULSE MODE": with interrupts inhibited, the PULSES
 * bit of DAPBOOLS on ("PULSES = 1 INDICATES MIN IMP MODE"). CS, MASK and
 * ADS add the bit only where it is off. */
void minimp(void)
{
    inhint();
    cs(ADDRESS_OF(DAPBOOLS));
    mask(PULSES);
    ads(ADDRESS_OF(DAPBOOLS));
    continue_at(GOPIN); /* TCF GOPIN: "RETURN VIA PINBRNCH" */
}

/* NOMINIMP -- LM p. 285.
 * Verb 77, "RATE COMMAND MODE": the PULSES bit off, then ZATTEROR (by
 * IBNKCALL, in its own bank) zeroes the attitude errors. */
void nominimp(void)
{
    inhint();
    set_A(negate(PULSES)); /* CS PULSES */
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(DAPBOOLS));
    tc(NOMINIMP + 4u, IBNKCALL); /* CADR ZATTEROR */
}

/* NOMINIMP +6 -- LM p. 285. */
void nominimp_after_ibnkcall(void)
{
    tc(NOMINIMP + 6u, GOPIN);
}

/* VERB96 -- LM p. 298.
 * "QUITFLAG WILL CAUSE INTEGRATION TO EXIT AT NEXT TIMESTEP", and P00. */
void verb96(void)
{
    tc(VERB96, UPFLAG); /* ADRES QUITFLAG */
}

/* VERB96 +2 -- LM p. 298.
 * Verb 37 with A +0: "GO TO P00". */
void verb96_after_upflag(void)
{
    set_A(ZERO);
    tc(VERB96 + 3u, POSTJUMP); /* CADR V37 */
}
#else
/* V61 -- CM p. 244.
 * Verb 61, select mode I: NEEDLFLG down, "PHASE PLANE A/P FOLLOWING ERROR
 * DISPLAYED". */
void v61(void)
{
    tc(V61, DOWNFLAG); /* ADRES NEEDLFLG */
}

/* V61 +2 -- CM p. 244. */
void v61_after_downflag(void)
{
    tc(V61 + 2u, GOPIN);
}

/* V62 -- CM p. 244.
 * Verb 62, select mode II: NEEDLFLG up, "TOTAL ATTITUDE ERROR DISPLAYED",
 * and N22ORN17 up, "COMPUTE TOTAL ATTITUDE ERROR WRT N22". */
void v62(void)
{
    tc(V62, UPFLAG); /* ADRES NEEDLFLG */
}

/* V62 +2 -- CM p. 244. */
void v62_after_upflag(void)
{
    tc(V62 + 2u, UPFLAG); /* ADRES N22ORN17 */
}

/* V62 +4 -- CM p. 244. */
void v62_after_second_upflag(void)
{
    tc(V62 + 4u, GOPIN);
}

/* V63 -- CM pp. 244-245.
 * Verb 63, select mode III: NEEDLFLG up and N22ORN17 down, "COMPUTE TOTAL
 * ASTRONAUT ATTITUDE ERROR" (N17). */
void v63(void)
{
    tc(V63, UPFLAG); /* ADRES NEEDLFLG */
}

/* V63 +2 -- CM pp. 244-245. */
void v63_after_upflag(void)
{
    tc(V63 + 2u, DOWNFLAG); /* ADRES N22ORN17 */
}

/* V63 +4 -- CM p. 245. */
void v63_after_downflag(void)
{
    tc(V63 + 4u, GOPIN);
}

/* ENATMA -- CM p. 250.
 * Verb 58, enable automatic attitude maneuvers: "RESET STIKFLAG". */
void enatma(void)
{
    tc(ENATMA, DOWNFLAG); /* ADRES STIKFLAG */
}

/* ENATMA +2 -- CM p. 250. */
void enatma_after_downflag(void)
{
    tc(ENATMA + 2u, GOPIN);
}

/* CSMVEC +2 -- CM p. 260. */
void csmvec_after_upflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}

/* LFTFLGON -- CM p. 260.
 * Verb 75, "SET LIFTOFF FLAG BIT": GRRBKFLG up. */
void lftflgon(void)
{
    tc(LFTFLGON, UPFLAG); /* ADRES GRRBKFLG */
}

/* LFTFLGON +2 -- CM p. 260. */
void lftflgon_after_upflag(void)
{
    tc(LFTFLGON + 2u, GOPIN);
}

/* WMATRXNG -- CM pp. 262-263.
 * Verb 93: "RESET RENDWFLG" and "RESET ORBWFLAG". */
void wmatrxng(void)
{
    tc(WMATRXNG, DOWNFLAG); /* ADRES RENDWFLG */
}

/* WMATRXNG +2 -- CM p. 263. */
void wmatrxng_after_downflag(void)
{
    tc(WMATRXNG + 2u, DOWNFLAG); /* ADRES ORBWFLAG */
}

/* WMATRXNG +4 -- CM p. 263. */
void wmatrxng_after_second_downflag(void)
{
    tc(WMATRXNG + 4u, GOPIN);
}

/* SETPRFLG -- CM p. 263.
 * Verb 76, "SET PREFERRED ATTITUDE FLAG": PRFTRKAT up. */
void setprflg(void)
{
    tc(SETPRFLG, UPFLAG); /* ADRES PRFTRKAT */
}

/* SETPRFLG +2 -- CM p. 263. */
void setprflg_after_upflag(void)
{
    tc(SETPRFLG + 2u, GOPIN);
}

/* RESETPRF -- CM p. 263.
 * Verb 77, reset the preferred attitude flag: PRFTRKAT down. */
void resetprf(void)
{
    tc(RESETPRF, DOWNFLAG); /* ADRES PRFTRKAT */
}

/* RESETPRF +2 -- CM p. 263. */
void resetprf_after_downflag(void)
{
    tc(RESETPRF + 2u, GOPIN);
}

/* VERB96 -- CM p. 266.
 * "QUITFLAG WILL CAUSE INTEGRATION TO EXIT AT NEXT TIMESTEP", V96ONFLG
 * up, and P00. */
void verb96(void)
{
    tc(VERB96, UPFLAG); /* ADRES QUITFLAG */
}

/* VERB96 +2 -- CM p. 266. */
void verb96_after_upflag(void)
{
    tc(VERB96 + 2u, UPFLAG); /* ADRES V96ONFLG */
}

/* VERB96 +4 -- CM p. 266.
 * Verb 37 with A +0: "GO TO P00". */
void verb96_after_second_upflag(void)
{
    set_A(ZERO);
    tc(VERB96 + 5u, POSTJUMP); /* CADR V37 */
}

/* SETSURF -- CM p. 267.
 * Verb 44, set the surface flag: SURFFLAG up. */
void setsurf(void)
{
    tc(SETSURF, UPFLAG); /* ADRES SURFFLAG */
}

/* SETSURF +2 -- CM p. 267. */
void setsurf_after_upflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}

/* RESTSRF -- CM p. 267.
 * Verb 45, reset the surface flag: SURFFLAG down. */
void restsrf(void)
{
    tc(RESTSRF, DOWNFLAG); /* ADRES SURFFLAG */
}

/* RESTSRF +2 -- CM p. 267. */
void restsrf_after_downflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}
#endif

/* ---- The job starters ---- */

/* Each claims the display system (TESTXACT), most only in P00 or P00-
 * (CHKPOOH, the operator error light in any other program), then starts a
 * job: CAF PRIOn, TC FINDVAC or NOVAC with the job's 2CADR after it, and
 * ends its own job (ENDOFJOB), PINBALL's. */

#if LUMINARY099
/* V47TXACT -- LM p. 274.
 * Verb 47, "AGS INITIALIZATION -- R47": AGSINIT at priority 4. */
void v47txact(void)
{
    tc(V47TXACT, TESTXACT); /* "NO OTHER EXTVERB." */
}

/* V47TXACT +1 -- LM p. 274. */
void v47txact_after_testxact(void)
{
    set_A(PRIO4);
    tc(V47TXACT + 2u, FINDVAC); /* 2CADR AGSINIT */
}

/* V47TXACT +5 -- LM p. 274. */
void v47txact_after_findvac(void)
{
    tc(V47TXACT + 5u, ENDOFJOB);
}
#endif

/* VB64 -- LM p. 280, CM p. 250.
 * Verb 64, "CALC., DISPLAY S-BAND ANTENNA ANGLES" (SBANDANT), in P00
 * only. */
void vb64(void)
{
    tc(VB64, CHKPOOH); /* "DEMAND PROGRAM 00." */
}

/* VB64 +1 -- LM p. 280, CM p. 250. */
void vb64_after_chkpooh(void)
{
    tc(VB64 + 1u, TESTXACT); /* "IF DISPLAY SYS. NOT BUSY MAKE IT BUSY." */
}

/* VB64 +2 -- LM p. 280, CM p. 250.
 * SBANDANT at priority 4; the CM's with interrupts inhibited. */
void vb64_after_testxact(void)
{
#if LUMINARY099
    set_A(PRIO4);
    tc(VB64 + 3u, FINDVAC); /* 2CADR SBANDANT */
#else
    inhint();
    set_A(PRIO4);
    tc(VB64 + 4u, FINDVAC); /* 2CADR SBANDANT */
#endif
}

/* VB64 +6 (LM) or +7 (CM) -- LM p. 280, CM p. 250. */
void vb64_after_findvac(void)
{
#if LUMINARY099
    tc(VB64 + 6u, ENDOFJOB);
#else
    tc(VB64 + 7u, ENDOFJOB);
#endif
}

/* V83PERF -- LM p. 283, CM p. 255.
 * Verb 83, the rendezvous parameter display (R31). The LM starts R31CALL
 * as a task in 20 ms (WAITLIST, BIT2 centiseconds); the CM sets the R31
 * flag (bit 4 of FLAGWRD9), which V85PERF clears for R34, and starts it
 * as a job at priority 5 (NOVAC). */
void v83perf(void)
{
    tc(V83PERF, TESTXACT);
}

#if LUMINARY099
/* V83PERF +1 -- LM p. 283. */
void v83perf_after_testxact(void)
{
    set_A(BIT2);
    tc(V83PERF + 2u, WAITLIST); /* 2CADR R31CALL */
}

/* V83PERF +5 -- LM p. 283. */
void v83perf_after_waitlist(void)
{
    tc(V83PERF + 5u, ENDOFJOB);
}
#else
/* V83PERF +1 -- CM p. 255.
 * "SET R31 FLAG-BIT 4 FLAGWRD9", with interrupts inhibited: CS, MASK and
 * ADS add the bit only where it is off. */
void v83perf_after_testxact(void)
{
    inhint();
    cs(ADDRESS_OF(FLAGWRD9));
    mask(R31FLBIT);
    ads(ADDRESS_OF(FLAGWRD9));
    v83perf_plus5();
}

/* V83PERF +5 -- CM p. 255.
 * R31CALL at priority 5; V85PERF comes here too. */
void v83perf_plus5(void)
{
    set_A(PRIO5);
    tc(V83PERF + 6u, NOVAC); /* 2CADR R31CALL */
}

/* V83PERF +9 -- CM p. 255. */
void v83perf_after_novac(void)
{
    tc(V83PERF + 9u, ENDOFJOB);
}

/* V85PERF -- CM p. 256.
 * Verb 85, range, range rate and the line of sight (R32): "RESET R31 FLAG
 * TO INDICATE R34", then R31CALL as verb 83 starts it. */
void v85perf(void)
{
    tc(V85PERF, TESTXACT);
}

/* V85PERF +1 -- CM p. 256. */
void v85perf_after_testxact(void)
{
    inhint();
    set_A(negate(R31FLBIT)); /* CS R31FLBIT */
    mask_at(ADDRESS_OF(FLAGWRD9));
    ts(ADDRESS_OF(FLAGWRD9));
    tc(V85PERF + 5u, V83PERF + 5u);
}
#endif

/* V89PERF -- LM p. 284, CM p. 262.
 * Verb 89, the rendezvous final attitude routine (R63), in P00 only:
 * V89CALL at priority 10, the CM's with interrupts inhibited. */
void v89perf(void)
{
    tc(V89PERF, CHKPOOH); /* "DEMAND P00" */
}

/* V89PERF +1 -- LM p. 284, CM p. 262. */
void v89perf_after_chkpooh(void)
{
    tc(V89PERF + 1u, TESTXACT);
}

/* V89PERF +2 -- LM p. 284, CM p. 262. */
void v89perf_after_testxact(void)
{
#if LUMINARY099
    set_A(PRIO10);
    tc(V89PERF + 3u, FINDVAC); /* 2CADR V89CALL */
#else
    inhint();
    set_A(PRIO10);
    tc(V89PERF + 4u, FINDVAC); /* 2CADR V89CALL */
#endif
}

/* V89PERF +6 (LM) or +7 (CM) -- LM p. 284, CM p. 262. */
void v89perf_after_findvac(void)
{
#if LUMINARY099
    tc(V89PERF + 6u, ENDOFJOB);
#else
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
#endif
}

/* V90PERF -- LM p. 284, CM p. 266.
 * Verb 90, the rendezvous out-of-plane display (R36), at priority 7. */
void v90perf(void)
{
    tc(V90PERF, TESTXACT);
}

/* V90PERF +1 -- LM p. 284, CM p. 266. */
void v90perf_after_testxact(void)
{
    set_A(PRIO7);              /* "R36,V90" */
    tc(V90PERF + 2u, FINDVAC); /* 2CADR R36 */
}

/* V90PERF +5 -- LM p. 284, CM p. 266. */
void v90perf_after_findvac(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* CREWMANU -- LM p. 286, CM p. 252.
 * Verb 49, "START AUTOMATIC ATTITUDE MANEUVER" (R62DISP, priority 10), in
 * P00 only. */
void crewmanu(void)
{
    tc(CREWMANU, CHKPOOH); /* "DEMAND P00" */
}

/* CREWMANU +1 -- LM p. 286, CM p. 252. */
void crewmanu_after_chkpooh(void)
{
    tc(CREWMANU + 1u, TESTXACT);
}

/* CREWMANU +2 -- LM p. 286, CM p. 252. */
void crewmanu_after_testxact(void)
{
    set_A(PRIO10);
    tc(CREWMANU + 3u, FINDVAC); /* 2CADR R62DISP */
}

/* CREWMANU +6 -- LM p. 286, CM p. 252. */
void crewmanu_after_findvac(void)
{
    tc(CREWMANU + 6u, ENDOFJOB);
}

/* SYSTEST -- LM p. 290, CM p. 247.
 * Verb 92, "OPERATE IMU PERFORMANCE TEST" (REDO), in P00 only. The CM
 * first gives the NODO alarm 1521 (POODOO) if P11 has set NODOP01, and
 * turns its autopilot off (EXDAPOFF). */
void systest(void)
{
    tc(SYSTEST, CHKPOOH); /* "DEMAND P00" */
}

#if LUMINARY099
/* SYSTEST +1 -- LM p. 290. */
void systest_after_chkpooh(void)
{
    tc(SYSTEST + 1u, TESTXACT);
}

/* SYSTEST +2 -- LM p. 290. */
void systest_after_testxact(void)
{
    set_A(PRIO22);
    tc(SYSTEST + 3u, FINDVAC); /* 2CADR REDO */
}

/* SYSTEST +6 -- LM p. 290. */
void systest_after_findvac(void)
{
    tc(SYSTEST + 6u, ENDOFJOB);
}
#else
/* SYSTEST +1 -- CM p. 247.
 * "IS NODOP01 FLAGBIT ON? (SET BY P11)": if not, V92CONT; else "SEND NODO
 * ALARM FOR P07" (POODOO, OCT 1521). */
void systest_after_chkpooh(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(NOP01BIT);
    if (A_is_zero()) { /* EXTEND; BZF V92CONT */
        continue_at(V92CONT);
        return;
    }
    tc(SYSTEST + 5u, POODOO); /* OCT 1521 */
}

/* V92CONT -- CM p. 247.
 * "TURN DAP OFF IF IT'S ON" (EXDAPOFF, which returns by TC Q). */
void v92cont(void)
{
    tc(V92CONT, EXDAPOFF);
}

/* V92CONT +1 -- CM p. 247.
 * REDO at priority 20. */
void v92cont_after_exdapoff(void)
{
    set_A(PRIO20);
    tc(V92CONT + 2u, FINDVAC); /* 2CADR REDO */
}

/* V92CONT +5 -- CM p. 247. */
void v92cont_after_findvac(void)
{
    tc(V92CONT + 5u, GOPIN);
}

/* CKOPTVB -- CM p. 247.
 * Verb 65, optical verification for prelaunch: only while P02 runs ("I
 * WONDER IF PRELAUNCH IS RUNNING"). CHECKMM returns to the word after
 * MM 02 if not, past it if so. */
void ckoptvb(void)
{
    tc(CKOPTVB, CHECKMM); /* MM 02 */
}

/* CKOPTVB +2 -- CM p. 247: "NOT RUNNING OPERATOR ERROR". */
void ckoptvb_after_checkmm02_no(void)
{
    tc(CKOPTVB + 2u, ALM_END);
}

/* CKOPTVB +3 -- CM p. 247.
 * COMPVER, "STANDARD LEADIN TO GCOMPVER", at priority 16. */
void ckoptvb_after_checkmm02_yes(void)
{
    inhint();
    set_A(PRIO16);
    tc(CKOPTVB + 5u, FINDVAC); /* 2CADR COMPVER */
}

/* CKOPTVB +8 -- CM p. 247. */
void ckoptvb_after_findvac(void)
{
    tc(CKOPTVB + 8u, GOPIN);
}

/* CHAZFOGC -- CM p. 248.
 * Verb 78, change the gyrocompass launch azimuth: only in P02 ("IS IT
 * PRELAUNCH"), AZMTHCG1 at priority 16, and a phase change (PHASCHNG,
 * OCT 00174). */
void chazfogc(void)
{
    tc(CHAZFOGC, CHECKMM); /* MM 02 */
}

/* CHAZFOGC +2 -- CM p. 248: "NO - OPERATOR ERROR". */
void chazfogc_after_checkmm02_no(void)
{
    tc(CHAZFOGC + 2u, ALM_END);
}

/* CHAZFOGC +3 -- CM p. 248. */
void chazfogc_after_checkmm02_yes(void)
{
    set_A(PRIO16);
    tc(CHAZFOGC + 4u, FINDVAC); /* 2CADR AZMTHCG1 */
}

/* CHAZFOGC +7 -- CM p. 248. */
void chazfogc_after_findvac(void)
{
    tc(CHAZFOGC + 7u, PHASCHNG); /* OCT 00174 */
}

/* CHAZFOGC +9 -- CM p. 248. */
void chazfogc_after_phaschng(void)
{
    tc(CHAZFOGC + 9u, GOPIN);
}

/* STROKON -- CM p. 251.
 * Verb 68, CSM stroke test on, "PERMITTED ONLY DURING TVC": DAPBIT1 and
 * DAPBIT2 (bits 15 and 14 of FLAGWRD6, OCT60000) 1 and 0. CS and MASK
 * leave their complement, positive only then; +0 or negative is "NOT
 * TVC....FLASH OP ERROR LIGHT" (BZMF). STRKTSTI at priority 30. */
void strokon(void)
{
    cs(ADDRESS_OF(FLAGWRD6));
    mask(OCT60000);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF ALM/END */
        continue_at(ALM_END);
        return;
    }
    set_A(PRIO30);
    tc(STROKON + 5u, NOVAC); /* 2CADR STRKTSTI */
}

/* STROKON +8 -- CM p. 251. */
void strokon_after_novac(void)
{
    tc(STROKON + 8u, GOPIN);
}
#endif

/* ATTACHED -- LM p. 297, CM p. 264.
 * Verb 66, "ATTACHED MOVE THIS TO OTHER STATE" (ATTACHIT, priority 10). */
void attached(void)
{
    set_A(PRIO10);
    tc(ATTACHED + 1u, FINDVAC); /* 2CADR ATTACHIT */
}

/* ATTACHED +4 -- LM p. 297, CM p. 264. */
void attached_after_findvac(void)
{
    tc(ATTACHED + 4u, ENDOFJOB);
}

/* V67 -- LM p. 298, CM p. 267.
 * Verb 67, the W matrix monitor (V67CALL, priority 5). */
void v67(void)
{
    tc(V67, TESTXACT);
}

/* V67 +1 -- LM p. 298, CM p. 267. */
void v67_after_testxact(void)
{
    set_A(PRIO5);
    tc(V67 + 2u, FINDVAC); /* 2CADR V67CALL */
}

/* V67 +5 -- LM p. 298, CM p. 267. */
void v67_after_findvac(void)
{
    tc(V67 + 5u, ENDOFJOB);
}

/* ---- The hand-overs ---- */

/* GOLOADLV -- LM p. 274, CM p. 244.
 * The "please perform" verbs (50, 52-54, 97 and 99 in the LM; 50, 51, 53,
 * 59, 97 and 99 in the CM): the flashing display off (FLASHOFF), then
 * LOADLV1, in PINBALL's superbank: CAF PINSUPBT, WRITE SUPERBNK, "TURN ON
 * FE7" (the CM). */
void goloadlv(void)
{
    tc(GOLOADLV, FLASHOFF);
}

/* GOLOADLV +1 -- LM p. 274, CM p. 244. */
void goloadlv_after_flashoff(void)
{
    set_A(PINSUPBT);
    write_superbnk();            /* EXTEND; WRITE SUPERBNK */
    tc(GOLOADLV + 4u, POSTJUMP); /* CADR LOADLV1 */
}

#if LUMINARY099
/* ALINTIME -- LM p. 275.
 * Verb 55, align time: R33, "NO ROOM IN 43". */
void alintime(void)
{
    tc(ALINTIME, TESTXACT);
}

/* ALINTIME +1 -- LM p. 275. */
void alintime_after_testxact(void)
{
    tc(ALINTIME + 1u, POSTJUMP); /* CADR R33 */
}
#endif

/* V82PERF -- LM p. 283, CM p. 255.
 * Verb 82, the orbit parameter display (R30): at priority 7 ("LESS THAN
 * LAMBERT"), V82CALL in this job. The LM goes there by SUPDXCHZ with
 * V82CON, its 2CADR ("V82CALL IN DIFF SUPERBANK FROM V82PERF"); the CM
 * by POSTJUMP ("V82CALL MUST NOT BE A FINDVAC JOB"). */
void v82perf(void)
{
    tc(V82PERF, TESTXACT);
}

/* V82PERF +1 -- LM p. 283, CM p. 255. */
void v82perf_after_testxact(void)
{
    set_A(PRIO7);
    tc(V82PERF + 2u, PRIOCHNG);
}

/* V82PERF +3 -- LM p. 283, CM p. 255. */
void v82perf_after_priochng(void)
{
#if LUMINARY099
    set_L(V82CON[1]); /* EXTEND; DCA V82CON */
    set_A(V82CON[0]);
    tc(V82PERF + 5u, SUPDXCHZ);
#else
    tc(V82PERF + 3u, POSTJUMP); /* CADR V82CALL */
#endif
}

#if LUMINARY099
/* TRMTRACK -- LM p. 287.
 * Verb 56, "TERMINATE TRACKING -- P20 + P25": nothing to do unless the
 * rendezvous or P25 flag is on (BITS9+7 of FLAGWRD0); else both off and
 * the search option ("ENSURE SEARCH FLAG IS OFF"), and if the track flag
 * is on, on to the rest in another bank. */
void trmtrack(void)
{
    set_A(BITS9_PLUS_7); /* "IS REND OR P25 FLAG ON" */
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (A_is_zero()) { /* EXTEND; BZF GOPIN: "NO" */
        continue_at(GOPIN);
        return;
    }
    tc(TRMTRACK + 4u, DOWNFLAG); /* ADRES RNDVZFLG */
}

/* TRMTRACK +6 -- LM p. 287. */
void trmtrack_after_downflag(void)
{
    tc(TRMTRACK + 6u, DOWNFLAG); /* ADRES P25FLAG */
}

/* TRMTRACK +8 -- LM p. 287. */
void trmtrack_after_second_downflag(void)
{
    tc(TRMTRACK + 8u, DOWNFLAG); /* ADRES SRCHOPTN */
}

/* TRMTRACK +10 -- LM p. 287: "IS TRACK FLAG ON?" */
void trmtrack_after_third_downflag(void)
{
    set_A(TRACKBIT);
    mask_at(ADDRESS_OF(FLAGWRD1));
    if (A_is_zero()) { /* EXTEND; BZF GOPIN */
        continue_at(GOPIN);
        return;
    }
    tc(TRMTRACK + 14u, POSTJUMP); /* CADR TRMTRAK1 */
}

/* DAPDISP -- LM p. 292.
 * Verb 48, load the autopilot's data (R03): DAPDATA1 at priority 7. */
void dapdisp(void)
{
    tc(DAPDISP, TESTXACT);
}

/* DAPDISP +1 -- LM p. 292. */
void dapdisp_after_testxact(void)
{
    set_A(PRIO7); /* "R03" */
    tc(DAPDISP + 2u, PRIOCHNG);
}

/* DAPDISP +3 -- LM p. 292. */
void dapdisp_after_priochng(void)
{
    tc(DAPDISP + 3u, POSTJUMP); /* CADR DAPDATA1 */
}

/* VERB85 -- LM pp. 298-299.
 * Verb 85, display the rendezvous radar's line of sight (DSPRRLOS). */
void verb85(void)
{
    tc(VERB85, TESTXACT);
}

/* VERB85 +1 -- LM p. 299. */
void verb85_after_testxact(void)
{
    tc(VERB85 + 1u, POSTJUMP); /* CADR DSPRRLOS */
}
#else
/* STABLISH -- CM p. 251.
 * Verb 46, "ESTABLISH G+C CONTROL", in E-bank 6. During TVC (DAPBIT1 and
 * DAPBIT2 1 and 0, tested as STROKON does) a docked CSM/LM (bit 14 of
 * DAPDATR1) switches from the high to the low bandwidth autopilot
 * (PRESWTCH), and a CSM alone is an operator error; else DAPFIG sets up
 * the RCS, entry or Saturn autopilot. The BZMFs go 8 and 3 words on, to
 * the words the listing marks +8 and +3. */
void stablish(void)
{
    set_A(EBANK6); /* "SET EBANK TO E6" */
    ts(EBANK_ADDRESS);
    cs(ADDRESS_OF(FLAGWRD6)); /* "TEST FOR TVC" */
    mask(OCT60000);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +8 */
        continue_at(STABLISH + 13u);
        return;
    }
    ca(ADDRESS_OF(DAPDATR1)); /* CAE DAPDATR1: "TEST FOR CSM/LM" */
    mask(BIT14);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +3 */
        continue_at(STABLISH + 12u);
        return;
    }
    tc(STABLISH + 10u, POSTJUMP); /* CADR PRESWTCH */
}

/* STABLISH +12, the listing's +3 -- CM p. 251: "CSM, SO ALARM AND EXIT". */
void stablish_plus12(void)
{
    tc(STABLISH + 12u, ALM_END);
}

/* STABLISH +13, the listing's +8 -- CM p. 251.
 * "SET UP RCS, ENTRY, OR SATURN-STICK DAP". */
void stablish_plus13(void)
{
    tc(STABLISH + 13u, POSTJUMP); /* CADR DAPFIG */
}

/* GOTOR21 -- CM p. 257.
 * Verb 57, start R21: "CLEAR R23FLG", then on as verb 54 does from its
 * TESTXACT (TC +3). */
void gotor21(void)
{
    tc(GOTOR21, DOWNFLAG); /* ADRES R23FLG */
}

/* GOTOR21 +2 -- CM p. 257. */
void gotor21_after_downflag(void)
{
    tc(GOTOR21 + 2u, GOTOR21 + 5u); /* TC +3: GOTOR23 +2 */
}

/* GOTOR23 -- CM p. 257.
 * Verb 54, "PLEASE MARK (R-21-BACKUP)": "SET R23FLG". "VB 57
 * UNACCEPTABLE UNLESS BOTH RENDEZVOUS AND TRACK FLAGS ON": else alarm 406
 * (R22ALARM). Then R23CSM if R23FLG is set, R21CSM if not, at priority
 * 16. */
void gotor23(void)
{
    tc(GOTOR23, UPFLAG); /* ADRES R23FLG */
}

/* GOTOR23 +2 -- CM p. 257. */
void gotor23_after_upflag(void)
{
    tc(GOTOR23 + 2u, TESTXACT);
}

/* GOTOR23 +3 -- CM p. 257. */
void gotor23_after_testxact(void)
{
    ca(ADDRESS_OF(FLAGWRD0));
    mask(RNDVZBIT);
    if (A_is_zero()) { /* EXTEND; BZF R22ALARM */
        continue_at(R22ALARM);
        return;
    }
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRACKBIT);
    if (A_is_zero()) { /* EXTEND; BZF R22ALARM */
        continue_at(R22ALARM);
        return;
    }
    ca(ADDRESS_OF(FLAGWRD1)); /* "TEST R23FLG" */
    mask(R23BIT);
    if (A_is_zero()) { /* EXTEND; BZF REGR21: "R21" */
        continue_at(REGR21);
        return;
    }
    set_A(PRIO16);
    tc(GOTOR23 + 16u, NOVAC); /* 2CADR R23CSM */
}

/* GOTOR23 +19 -- CM p. 257. */
void gotor23_after_novac(void)
{
    tc(GOTOR23 + 19u, ENDOFJOB);
}

/* REGR21 -- CM p. 257. */
void regr21(void)
{
    set_A(PRIO16);
    tc(REGR21 + 1u, NOVAC); /* 2CADR R21CSM */
}

/* REGR21 +4 -- CM p. 257. */
void regr21_after_novac(void)
{
    tc(REGR21 + 4u, ENDOFJOB);
}

/* R22ALARM -- CM p. 257.
 * "VERB 57 WAS SELECTED AND NEITHER REND NOR TRACK FLAG WERE ON": alarm
 * 406, and the display system released (ENDEXT). */
void r22alarm(void)
{
    tc(R22ALARM, ALARM); /* OCT 00406 */
}

/* R22ALARM +2 -- CM p. 257. */
void r22alarm_after_alarm(void)
{
    tc(R22ALARM + 2u, ENDEXT);
}

/* VERB94 -- CM pp. 265-266.
 * Verb 94, R64: only if V94FLAG is set ("IS V94FLAG SET", else "NO -
 * OPERATOR ERROR"), which it clears, and only in P23 ("IS IT P23"). Then
 * "SET GROUP 2 TO DO R64" (PHASCHNG, OCT 112) and "CAUSE RESTART"
 * (CLEANOUT). */
void verb94(void)
{
    set_A(V94FLBIT);
    mask_at(ADDRESS_OF(FLAGWRD9));
    if (A_is_zero()) { /* EXTEND; BZF ALM/END */
        continue_at(ALM_END);
        return;
    }
    tc(VERB94 + 4u, DOWNFLAG); /* ADRES V94FLAG */
}

/* VERB94 +6 -- CM p. 266. */
void verb94_after_downflag(void)
{
    tc(VERB94 + 6u, CHECKMM); /* MM 23 */
}

/* VERB94 +8 -- CM p. 266. */
void verb94_after_checkmm23_no(void)
{
    tc(VERB94 + 8u, ALM_END);
}

/* VERB94 +9 -- CM p. 266. */
void verb94_after_checkmm23_yes(void)
{
    tc(VERB94 + 9u, PHASCHNG); /* OCT 112 */
}

/* VERB94 +11 -- CM p. 266. */
void verb94_after_phaschng(void)
{
    tc(VERB94 + 11u, CLEANOUT);
}
#endif

/* ---- The checks ---- */

/* CKMODCAD -- LM p. 274, CM pp. 243-244.
 * The IMU verbs' check that no one is using MODECADR: if it is +0 or -0,
 * back to the caller (TCQ); else "SOMEBODY IS USING MODECADR SO EXIT"
 * (the operator error light). The CM's OPTCOARK checks OPTCADR the same
 * way, with TC CKMODCAD +1. */
void ckmodcad(void)
{
    ca(ADDRESS_OF(MODECADR));
    ckmodcad_plus1();
}

/* CKMODCAD +1 -- LM p. 274, CM p. 244. */
void ckmodcad_plus1(void)
{
    if (A_is_zero()) { /* EXTEND; BZF TCQ */
        continue_at(TCQ);
        return;
    }
    tc(CKMODCAD + 3u, ALM_END);
}

#if COMANCHE055
/* CHKPOOH -- CM p. 261.
 * "CHECK FOR P00 OR P00-.": with MODREG +0 or -0, return to the caller
 * (TCQ, a TC Q); else the operator error light, by TCF. (The LM's is in
 * part 1.) */
void chkpooh(void)
{
    ca(ADDRESS_OF(MODREG));
    if (A_is_zero()) { /* EXTEND; BZF TCQ */
        continue_at(TCQ);
        return;
    }
    continue_at(ALM_END); /* TCF ALM/END */
}

/* EXDAPOFF -- CM p. 261.
 * The autopilot off: T5 to idle ("SET T5 TO IDLE": IDLECADR, the 2CADR of
 * T5IDLOC, into T5LOC) and "RESET DAPBITS 1 AND 2" (bits 15 and 14 of
 * FLAGWRD6), then TC Q. TC Q runs the word in Q as an instruction: a
 * return into fixed memory goes on there, with Q 3; any other word in Q
 * is left for the program to run at Z 2, as in the faithful port. */
void exdapoff(void)
{
    set_L(IDLECADR[1]); /* EXTEND; DCA IDLECADR */
    set_A(IDLECADR[0]);
    dxch(ADDRESS_OF(T5LOC));
    set_A(negate(OCT60000)); /* CS OCT60000 */
    mask_at(ADDRESS_OF(FLAGWRD6));
    ts(ADDRESS_OF(FLAGWRD6));
    if (Q >= 02000u && Q <= 07777u)
        tc_through(Q_ADDRESS); /* TC Q */
    else
        continue_at(Q_ADDRESS);
}
#endif

/* ---- Small settings ---- */

/* DNEDUMP -- LM p. 288, CM p. 260.
 * Verb 74, "INITIALIZE DOWN-TELEMETRY PROGRAM FOR ERASABLE DUMP": the
 * down-telemetry goes to DNDUMPI next (LDNDUMPI, its REMADR, in
 * DNTMGOTO). */
void dnedump(void)
{
    set_A(LDNDUMPI);
    ts(ADDRESS_OF(DNTMGOTO));
    tc(DNEDUMP + 2u, GOPIN);
}

#if COMANCHE055
/* V60 -- CM p. 244.
 * Verb 60, "SET ASTRONAUT TOTAL ATTITUDE (N17) EQUAL TO PRESENT ATTITUDE":
 * CPHIX, CPHIY and CPHIZ from the CDUs. */
void v60(void)
{
    dca(ADDRESS_OF(CDUX)); /* EXTEND; DCA CDUX */
    dxch(ADDRESS_OF(CPHIX));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(CPHIX) + 2u);
    tc(V60 + 5u, GOPIN);
}

/* V86PERF -- CM p. 258.
 * Verb 86, "BACKUP MARK REJECT (R23)": MRKBUF1 -1, in E-bank 7. */
void v86perf(void)
{
    set_A(EBANK7);
    xch(EBANK_ADDRESS);
    set_A(NEGONE);
    ts(ADDRESS_OF(MRKBUF1));
    tc(V86PERF + 4u, GOPIN);
}

/* V52 -- CM pp. 266-267.
 * Verb 52, the offset number for P22, only while P22 runs ("IS P22
 * OPERATING"). */
void v52(void)
{
    tc(V52, CHECKMM); /* MM 22 */
}

/* V52 +2 -- CM p. 266. */
void v52_after_checkmm22_no(void)
{
    tc(V52 + 2u, ALM_END);
}

/* V52 +3 -- CM pp. 266-267.
 * "SET BITS 12,11,10 OF LANDMARK = BITS 14,13,12 OF MARKSTAT AFTER SUBT.
 * THEM FROM 5 TO GET OFFSET MARK NO.", in LANDMARK's E-bank. PRIO7 is
 * those bits, OCT 7000: CS PRIO7 and MASK clear LANDMARK's. SR shifts a
 * word right as it is stored and again each time it is read, so the
 * second CA SR reads MARKSTAT shifted twice, bits 14-12 in bits 12-10.
 * CS A and AD PRIO5 make them 5 less the bits, and ADS puts them in
 * LANDMARK. */
void v52_after_checkmm22_yes(void)
{
    set_A(LANDBANK);
    ts(EBANK_ADDRESS);
    set_A(negate(PRIO7)); /* CS PRIO7 */
    mask_at(ADDRESS_OF(LANDMARK));
    ts(ADDRESS_OF(LANDMARK));
    ca(ADDRESS_OF(MARKSTAT));
    ts(SR_ADDRESS);
    ca(SR_ADDRESS);
    ca(SR_ADDRESS);
    mask(PRIO7);
    cs(A_ADDRESS);
    ad(PRIO5);
    ads(ADDRESS_OF(LANDMARK));
    tc(V52 + 16u, GOPIN);
}
#endif

/* ---- SHOWSUM ---- */

/* SHOWSUM -- LM p. 290, CM p. 263.
 * Verb 91's bank sums, in P00 only: ROPECHK, the self-check's sum of the
 * fixed banks (STSHOSUM), with SKEEP6 1, the "SHOWSUM OPTION", and SMODE
 * +0 ("TURN OFF SELF-CHECK"); it comes back to SDISPLAY with each bank.
 * The LM first lowers its job to priority 7 ("ALLOW OTHER CHARINS."). */
void showsum(void)
{
    tc(SHOWSUM, CHKPOOH);
}

/* SHOWSUM +1 -- LM p. 290, CM p. 263. */
void showsum_after_chkpooh(void)
{
    tc(SHOWSUM + 1u, TESTXACT);
}

/* The SHOWSUM option, then ROPECHK (STSHOSUM), from the word at at. */
static void enter_ropechk(word at)
{
    set_A(S_PLUS_1);
    ts(ADDRESS_OF(SKEEP6)); /* "SHOWSUM OPTION" */
    set_A(S_PLUS_ZERO);
    ts(ADDRESS_OF(SMODE)); /* "TURN OFF SELF-CHECK" */
    set_A(SELFADRS);
    ts(ADDRESS_OF(SELFRET));
    tc(at + 6u, STSHOSUM); /* "ENTER ROPECHK" */
}

#if LUMINARY099
/* SHOWSUM +2 -- LM p. 290. */
void showsum_after_testxact(void)
{
    set_A(PRIO7); /* "ALLOW OTHER CHARINS." */
    tc(SHOWSUM + 3u, PRIOCHNG);
}

/* SHOWSUM +4 -- LM p. 290. */
void showsum_after_priochng(void)
{
    enter_ropechk(SHOWSUM + 4u);
}
#else
/* SHOWSUM +2 -- CM p. 263. */
void showsum_after_testxact(void)
{
    enter_ropechk(SHOWSUM + 2u);
}
#endif

/* SDISPLAY -- LM p. 290, CM p. 263.
 * The bank's number, in L, into SKEEP2 "FOR DISPLAY" (LXCH SKEEP2), and
 * the word SKEEP2 held, the "BUGGER WORD", into SKEEP3 (LXCH SKEEP3).
 * Then NOKILL. */
void sdisplay(void)
{
    lxch(ADDRESS_OF(SKEEP2));
    lxch(ADDRESS_OF(SKEEP3));
    nokill();
}

/* NOKILL -- LM p. 290, CM p. 263.
 * A flashing verb 05 noun 01 (VNCON) of the words from SKEEP1 (ADRS1, its
 * address, in MPAC +2), by GOXDSPF in its own bank (BANKCALL). */
void nokill(void)
{
    set_A(ADRS1);
    ts(ADDRESS_OF(MPAC) + 2u);
    set_A(VNCON);              /* "0501" */
    tc(NOKILL + 3u, BANKCALL); /* CADR GOXDSPF */
}

/* NOKILL +5 -- LM p. 290, CM p. 263.
 * GOXDSPF returns here on TERMINATE: done (TC +3). */
void nokill_after_terminate(void)
{
    tc(NOKILL + 5u, NOKILL + 8u);
}

/* NOKILL +6 -- LM p. 290, CM p. 263.
 * On PROCEED: the next bank (NXTBNK). */
void nokill_after_proceed(void)
{
    tc(NOKILL + 6u, NXTBNK);
}

/* NOKILL +7 -- LM p. 291, CM p. 263.
 * On ENTER: the same bank again. */
void nokill_after_enter(void)
{
    tc(NOKILL + 7u, NOKILL);
}

/* NOKILL +8 -- LM p. 291, CM p. 263.
 * SKEEP1 gets SELFADRS, the self-check's address, and the display system
 * is released (ENDEXT). */
void nokill_plus8(void)
{
    set_A(SELFADRS);
    ts(ADDRESS_OF(SKEEP1));
    tc(NOKILL + 10u, ENDEXT);
}

/* ENDSUMS -- LM p. 291, CM p. 263.
 * The self-check at the end of the banks: with SKEEP6 +0 or -0 "START
 * SELFCHK AGAIN", else "START SHOWSUM AGAIN". */
void endsums(void)
{
    ca(ADDRESS_OF(SKEEP6));
    if (A_is_zero()) { /* EXTEND; BZF SELFCHK */
        continue_at(SELFCHK);
        return;
    }
    tc(ENDSUMS + 3u, STSHOSUM);
}
