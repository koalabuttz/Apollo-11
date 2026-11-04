/* SPDX-License-Identifier: MIT */
/* R30.agc (Luminary099 pp. 712-722, Comanche055 pp. 514-524): R30, verb
 * 82, which will "CALCULATE AND DISPLAY ORBITAL PARAMETERS", and SR30.1,
 * the "ORBITAL PARAMETERS DISPLAY FOR NOUNS 32 AND 44". Its basic code, and
 * its interpretive code in the listing's notation (src/interpreter/
 * notation.h), which the readable interpreter runs (src/binding/
 * r30_routines.c). The faithful C is c64-port/src/lm_r30.c and cm_r30.c.
 * The LM's is in bank 22, the CM's in bank 23.
 *
 * "IF AVERAGE G IS OFF", verb 04 noun 06 asks which vehicle's state vector
 * to use ("INITIAL CHOICE IS THIS SHIP (R2=1)"); a job (V82GOFF1)
 * integrates it to now (THISPREC or OTHPREC), works out the parameters
 * (SR30.1) and stops, and the display, verb 16 noun 44, shows HAPOX, HPERX
 * and TFF once the first are there (V82STALL). A waitlist task, TICKTEST,
 * counts TFF or -TPER on a second at a time until the display is answered.
 * "IF AVERAGE G IS ON", a job (V82GON1) works the parameters out from RN
 * and VN, which average G keeps, about once a second, while the display
 * shows them. The CM's job, in P00 or P11 (MODREG +0 or 11), also updates
 * the splash-down error (DELRSPL).
 *
 * SR30.1 takes the state in RONE and VONE about the earth or the moon
 * (V82EMFLG), with TFF/RTMU, HPERMIN and RPADTEM for that body, and leaves
 * HAPOX, HPERX ("LIMITED TO 9999.9 NM", MAXCHK), -TPER (+0 if the perigee
 * is below HPERMIN) and TFF.
 *
 * Each routine ends where its code leaves it. A call out of the file
 * (BANKCALL, WAITLIST, FINDVAC, NOVAC, ENDEXT, B5OFF, ENDOFJOB, TASKOVER,
 * CHANG1, the CM's CHECKMM) or a TC INTPRET leaves Z at the routine and Q
 * at the word after the TC (for INTPRET the first interpretive word); a
 * BZF ENDEXT leaves Z at ENDEXT. A TC within the file (TC FLAGGON, TC +2,
 * TC -5, TC V82STALL ...) goes on into the code it names. Each TS after a
 * CAF would skip the next word on an overflow, which a CAF never leaves. */
#include "r30.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#if LUMINARY099
#define R30_BANK 022u
#else
#define R30_BANK 023u
#endif

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

/* TC at, to a routine out of the file or to INTPRET: Q gets the return
 * (for INTPRET, the first interpretive word), and the routine runs
 * next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- V82CALL -- LM pp. 713-714, CM pp. 515-516 ---- */

/* V82CALL: to the interpreter, for V82CALL_CODE. */
void v82call(void)
{
    tc_to(V82CALL, INTPRET);
}

/* "IF AVERAGE G ON", V82GON; else V82GOFF, back to the basic code. */
static const word v82call_words[] = {
    ORDERS(BON, GOTO),         SWITCH(BON, AVEGFLAG), ADDRESS(V82GON), /* "IF AVERAGE G ON" */
    ADDRESS(V82GOFF),                                                  /* "IF AVERAGE G OFF" */
    /* V82GOFF */ ORDER(EXIT), /* "ALLOW ASTRONAUT TO SELECT VEHICLE" */
};

/* "DESIRED FOR ORBITAL PARAMETERS CALCULATION AND DISPLAY": OPTIONX 2
 * and OPTIONX +1 1 (this vehicle), */
void v82goff_options(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(OPTIONX));
    set_A(ONE);
    ts(ADDRESS_OF(OPTIONX) + 1u);
    v82goff_display();
}

/* flashed with verb 04 noun 06 (OPTIONVN; BANKCALL, CADR GOXDSPF). */
void v82goff_display(void)
{
    set_A(OPTIONVN); /* "V 04 N 06" */
    tc_to(V82GOFF + 6u, BANKCALL);
}

/* GOXDSPF's returns: terminate (TC ENDEXT), */
void v82goff_terminate(void)
{
    tc_to(V82GOFF + 8u, ENDEXT);
}

/* proceed (TC +2), */
void v82goff_proceed(void)
{
    tc_at(V82GOFF + 9u);
    v82goff_ticktest();
}

/* and "DATA IN. OPTION1+1 = 1 FOR THIS VEHICLE. UNEQ 1 FOR OTHER
 * VEHICLE." (TC -5): flash again. */
void v82goff_recycle(void)
{
    tc_at(V82GOFF + 10u);
    v82goff_display();
}

/* On proceed, TICKTEST in "80 MS" (WAITLIST, 2CADR TICKTEST), */
void v82goff_ticktest(void)
{
    set_A(BIT4); /* "80 MS" */
    tc_to(V82GOFF + 12u, WAITLIST);
}

/* then RELINT and on into V82GOFLP. */
void v82goflp_relint(void)
{
    relint();
    v82goflp();
}

/* V82GOFLP, the "MAJOR RECYCLE LOOP ENTRY": EBANK TFF's, V82FLAGS +0
 * ("ZERO FLAGS FOR TICKTEST. INHIBITS DECREMENTING OF TFF AND -TPER"), and
 * a job at V82GOFF1, priority 7, which "WILL EXECUTE STATE VECTOR UPDATE
 * AND ORBIT CALCULATIONS FOR SELECTED VEHICLE ABOUT PROPER BODY" (FINDVAC,
 * 2CADR V82GOFF1). */
void v82goflp(void)
{
    set_A(TFFBANK);
    ts(EBANK_ADDRESS);
    set_A(ZERO);
    ts(ADDRESS_OF(V82FLAGS));
    set_A(PRIO7);
    tc_to(V82GOFLP + 5u, FINDVAC);
}

/* Then RELINT and on into V82STALL. */
void v82stall_relint(void)
{
    relint();
    v82stall();
}

/* V82STALL: "STALL IN THIS LOOP AND WITHOLD V 16 N 44 UNTIL STATE VECTOR
 * UPDATE SETS ONE OF OUR FLAG BITS" (CCS A on bits 1 and 2 of V82FLAGS):
 * either, and FLAGGON "EXIT FROM STALL LOOP" (TC FLAGGON); */
void v82stall(void)
{
    set_A(THREE);
    mask_at(ADDRESS_OF(V82FLAGS));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(V82STALL + 3u); /* TC FLAGGON */
        flaggon();
        return;
    }
    v82stall_wait();
}

/* neither, and the job sleeps a second (BANKCALL, CADR DELAYJOB), */
void v82stall_wait(void)
{
    set_A(ONE_SEC);
    tc_to(V82STALL + 5u, BANKCALL);
}

/* and looks again (TC V82STALL). */
void v82stall_again(void)
{
    tc_at(V82STALL + 7u);
    v82stall();
}

/* FLAGGON: "MONITOR HAPO,HPER,TFF", verb 16 noun 44 (BANKCALL, CADR
 * GOXDSPF). */
void flaggon(void)
{
    set_A(V16N44);
    tc_to(FLAGGON + 1u, BANKCALL);
}

/* Its returns: terminate, "THIS TELLS TICKTEST TO KILL ITSELF" (B5OFF:
 * bit 5 of EXTVBACT clear), */
void flaggon_terminate(void)
{
    tc_to(FLAGGON + 3u, B5OFF);
}

/* proceed, "DITTO", */
void flaggon_proceed(void)
{
    tc_to(FLAGGON + 4u, B5OFF);
}

/* and recycle, "RECOMPUTE STATE VECT + DISPLAY" (TC V82GOFLP). */
void flaggon_recycle(void)
{
    tc_at(FLAGGON + 5u);
    v82goflp();
}

/* ---- V82GOFF1 -- LM pp. 714-716, CM pp. 516-518 ---- */

/* V82GOFF1, the job: to the interpreter, for V82GOFF1_CODE. */
void v82goff1(void)
{
    tc_to(V82GOFF1, INTPRET);
}

/* "TIME FOR STATE VECTOR UPDATE" (TDEC1) and "TIME FOR INTERNAL USE"
 * (TSTART82): now (LOADTIME). */
static const word v82goff1_words[] = {
    ORDER(RTB),      ADDRESS(LOADTIME), STORE(TDEC1), /* "TIME FOR STATE VECTOR UPDATE." */
    STORE(TSTART82),                                  /* "TIME FOR INTERNAL USE." */
    ORDER(EXIT),
};

/* OPTIONX +1 "1 FOR THIS VEHICLE, NOT 1 FOR OTHER": THISSHIP's or
 * OTHSHIP's update (EXTEND; BZF THISSHIP), each in interpretive code. */
void v82goff1_vehicle(void)
{
    cs(ADDRESS_OF(OPTIONX) + 1u);
    ad(ONE);
    if (A_is_zero()) { /* EXTEND; BZF THISSHIP */
        tc_to(THISSHIP, INTPRET);
        return;
    }
    tc_to(OTHSHIP, INTPRET);
}

/* OTHSHIP: "CALL STATE VECTOR UPDATE FOR OTHER SHIP" (OTHPREC). BOTHSHIP:
 * "MOVE RESULTS INTO TFFCONIC STORAGE AREAS TO BE CALLED BY SR30.1", RATT
 * to RONE and VATT to VONE, and the earth's or the moon's parameters, by X2
 * "AS LEFT BY THISPREC OR OTHPREC" ("X2 IS 0 FOR EARTH CENTERED STATE VEC",
 * "X2 IS 2 FOR MOON"): 1/RTMUE or 1/RTMUM to TFF/RTMU, MINPERE or MINPERM
 * to HPERMIN; then EARTHPAD or MOONPAD. */
static const word othship_words[] = {
    ORDER(CALL), /* "CALL STATE VECTOR UPDATE FOR OTHER SHIP." */
    ADDRESS(OTHPREC),
    /* BOTHSHIP */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(RONE), /* "RATT AT (-29)M FOR EARTH OR MOON" */
    ADDRESS_PLUS_ONE(VATT),
    STORE(VONE), /* "VATT AT (-7)M/CS FOR EARTH OR MOON" */
    ORDER(DLOAD_STAR),
    ADDRESS_PLUS_ONE_AT_X2(1_RTMUE, 0u), /* "X2 IS 0 FOR EARTH CENTERED STATE VEC" */
    STORE(TFF_RTMU),                     /* "X2 IS 2 FOR MOON" */
    ORDER(DLOAD_STAR),                   /* "AS LEFT BY THISPREC OR OTHPREC." */
    ADDRESS_PLUS_ONE_AT_X2(MINPERE, 0u),
    STORE(HPERMIN),      /* "TFFRTMU, HPERMIN AND RPADTEM ARE ALL" */
    ORDERS(SLOAD, BHIZ), /* "EARTH/MOON PARAMETERS AS SET HERE." */
    ADDRESS_PLUS_ONE(X2),
    ADDRESS(EARTHPAD),
    ORDER(GOTO),
    ADDRESS(MOONPAD),
};

/* THISSHIP: "CALL STATE VECTOR UPDATE FOR THIS SHIP" (THISPREC), then
 * BOTHSHIP. */
static const word thisship_words[] = {
    ORDER(CALL), /* "CALL STATE VECTOR UPDATE FOR THIS SHIP." */
    ADDRESS(THISPREC),
    ORDER(GOTO),
    ADDRESS(BOTHSHIP),
};

/* EARTHPAD: "PAD 37-B RADIUS" (RPAD), V82EMFLG clear ("INDICATE EARTH
 * SCALING FOR SR30.1"). MOONPAD: "COMPUTE MOON PAD RADIUS FROM RLS VECTOR",
 * V82EMFLG set. BOTHPAD: the radius to RPADTEM, and SR30.1 "CALCULATE
 * ORBITAL PARAMETERS". Then (the CM's after SPLRET1) "PRESENT TIME - TIME
 * V82GOFF1 BEGAN" to TSTART82, and as SR30.1 left -TPER ("SR30.1 SETS
 * -TPER=0 IF HPER L/ HPERMIN"): +0, TICKTFF; else TICKTPER, -TPER
 * "CORRECTED FOR TIME SINCE V82GOFF1 BEGAN". */
static const word earthpad_words[] = {
    ORDERS(DLOAD, CLRGO), /* "PAD 37-B RADIUS. SCALED AT (-29)M." */
    ADDRESS_PLUS_ONE(RPAD),
    SWITCH(CLRGO, V82EMFLG), /* "INDICATE EARTH SCALING FOR SR30.1" */
    ADDRESS(BOTHPAD),
    /* MOONPAD */ ORDERS(VLOAD, ABVAL), /* "COMPUTE MOON PAD RADIUS FROM RLS VECTOR." */
    ADDRESS_PLUS_ONE(RLS),              /* "SCALED AT (-27)M." */
    ORDER(SET),
    SWITCH(SET, V82EMFLG), /* "INDICATE MOON SCALING FOR SR30.1" */
    /* BOTHPAD */ STCALL(RPADTEM),
    ADDRESS(SR30_1), /* "CALCULATE ORBITAL PARAMETERS" */
#if LUMINARY099
    ORDERS(RTB, DSU),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TSTART82), /* "PRESENT TIME - TIME V82GOFF1 BEGAN" */
    STORE(TSTART82),            /* "SAVE IT" */
    ORDERS(DLOAD, BZE),         /* "SR30.1 SETS -TPER=0 IF HPER L/" */
    ADDRESS_PLUS_ONE(MTPER),    /* "HPERMIN (300 OR 35) KFT." */
    ADDRESS(TICKTFF),
    /* TICKTPER */ ORDERS(DLOAD, DAD), /* "(-TPER NON ZERO) TFF WAS NOT COMPUTED," */
    ADDRESS_PLUS_ONE(MTPER),           /* "BUT WAS SET TO 59M59S. DONT TICK TFF, DO" */
    ADDRESS_PLUS_ONE(TSTART82),        /* "TICK -TPER. DISPLAY BOTH." */
    STORE(MTPER),                      /* "-TPER CORRECTED FOR TIME SINCE V82GOFF1" */
#endif
    ORDER(EXIT),
};

#if !LUMINARY099
/* BOTHPAD +3: in P00 (MODREG +0), "YES, DO DELRSPL" (CANDEL); else on at
 * SPLRET1, each in interpretive code. */
void bothpad_modreg(void)
{
    ca(ADDRESS_OF(MODREG)); /* "ARE WE IN POO" */
    if (A_is_zero()) {      /* EXTEND; BZF CANDEL */
        candel();
        return;
    }
    tc_to(SPLRET1, INTPRET);
}

/* SPLRET1: the time and -TPER, as the LM's after BOTHPAD. */
static const word splret1_words[] = {
    ORDERS(RTB, DSU),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TSTART82), /* "PRESENT TIME - TIME V82GOFF1 BEGAN" */
    STORE(TSTART82),            /* "SAVE IT" */
    ORDERS(DLOAD, BZE),         /* "SR30.1 SETS -TPER=0 IF HPER L/" */
    ADDRESS_PLUS_ONE(MTPER),    /* "HPERMIN (300 OR 35) KFT." */
    ADDRESS(TICKTFF),
    /* TICKTPER */ ORDERS(DLOAD, DAD), /* "(-TPER NON ZERO) TFF WAS NOT COMPUTED," */
    ADDRESS_PLUS_ONE(MTPER),           /* "BUT WAS SET TO 59M59S. DONT TICK TFF, DO" */
    ADDRESS_PLUS_ONE(TSTART82),        /* "TICK -TPER. DISPLAY BOTH." */
    STORE(MTPER),                      /* "-TPER CORRECTED FOR TIME SINCE V82GOFF1" */
    ORDER(EXIT),
};
#endif

/* After TICKTPER: "INFORMS TICKTEST TO INCREMENT ONLY -TPER" (V82FLAGS bit
 * 1), and the job ends (TC ENDOFJOB). */
void tickpart_tper(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(V82FLAGS)); /* "INFORMS TICKTEST TO INCREMENT ONLY -TPER" */
    tc_to(TICKTPER + 7u, ENDOFJOB);
}

/* TICKTFF: "(-TPER=0) TFF WAS COMPUTED. TICK TFF. DO NOT TICK -TPER.
 * DISPLAY TFF, BUT NOT -TPER": TFF "CORRECTED FOR TIME SINCE V82GOFF1
 * BEGAN". */
static const word ticktff_words[] = {
    ORDERS(DLOAD, DAD), /* "(-TPER=0) TFF WAS COMPUTED. TICK TFF." */
    ADDRESS_PLUS_ONE(TFF),
    ADDRESS_PLUS_ONE(TSTART82),
    STORE(TFF), /* "TFF CORRECTED FOR TIME SINCE V82GOFF1" */
    ORDER(EXIT),
};

/* After TICKTFF: "INFORMS TICKTEST TO INCREMENT ONLY TFF" (bit 2), and the
 * job ends. */
void tickpart_tff(void)
{
    set_A(BIT2);
    ts(ADDRESS_OF(V82FLAGS)); /* "INFORMS TICKTEST TO INCREMENT ONLY TFF." */
    tc_to(TICKTFF + 7u, ENDOFJOB);
}

/* ---- TICKTEST -- LM p. 716, CM p. 518 ---- */

/* TICKTEST: "THIS WAITLIST PROGRAM PERPETUATES ITSELF ONCE A SEC UNTIL BIT
 * 5 OF EXTVBACT =0" (CCS A): set, DOTICK (TC DOTICK); */
void ticktest(void)
{
    set_A(BIT5);
    mask_at(ADDRESS_OF(EXTVBACT));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(TICKTEST + 3u); /* TC DOTICK */
        dotick();
        return;
    }
    ticktest_end();
}

/* clear, "TERMINATE V 82. CANT CALL ENDEXT IN RUPT" (NOVAC, 2CADR ENDEXT,
 * priority 25), */
void ticktest_end(void)
{
    set_A(PRIO25);
    tc_to(TICKTEST + 5u, NOVAC);
}

/* and the task ends (TC TASKOVER). */
void ticktest_taskover(void)
{
    tc_to(TICKTEST + 8u, TASKOVER);
}

/* DOTICK: "RE-REQUEST TICKTEST" in a second (WAITLIST, 2CADR TICKTEST), */
void dotick(void)
{
    set_A(ONE_SEC);
    tc_to(DOTICK + 1u, WAITLIST);
}

/* then by V82FLAGS (INDEX A; TC +1): "IF NO FLAGBITS SET DONT CHANGE TFF
 * OR -TPER, BUT CONTINUE LOOP" (TC TASKOVER); "ONLY BIT 1 SET. INCR -TPER BY
 * 1 SEC." (TPERTICK); "ONLY BIT 2 SET. INCR TFF BY 1 SEC." (TFFTICK); both
 * (which no program sets), TFFTICK's TS L with the 3 in A. */
static void tfftick_ts_l(void);
void dotick_flags(void)
{
    set_A(THREE);
    mask_at(ADDRESS_OF(V82FLAGS));
    tc_at(DOTICK + 7u); /* INDEX A; TC +1: Q the word after it */
    switch (a_word()) {
    case 0u: dotick_taskover(); break;
    case 1u:
        tc_at(DOTICK + 9u); /* TC TPERTICK */
        tpertick();
        break;
    case 2u: tfftick(); break;
    default: tfftick_ts_l(); break;
    }
}

void dotick_taskover(void)
{
    tc_to(DOTICK + 8u, TASKOVER);
}

/* TFFTICK: TFF one second on (DAS TFF, with the second in L). */
void tfftick(void)
{
    set_A(ONE_SEC);
    tfftick_ts_l();
}

static void tfftick_ts_l(void)
{
    ts(L_ADDRESS);
    set_A(ZERO);
    das(ADDRESS_OF(TFF));
    tc_to(TFFTICK + 4u, TASKOVER);
}

/* TPERTICK: -TPER one second on. */
void tpertick(void)
{
    set_A(ONE_SEC);
    ts(L_ADDRESS);
    set_A(ZERO);
    das(ADDRESS_OF(MTPER));
    tc_to(TPERTICK + 4u, TASKOVER);
}

/* ---- V82GON -- LM pp. 717-718, CM pp. 519-520 ---- */

/* V82GON: "AVERAGE G ON. USE CURRENT STATE VECTOR FOR ORBITAL PARAMETER
 * CALCULATIONS": back to the basic code. */
static const word v82gon_words[] = {
    ORDER(EXIT), /* "AVERAGE G ON." */
};

/* V82GON +1: a job at V82GON1, priority 7, "LESS THAN LAMBERT", which
 * "WILL PERFORM ORBIT CALCULATIONS ABOUT PROPER BODY APPROX ONCE PER SEC"
 * (FINDVAC, 2CADR V82GON1), */
void v82gon_findvac(void)
{
    set_A(PRIO7); /* "LESS THAN LAMBERT" */
    tc_to(V82GON + 2u, FINDVAC);
}

/* then RELINT, and "WITHOLD V16 N44 UNTIL FIRST ORBIT CALC IS DONE" (CCS
 * NEWJOB): a job waiting, CHANG1 lets it run first; else the display. */
void v82gon_relint(void)
{
    relint();
    if (ccs_at(ADDRESS_OF(NEWJOB)) == POSITIVE) {
        tc_to(V82GON + 7u, CHANG1);
        return;
    }
    v82redsp();
}

/* V82REDSP: "MONITOR HAPO, HPER, TFF", verb 16 noun 44 (BANKCALL, CADR
 * GOXDSPF). */
void v82redsp(void)
{
    set_A(V16N44);
    tc_to(V82REDSP + 1u, BANKCALL);
}

/* Its returns: terminate, "THIS TELLS V82GON1 TO KILL ITSELF" (B5OFF), */
void v82redsp_terminate(void)
{
    tc_to(V82REDSP + 3u, B5OFF);
}

/* proceed, "DITTO", */
void v82redsp_proceed(void)
{
    tc_to(V82REDSP + 4u, B5OFF);
}

/* and recycle (TC V82REDSP). */
void v82redsp_recycle(void)
{
    tc_at(V82REDSP + 5u);
    v82redsp();
}

/* V82GON1, the job that "PERPETUATES ITSELF ONCE A SEC UNTIL BIT 5 OF
 * EXTVBACT =0": to the interpreter, for V82GON1_CODE. */
void v82gon1(void)
{
    tc_to(V82GON1, INTPRET);
}

/* RN to RONE and VN to VONE (the GOTO "HOLDS OFF CCS NEWJOB BETWEEN RN AND
 * VN FETCH SO RN, VN ARE FROM SAME STATE VECTOR UPDATE"); by the
 * vehicle's sphere of influence (the LM's MOONTHIS, the CM's AMOONFLG:
 * "IF SET - MOON, IF RESET - EARTH") the moon's parameters (MOONGON:
 * V82EMFLG set, 1/RTMUM, MINPERM and RLS's length) or the earth's
 * (EARTHGON: V82EMFLG clear, 1/RTMUE, MINPERE, RPAD); V82GON2: SR30.1. */
static const word v82gon1_words[] = {
    ORDERS(VLOAD, GOTO),        /* "HOLDS OFF CCS NEWJOB BETWEEN RN AND" */
    ADDRESS_PLUS_ONE(RN),       /* "VN FETCH SO RN, VN ARE FROM SAME" */
    ADDRESS(NEXTLINE),          /* "STATE VECTOR UPDATE." */
    /* NEXTLINE */ STOVL(RONE), /* "RN AT (-29)M FOR EARTH OR MOON" */
    ADDRESS_PLUS_ONE(VN),
    STORE(VONE), /* "VN AT (-7)M/CS FOR EARTH OR MOON" */
    ORDERS(BON, GOTO),
#if LUMINARY099
    SWITCH(BON, MOONTHIS), /* "FLAG INDICATES BODY ABOUT WHICH ORBITAL" */
#else
    SWITCH(BON, AMOONFLG), /* "FLAG INDICATES BODY ABOUT WHICH ORBITAL" */
#endif
    ADDRESS(MOONGON),  /* "CALCULATIONS ARE TO BE PERFORMED." */
    ADDRESS(EARTHGON), /* "IF SET - MOON, IF RESET - EARTH." */
    /* MOONGON */ ORDERS(SET, DLOAD),
    SWITCH(SET, V82EMFLG),     /* "INDICATE MOON SCALING FOR SR30.1" */
    ADDRESS_PLUS_ONE(1_RTMUM), /* "LUNAR PARAMETERS LOADED HERE FOR SR30.1" */
    STODL(TFF_RTMU),
    ADDRESS_PLUS_ONE(MINPERM),
    STOVL(HPERMIN),
    ADDRESS_PLUS_ONE(RLS), /* "SCALED AT (-27)M." */
    ORDERS(ABVAL, GOTO),
    ADDRESS(V82GON2),
    /* EARTHGON */ ORDERS(CLEAR, DLOAD),
    SWITCH(CLEAR, V82EMFLG),   /* "INDICATE EARTH SCALING FOR SR30.1" */
    ADDRESS_PLUS_ONE(1_RTMUE), /* "EARTH PARAMETERS LOADED HERE FOR SR30.1" */
    STODL(TFF_RTMU),
    ADDRESS_PLUS_ONE(MINPERE),
    STODL(HPERMIN),
    ADDRESS_PLUS_ONE(RPAD),
    /* V82GON2 */ STCALL(RPADTEM), /* "COMMON CODE FOR EARTH & MOON." */
    ADDRESS(SR30_1),
    ORDER(EXIT),
};

#if !LUMINARY099
/* V82GON2 +3: in P11 (CHECKMM, DEC 11) CANDEL, */
void v82gon2_checkmm(void)
{
    tc_to(V82GON2 + 3u, CHECKMM);
}

/* "NOT IN MODE 11", V82GON3 (TC V82GON3). */
void v82gon2_not_11(void)
{
    tc_at(V82GON2 + 5u);
    v82gon3();
}

/* CANDEL, "IN MODE 11 OR 00": to the interpreter, for CANDEL_CODE. */
void candel(void)
{
    tc_to(CANDEL, INTPRET);
}

/* INTSTALL ("DELRSPL DOES INTWAKE"), and DELRSPL, the splash-down error from
 * TFF; "RETURN IS TO NEXT LINE (SPLRET)": back to the basic code. */
static const word candel_words[] = {
    ORDER(CALL),
    ADDRESS(INTSTALL), /* "DELRSPL DOES INTWAKE" */
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TFF),
    ADDRESS(DELRSPL), /* "RETURN IS TO NEXT LINE (SPLRET)." */
    /* SPLRET */ ORDER(EXIT),
};

/* SPLRET +1: from P00 (MODREG +0) on at SPLRET1, for the time and -TPER;
 * else V82GON3. */
void splret_modreg(void)
{
    ca(ADDRESS_OF(MODREG));
    if (A_is_zero()) { /* EXTEND; BZF SPLRET1 */
        tc_to(SPLRET1, INTPRET);
        return;
    }
    v82gon3();
}
#endif

/* V82GON3: "SEE IF ASTRONAUT HAS SIGNALLED TERMINATE" (bit 5 of EXTVBACT
 * clear): "YES, TERMINATE VB 82 LOOP" (BZF ENDEXT); else "WAIT ONE SECOND
 * BEFORE REPEATING ORBITAL PARAMETER COMPUTATION" (BANKCALL, CADR
 * DELAYJOB), */
void v82gon3(void)
{
    set_A(BIT5);
    mask_at(ADDRESS_OF(EXTVBACT)); /* "SEE IF ASTRONAUT HAS SIGNALLED TERMINATE" */
    if (A_is_zero()) {             /* EXTEND; BZF ENDEXT */
        continue_at(ENDEXT);
        return;
    }
    set_A(ONE_SEC);
    tc_to(V82GON3 + 5u, BANKCALL); /* "WAIT ONE SECOND BEFORE REPEATING" */
}

/* and again (TC V82GON1). */
void v82gon3_again(void)
{
    tc_at(V82GON3 + 7u);
    v82gon1();
}

/* ---- SR30.1 -- LM pp. 720-721, CM pp. 522-523 ---- */

/* SR30.1: "INITIALIZE PUSHDOWN LIST" and keep the return in S2. About the
 * moon (V82EMFLG set) RONE and VONE are rescaled ("MOON CENTERED
 * (RESCALING REQUIRED)", VSL2). TFFCONMU, and TFFRP/RA, which "COMPUTES
 * RAPO,RPER" and "RETURNS WITH RAPO IN D(MPAC)". HAPOX: RAPO - RPADTEM ("NEED
 * HAPO AT (-29)M FOR DISPLAY. IF MOON CENTERED, RESCALE FROM (-27)M", SR2R),
 * at most 9999.9 NM (MAXCHK). HPERX the same from RPER, kept in MPAC +4 "FOR
 * COMPARISON TO HPERMIN": "IF HPER L/ HPERMIN (300 OR 35) KFT, THEN ZERO
 * INTO -TPER"; "OTHERWISE CALCULATE TPER" (CALCTPER), "PUT NEG INTO -TPER".
 * TFF from CALCTFF, which "GIVES 59M59S FOR TFF IF RPER G/ HPERMIN +
 * RPADTEM", negated. Then back through S2. */
static const word sr30_1_words[] = {
    ORDERS(SETPD, STQ), /* "INITIALIZE PUSHDOWN LIST." */
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(S2),
    ORDERS(BOFF, VLOAD),
    SWITCH(BOF, V82EMFLG), /* "OFF FOR EARTH, ON FOR MOON." */
    ADDRESS(TFFCALLS),
    ADDRESS_PLUS_ONE(RONE),
    ORDER(VSL2),
    STOVL(RONE),
    ADDRESS_PLUS_ONE(VONE),
    ORDER(VSL2),
    STORE(VONE),
    /* TFFCALLS */ ORDER(CALL),
    ADDRESS(TFFCONMU),
    ORDER(CALL), /* "TFFRP/RA COMPUTES RAPO,RPER." */
    ADDRESS(TFFRP_RA),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(RPADTEM),
    ORDERS(BOFF, SR2R),       /* "NEED HAPO AT (-29)M FOR DISPLAY." */
    SWITCH(BOF, V82EMFLG),    /* "OFF FOR EARTH, ON FOR MOON." */
    ADDRESS_AT(SR30_1, 025u), /* +1 */
    ORDER(CALL),              /* "IF HAPO > MAXNM, SET HAPO =9999.9 NM." */
    ADDRESS(MAXCHK),          /* "OTHERWISE STORE (RAPO-RPADTEM) IN HAPO." */
    /* STORHAPO */ STODL(HAPOX),
    ADDRESS_PLUS_ONE(RPER),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(RPADTEM), /* "GIVES HPER AT (-29)M EARTH, (-27)M MOON." */
    STORE_AT(MPAC, 4u),        /* "SAVE THIS FOR COMPARISON TO HPERMIN." */
    ORDERS(BOFF, SR2R),        /* "NEED HPER AT (-29)M FOR DISPLAY." */
    SWITCH(BOF, V82EMFLG),
    ADDRESS_AT(SR30_1, 037u), /* +1 */
    ORDER(CALL),              /* "IF HPER > MAXNM, SET HPER = 9999.9 NM." */
    ADDRESS(MAXCHK),
    /* STORHPER */ STODL(HPERX), /* "STORE (RPER - RPADTEM) INTO HPERX." */
    ADDRESS_PLUS_ONE_AT(MPAC, 4u),
    ORDERS(DSU, BPL),          /* "HPERMIN AT (-29)M FOR EARTH, (-27)M MOON" */
    ADDRESS_PLUS_ONE(HPERMIN), /* "IF HPER L/ HPERMIN (300 OR 35) KFT," */
    ADDRESS(DOTPER),           /* "THEN ZERO INTO -TPER." */
    ORDERS(DLOAD, GOTO),       /* "OTHERWISE CALCULATE TPER." */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ADDRESS(SKIPTPER),
    /* DOTPER */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(RPER),
    ADDRESS(CALCTPER),
    ORDER(DCOMP), /* "TPER IS PUT NEG INTO -TPER." */
    /* SKIPTPER */ STODL(MTPER),
    ADDRESS_PLUS_ONE(HPERMIN), /* "HPERMIN AT (-29)M FOR EARTH, (-27)M MOON" */
    ORDERS(DAD, CALL),
    ADDRESS_PLUS_ONE(RPADTEM), /* "RPADTEM AT (-29)M FOR EARTH, (-27)M MOON" */
    ADDRESS(CALCTFF),          /* "GIVES 59M59S FOR TFF IF RPER G/" */
    ORDER(DCOMP),              /* "HPERMIN + RPADTEM. (TPER WAS NON ZERO)" */
    STCALL(TFF),               /* "OTHERWISE COMPUTES TFF. (GOTO)" */
    ADDRESS(S2),
    /* MAXCHK */ ORDERS(DSU, BPL), /* "IF C(MPAC) > 9999.9 NM, MPAC = 9999.9 NM" */
    ADDRESS_PLUS_ONE(MAXNM),
    ADDRESS_AT(MAXCHK, 5u), /* +3: "OTHERWISE C(MPAC) = B(MPAC)." */
    ORDERS(DAD, RVQ),
    ADDRESS_PLUS_ONE(MAXNM),
    /* +3 */ ORDERS(DLOAD, RVQ), /* "(USED BY P30 - P37 ALSO)" */
    ADDRESS_PLUS_ONE(MAXNM),
};

const struct interpretive_code V82CALL_CODE = {R30_BANK, IN_BANK(V82CALL) + 1u,
                                               COUNT(v82call_words), v82call_words};
const struct interpretive_code V82GOFF1_CODE = {R30_BANK, IN_BANK(V82GOFF1) + 1u,
                                                COUNT(v82goff1_words), v82goff1_words};
const struct interpretive_code OTHSHIP_CODE = {R30_BANK, IN_BANK(OTHSHIP) + 1u,
                                               COUNT(othship_words), othship_words};
const struct interpretive_code THISSHIP_CODE = {R30_BANK, IN_BANK(THISSHIP) + 1u,
                                                COUNT(thisship_words), thisship_words};
const struct interpretive_code EARTHPAD_CODE = {R30_BANK, IN_BANK(EARTHPAD), COUNT(earthpad_words),
                                                earthpad_words};
const struct interpretive_code TICKTFF_CODE = {R30_BANK, IN_BANK(TICKTFF), COUNT(ticktff_words),
                                               ticktff_words};
const struct interpretive_code V82GON_CODE = {R30_BANK, IN_BANK(V82GON), COUNT(v82gon_words),
                                              v82gon_words};
const struct interpretive_code V82GON1_CODE = {R30_BANK, IN_BANK(V82GON1) + 1u,
                                               COUNT(v82gon1_words), v82gon1_words};
const struct interpretive_code SR30_1_CODE = {R30_BANK, IN_BANK(SR30_1), COUNT(sr30_1_words),
                                              sr30_1_words};
#if !LUMINARY099
const struct interpretive_code SPLRET1_CODE = {R30_BANK, IN_BANK(SPLRET1) + 1u,
                                               COUNT(splret1_words), splret1_words};
const struct interpretive_code CANDEL_CODE = {R30_BANK, IN_BANK(CANDEL) + 1u, COUNT(candel_words),
                                              candel_words};
#endif
