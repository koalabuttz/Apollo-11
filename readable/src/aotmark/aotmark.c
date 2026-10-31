/* SPDX-License-Identifier: MIT */
/* AOTMARK.agc, Luminary099 only (listing pages 244-261): the sighting
 * marks of the alignment programs, P51-P53 and R59. The faithful C is
 * c64-port/src/lm_aotmark.c.
 *
 * How a sighting goes. AOTMARK takes a VAC area for the marks and starts
 * GETDAT as a job. GETDAT shows V01N71 ("DISPLAY DETENT AND STAR CODE")
 * and waits; on proceed the detent picks the AOT's azimuth and elevation
 * (or the COAS's, detent 7), and OPTAXIS works out the mark planes. GETMKS
 * then shows V54 ("MAKE X OR Y MARK"). Each mark button raises MARKRUPT,
 * which keeps the CDU angles and the time in the VAC area (VACSTOR) and
 * starts CHANGEVB to show the next verb: V53 for the Y mark, V52 for the
 * X, V54 again once a pair is made. Up to five pairs. On proceed,
 * MARKCHEX and AVESTAR turn each pair into a star direction and average
 * them (AVEIT) into STARAD +6. On the lunar surface (SURFFLAG) a mark is
 * a single one, with the cursor and spiral readings of V06N79, and
 * SURFSTAR makes the direction.
 *
 * MARKSTAT keeps the state: the VAC area's address in bits 1-9, the marks
 * wanted or made in bits 10 (X) and 11 (Y), bit 12 set while MARKRUPT
 * must not take marks, bit 13 a mark rejected, bit 14 a pair complete.
 *
 * The basic code runs in bank 07, where the binding (src/binding/
 * aotmark_routines.c) runs it: each call out of the file (BANKCALL, ALARM,
 * FINDVAC, NOVAC, WAITLIST, RESUME...) ends a function with Q and Z set,
 * and the word the call returns to is an entry of its own. A TC INTPRET
 * runs into the interpreter's NEWOPS. */
#include "aotmark.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

/* The instruction TS 0, as INDEX adds to it. */
#define TS_0           054000u
#define TS_0_CODE_MASK 076000u

#define COUNT_OF(table) (sizeof(table) / sizeof(table)[0])

static void aotmark_after_ccs(enum sign markstat);
static void mkvac(unsigned area);
static void mkabort(void);
static void dodat(void);
static void code7(void);
static void code1to6(void);
static void coascode(void);
static void getmks(void);
static void getmks_plus3(void);
static void pastit(void);
static void markchex(void);
static void avestar(void);
static void somekey(void);
static void ymkrupt(void);
static void ymkrupt_plus3(void);
static void ymkrupt_in_flight(void);
static void verifymk(void);
static void five_mkalarm_in_flight(void);
static void mkrej_in_flight(void);
static void rejalm(void);
static void surfrej(void);
static void surfstor(void);
static void vacstor(void);
static void vacstor_in_flight(word at);
static void remark(void);
static void surfjob(void);
static void changevb_in_flight(void);
static void dspv6n79(void);
static void surfend(void);
static void surfagan(void);
static void surfagan_plus1(void);
static void after_marktype(word at);
static void marktype_run(void);

/* ---- Helpers ---- */

/* INDEX k; TS n: the AGC adds C(k) to the TS's whole instruction word
 * (indexed_instruction()) and runs the sum. Not modelled, as in the
 * faithful port: a sum that is no longer a TS, or a TS into EBANK, FBANK,
 * Z or BBANK (the index is a VAC area's address or a place in one). */
static bool ts_indexed(word instruction, word index)
{
    word sum = indexed_instruction(instruction, index);
    unsigned to = sum & 01777u;

    if ((sum & TS_0_CODE_MASK) != TS_0 || (to >= EBANK_ADDRESS && to <= BBANK_ADDRESS))
        not_modelled();
    return ts(to);
}

/* INDEX XYMARK; CA <table> -1: the detent's word of one of the AOT's
 * tables (AOTAZ, AOTEL: a word for each detent). Not modelled: a code that
 * runs past erasable memory (above 0366; the crew key 1 to 6 here), where
 * the AGC would read fixed memory and the faithful port wraps around into
 * erasable. */
static void ca_by_detent(unsigned table)
{
    unsigned at = indexed(table - 1u, XYMARK);

    if (at >= 02000u) not_modelled();
    ca(at);
}

/* TC Q: the routine returns to the address in Q (bits 1-12), and Q
 * becomes 3. Not modelled: a Q that is not such an address (bits 13-15
 * set, or below 010). Returns the address. */
static word tc_q(void)
{
    word at = (word)(Q & 07777u);

    if ((Q & 070000u) || Q < 010u) not_modelled();
    Q = 3u;
    Z = at;
    return at;
}

/* ---- Starting the marks ---- */

/* AOTMARK -- LM p. 244.
 * "SEE IF AOTMARK BUSY": MARKSTAT holds the VAC area of a sighting in
 * progress. If so, "MARK SYSTEM BUSY -- DO ALARM": POODOO, alarm 105
 * (the OCT after the TC). Otherwise on to EXTVBCHK. */
void aotmark(void)
{
    inhint();
    aotmark_after_ccs(ccs_at(ADDRESS_OF(MARKSTAT)));
}

/* CCS MARKSTAT's four ways: TC +2 (busy), TC EXTVBCHK (+0), TC POODOO,
 * and the alarm's code, OCT 00105, which -0 would run as TC 105 (never:
 * MARKSTAT is never negative; not modelled). */
static void aotmark_after_ccs(enum sign markstat)
{
    switch (markstat) {
    case POSITIVE:
        tc_at(AOTMARK + 2u); /* TC +2 */
        aotmark_plus4();
        return;
    case IS_PLUS_ZERO: aotmark_plus3(); return;
    case NEGATIVE: aotmark_plus4(); return;
    case IS_MINUS_ZERO: not_modelled(); return;
    }
}

/* AOTMARK +3: TC EXTVBCHK. */
void aotmark_plus3(void)
{
    tc_at(AOTMARK + 3u);
    extvbchk();
}

/* AOTMARK +4: TC POODOO; OCT 00105. */
void aotmark_plus4(void)
{
    tc_at(AOTMARK + 4u);
    continue_at(POODOO);
}

/* EXTVBCHK -- LM p. 244.
 * "SEE IF EXT. VERB WORKING" (EXTVBACT bits 2 and 3): "YES -- ABORT".
 * "NO -- DISALLOW SOME EXTENDED VERB ACTION" (BIT2, "RESET IN ENDMARK"),
 * and find a VAC area. (CCS of a masked word is never negative.) */
void extvbchk(void)
{
    set_A(SIX);
    mask_at(ADDRESS_OF(EXTVBACT));
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        mkabort();
        return;
    }
    set_A(BIT2);
    ads(ADDRESS_OF(EXTVBACT));
    mkvac(1u);
}

/* MKVAC -- LM p. 244.
 * "LOOK FOR A VAC AREA -- DO ABORT IF NONE AVAILABLE": CCS VAC1USE, then
 * VAC2USE ... VAC5USE, from the area given. A free area's word holds its
 * own address, so CCS leaves the address less one in A for MKVACFND; a
 * taken one's +0, and the ladder goes on. Past the last: "ALL VAC AREAS
 * OCCUPIED -- ABORT" (BAILOUT1, 1207).
 *
 * Never, as a VACnUSE is never negative: a negative word goes to MKVACFND
 * too (VAC5USE's to the TC BAILOUT1, past the DXCH), and -0 skips the next
 * area's CCS (VAC5USE's would run the OCT 1207: not modelled). */
static void mkvac(unsigned area)
{
    while (area <= VAC_AREAS) {
        switch (ccs_at(VACNUSE_ADDRESS(area))) {
        case POSITIVE: mkvacfnd(); return;
        case IS_PLUS_ZERO: area += 1u; break;
        case NEGATIVE:
            if (area == VAC_AREAS) {
                tc_at(MKVAC + 013u);
                continue_at(BAILOUT1);
                return;
            }
            mkvacfnd();
            return;
        case IS_MINUS_ZERO:
            if (area == VAC_AREAS) not_modelled();
            area += 2u;
            break;
        }
    }
    dxch(ADDRESS_OF(BUF2));
    tc_at(MKVAC + 013u); /* TC BAILOUT1; OCT 01207 */
    continue_at(BAILOUT1);
}

/* MKVACFND -- LM p. 244.
 * The VAC area's address "IN LOW 9 OF MARKSTAT", +0 in its VACnUSE "TO
 * SHOW VAC OCCUPIED" (INDEX MARKSTAT; TS 0 -1), and FINDVAC starts GETDAT
 * as a job ("SET UP JOB FOR GETDAT"). An overflow in AD TWO (never: an
 * address) would skip the CAF ZERO. */
void mkvacfnd(void)
{
    ad(TWO);
    if (!ts(ADDRESS_OF(MARKSTAT))) set_A(ZERO);
    ts_indexed(TS_0 - 1u, MARKSTAT);
    set_A(PRIO15);
    tc_at(MKVACFND + 6u); /* TC FINDVAC; 2CADR GETDAT */
    continue_at(FINDVAC);
}

/* MKVACFND +011, where FINDVAC returns: back to the caller of AOTMARK,
 * through BANKCALL's SWRETURN. */
void mkvacfnd_after_findvac(void)
{
    relint();
    continue_at(SWRETURN);
}

/* MKABORT -- LM p. 245. "CONFLICT WITH EXTENDED VERB": BAILOUT1, 1211. */
static void mkabort(void)
{
    dxch(ADDRESS_OF(BUF2));
    tc_at(MKABORT + 1u);
    continue_at(BAILOUT1);
}

/* MKRELEAS -- LM p. 245.
 * The end of the marks, as a task ENDMARKS starts: "SET MARKSTAT TO
 * ZERO", "PICK UP VAC AREA ADR" and "SHOW MKVAC AREA AVAILABLE" (INDEX A;
 * TS 0: the area's VACnUSE gets its own address back). Then "GO WAKE UP
 * CALLING JOB" (IBNKCALL GOODEND, A = 1). With no area (+0) the TS 0 is TS
 * A, which only tests A. (CCS of a masked word is never negative.) */
void mkreleas(void)
{
    set_A(ZERO);
    xch(ADDRESS_OF(MARKSTAT));
    mask(LOW9);
    if (ccs_at(A_ADDRESS) == POSITIVE) ts_indexed(TS_0, a_word());
    set_A(ONE);
    tc_at(MKRELEAS + 7u); /* TC IBNKCALL; CADR GOODEND */
    continue_at(IBNKCALL);
}

/* ---- The star: V01N71 and the optic axis ---- */

/* KILLAOT -- LM p. 246. "TERMINATE AOTMARK -- ALLOW EXT VERB": V34 ends
 * the program (GOTOPOOH). */
void killaot(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(EXTVBACT));
    tc_at(KILLAOT + 2u);
    continue_at(GOTOPOOH);
}

/* GETDAT -- LM p. 246.
 * "SET BIT12 TO DISCOURAGE MARKRUPT" ("BIT12 RESET AT GETMARK") and
 * "DISPLAY DETENT AND STAR CODE" (V01N71), waiting for the crew
 * (GOMARKF). */
void getdat(void)
{
    cs(ADDRESS_OF(MARKSTAT));
    mask(BIT12);
    ads(ADDRESS_OF(MARKSTAT));
    set_A(V01N71);
    tc_at(GETDAT + 4u); /* TC BANKCALL; CADR GOMARKF */
    continue_at(BANKCALL);
}

/* GETDAT +6: "V34 -- DOES GOTOPOOH". */
void getdat_after_terminate(void)
{
    killaot();
}

/* GETDAT +7: "V33 -- PROCEED -- USE THIS STAR FOR MARKS". */
void getdat_after_proceed(void)
{
    dodat();
}

/* ENTERDAT -- LM p. 246. "ENTER -- REDISPLAY STAR CODE". */
void enterdat(void)
{
    getdat();
}

/* DODAT -- LM p. 246.
 * "PICK DETENT CODE FROM BITS7-9 OF AOTCODE AND SEE IF CODE 1 TO 6": the
 * code shifted down (MP BIT9) is the detent ("STORE DETENT"). +0 is the
 * "COAS CALIBRATION CODE - NO GOOD HERE": GETDAT again. "SEE IF DETENT 7
 * FOR COAS". */
static void dodat(void)
{
    set_A(HIGH9);
    mask_at(ADDRESS_OF(AOTCODE));
    mp(BIT9);
    ts(ADDRESS_OF(XYMARK));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF GETDAT */
        getdat();
        return;
    }
    ad(NEG7);
    if (A_is_zero()) { /* EXTEND; BZF CODE7 */
        code7();
        return;
    }
    code1to6();
}

/* CODE7 -- LM p. 246.
 * "CODE 7, COAS SIGHTING, GET OPTIC AXIS AZ AND EL OF SIGHTING DEVICE FROM
 * ASTRO" (V06N87). */
static void code7(void)
{
    set_A(V06N87_STAR);
    tc_at(CODE7 + 1u); /* TC BANKCALL; CADR GOMARKF */
    continue_at(BANKCALL);
}

/* CODE7 +3: "V34 -- DOES GOTOPOOH". */
void code7_after_terminate(void)
{
    killaot();
}

/* CODE7 +4: "PROCEED" (TCF +2). "PICK UP AZ AND EL IN SP 25 COMP" and
 * "STORE IN 8D AND 9D OF LOCAL VAC". The COAS is the "BACKUP SYSTEM TO
 * BE USED": "ZERO APPARENT ROTATION". */
void code7_after_proceed(void)
{
    dca(ADDRESS_OF(AZ));
    dxch(indexed(D(8), FIXLOC));
    set_A(ZERO);
    coascode();
}

/* CODE7 +5: "ON ENTER, RECYCLE". */
void code7_after_enter(void)
{
    code7();
}

/* CODE1TO6 -- LM pp. 246-247.
 * "INDEX AOT POSITION BY DET CODE": the detent's elevation and azimuth
 * into 9D and 8D of the work area. The "COMPENSATION FOR APPARENT
 * ROTATION OF AOT FIELD OF VIEW IN LEFT AND RIGHT DETENTS IS STORED IN
 * VAC +10D IN SP PRECISION ONE'S COMPLEMENT": AOTAZ +1 less the azimuth. */
static void code1to6(void)
{
    ca_by_detent(ADDRESS_OF(AOTEL));
    ts(indexed(D(9), FIXLOC));
    ca_by_detent(ADDRESS_OF(AOTAZ));
    ts(indexed(D(8), FIXLOC));
    ca(ADDRESS_OF(AOTAZ) + 1u);
    msu_at(indexed(D(8), FIXLOC)); /* EXTEND; INDEX FIXLOC; MSU 8D */
    coascode();
}

/* COASCODE -- LM p. 247. The "ROT ANGLE" into 10D, and TC INTPRET:
 * "COMPUTE X AND Y PLANE VECTORS" (OPTAXIS_CODE). */
static void coascode(void)
{
    ts(indexed(D(10), FIXLOC));
    tc_at(COASCODE + 2u);
    intpret();
}

/* OPTAXIS -- LM p. 248.
 * "THE OPTAXIS SUBROUTINE COMPUTES THE X AND Y MARK PLANE VECS AND ROTATES
 * THEM THRU THE APPARENT FIELD OF VIEW ROTATION UNIQUE TO AOT. OPTAXIS
 * USES OANB TO COMPUTE THE OPTIC AXIS.
 *	INPUT --	AZIMUTH ANGLE IN SINGLE PREC AT CDU SCALE IN 8D OF JOB VAC
 *			ELEVATION ANGLE IN SINGLE PREC AT CDU SCALE IN 9D OF JOB VAC
 *			ROTATION ANGLE IN SINGLE PREC IS COMP SCALED BY PI IN 10D OF VAC
 *	OUTPUT --	OPTIC AXIS VEC IN NG COORDS IN SCAXIS
 *			X-MARK PLANE 1/4VEC IN NB COORDS AT 18D OF JOB VAC
 *			Y-MARK PLANE 1/4VEC IN NB COORDS AT 12D OF JOB VAC" */
static const word optaxis_words[] = {
    /* OPTAXIS */ ORDER(CALL), /* "GO COMPUTE OA AN X AND Y PLANE VECS" */
    ADDRESS(OANB),
    ORDERS(SLOAD, SR1),         /* "LOAD APP ROTATION IN ONES COMP" */
    ADDRESS_PLUS_ONE_D(10),     /* "RESCALE BY 2PI" */
    ORDERS(PUSH, SIN),          /* "1/2SIN(ROT) 0-1" */
    ORDERS(PDDL, COS),          /* */
    ORDERS(PUSH, VXSC),         /* "1/2COS(ROT) 2-3" */
    ADDRESS_PLUS_ONE_D(18),     /* */
    ORDERS(PDDL, VXSC),         /* "1/4COS(ROT)UYP 4-9" */
    ADDRESS_PLUS_ONE_D(0),      /* */
    ADDRESS_PLUS_ONE_D(24),     /* "1/4SIN(ROT)UXP" */
    ORDERS(BVSU, STADR),        /* "UP 4-9" */
    AFTER_STADR(STODL_D(12)),   /* "YPNB=1/4(COS(ROT)UYP-SIN(ROT)UXP)" */
    ORDERS(VXSC, PDDL),         /* "UP 2-3 UP 0-1 FOR EXCHANGE" */
    ADDRESS_PLUS_ONE_D(24),     /* "1/4COS(ROT)UXP PUSH 0-5" */
    ORDERS(VXSC, VAD),          /* "1/4SIN(ROT)UYP" */
    ADDRESS_PLUS_ONE_D(18),     /* "UP 0-5" */
    ORDER(STADR),               /* */
    AFTER_STADR(STOVL_D(18)),   /* "XPNB=1/4(COS(ROT)UXP+SIN(ROT)UYP)" */
    ADDRESS_PLUS_ONE(LO6ZEROS), /* "INITIALIZE AVE STAR VEC ACCUMULATOR" */
    STORE_AT(STARAD, 6),        /* */
    ORDER(EXIT),
};
const struct interpretive_code OPTAXIS_CODE = {07u, OPTAXIS, COUNT_OF(optaxis_words),
                                               optaxis_words};

/* OPTAXIS +026, after the EXIT: TCF GETMKS. */
void optaxis_after_exit(void)
{
    getmks();
}

/* OANB -- LM p. 249.
 * "THE OANB SUBROUTINE COMPUTES THE OPTIC AXIS OF THE SIGHTING INSTRUMENT
 * FROM AZIMUTH AND ELEVATION INPUT FROM THE ASTRONAUT.
 *	INPUT --	AZIMUTH ANGLE IN SINGLE PREC 2'S COMP IN 8D OF JOB VAC
 *			ELEVATION ANGLE IN SINGLE PREC 2'S COMP IN 9D OF VAC
 *	OUTPUT --	OPTIC AXIS IN NB COORDS. IN SCAXIS
 *			X-PLANE 1/2VEC IN NB COORDS AT 24D OF VAC
 *			Y-PLANE 1/2VEC IN NB COORDS AT 18D OF VAC"
 * In bank 04 (AOTMARK2). The angles are CDU readings; RTB CDULOGIC turns
 * each into a double-precision fraction of a revolution. */
static const word oanb_words[] = {
    /* OANB */ ORDERS(SETPD, STQ),    /* */
    ADDRESS_PLUS_ONE_D(0),            /* */
    ADDRESS(GCTR),                    /* "STORE RETURN" */
    ORDERS(SLOAD, RTB),               /* */
    ADDRESS_PLUS_ONE_D(9),            /* "PICK UP SP ELV" */
    ADDRESS(CDULOGIC),                /* */
    ORDERS(PUSH, COS),                /* */
    ORDERS(PDDL, SIN),                /* "1/2COS(ELV) PD 0-1" */
    ORDER(STADR),                     /* */
    AFTER_STADR(STODL(SCAXIS)),       /* "OAX=1/2SIN(ELV)" */
    ADDRESS_PLUS_ONE_D(8),            /* */
    ORDER(RTB),                       /* */
    ADDRESS(CDULOGIC),                /* */
    ORDERS(PUSH, COS),                /* */
    STORE_D(20),                      /* "STORE UYP(Y) 20-21" */
    ORDERS(PDDL, SIN),                /* "1/2COS(AZ) PD 2-3" */
    ORDERS(PUSH, DCOMP),              /* "PUSH 1/2S IN (AZ) 4-5" */
    STODL_D(22),                      /* "STORE UYP(Z) 22-23" */
    ADDRESS_PLUS_ONE(LO6ZEROS),       /* */
    STODL_D(18),                      /* "STORE UYP(X) 18-19" */
    ORDERS(DMP, SL1),                 /* */
    ADDRESS_PLUS_ONE_D(0),            /* */
    STODL_AT(SCAXIS, 2),              /* "OAY=1/2COS(ELV)SIN(AZ)" */
    ORDERS(DMP, SL1),                 /* "UP 2-3" */
    ORDER(STADR),                     /* "UP 0-1" */
    AFTER_STADR(STOVL_AT(SCAXIS, 4)), /* "OAZ=1/2COS(ELV)COS(AZ)" */
    ADDRESS_PLUS_ONE_D(18),           /* "LOAD UYP VEC" */
    ORDERS(VXV, UNIT),                /* */
    ADDRESS_PLUS_ONE(SCAXIS),         /* "UXP VEC=UYP X OA" */
    STORE_D(24),                      /* "STORE UXP" */
    ORDER(GOTO),                      /* */
    ADDRESS(GCTR),
};
const struct interpretive_code OANB_CODE = {04u, OANB, COUNT_OF(oanb_words), oanb_words};

/* SURFSTAR -- LM p. 250.
 * "SURFSTAR COMPUTES A STAR VECTOR IN SM COORDINATES FOR LUNAR SURFACE
 * ALIGNMENT AND EXITS TO AVEIT TO AVERAGE STAR VECTORS.
 *	GIVEN	X-MARK PLANE 1/4 VEC IN NB AT 18D OF LOCAL VAC
 *		Y-MARK PLANE 1/4 VEC IN NB AT 12D OF LOCAL VAC
 *		CURSOR SP 2COMP AT POSITION 1 OF INDEXED MARKVAC
 *		SPIRAL SP 2COMP AT POSITION 3 OF INDEXED MARKVAC
 *		CDUY,Z,X AT POSITIONS 0,2,4 OF INDEXED MARKVAC"
 * In bank 15 (P50S). X1 holds the mark's place in the VAC area. */
static const word surfstar_words[] = {
    /* SURFSTAR */ ORDER(VLOAD_STAR), /* */
    ADDRESS_PLUS_ONE_X1(0),           /* "PUT X-MARK CDUS IN CDUSPOT FOR TRG*NBSM" */
    STORE(CDUSPOT),                   /* */
    ORDERS(SLOAD_STAR, RTB),          /* */
    ADDRESS_PLUS_ONE_X1(1),           /* "PICK UP YROT" */
    ADDRESS(CDULOGIC),                /* */
    STORE_D(24),                      /* "STORE CURSOR FOR SPIRAL COMP (REVS)" */
    ORDER(BZE),                       /* */
    ADDRESS(YZCHK),                   /* "IF YROT ZERO -- SEE IF SROT ZERO" */
    /* JUSTZY */ ORDERS(PUSH, COS),   /* */
    ORDERS(PDDL, SIN),                /* "1/2COS(YROT) 0-1" */
    ORDERS(VXSC, PDDL),               /* "UP 0-1 1/8SIN(YROT)UXP 0-5" */
    ADDRESS_PLUS_ONE_D(18),           /* */
    ORDERS(VXSC, VSU),                /* "UP 0-5" */
    ADDRESS_PLUS_ONE_D(12),           /* "UYP" */
    ORDERS(UNIT, VXV),                /* */
    ADDRESS_PLUS_ONE(SCAXIS),         /* */
    ORDERS(UNIT, PUSH),               /* */
    ORDERS(SLOAD_STAR, RTB),          /* */
    ADDRESS_PLUS_ONE_X1(3),           /* "PICK UP SPIRAL" */
    ADDRESS(CDULOGIC),                /* */
    STORE_D(26),                      /* "STORE SPIRAL (REVS)" */
    ORDERS(DSU, DAD),                 /* */
    ADDRESS_PLUS_ONE_D(24),           /* */
    ADDRESS_PLUS_ONE(ABOUTONE),       /* */
    ORDER(DMP),                       /* */
    ADDRESS_PLUS_ONE(DP1_12),         /* */
    STORE_D(26),                      /* "SEP=(360 + SPIRAL -CURSOR)/12" */
    ORDERS(SIN, VXSC),                /* "UP 0-5" */
    ORDERS(VSL1, PDDL),               /* "1/2SIN(SEP)(UPP X OA) 0-5" */
    ADDRESS_PLUS_ONE_D(26),           /* */
    ORDERS(COS, VXSC),                /* */
    ADDRESS_PLUS_ONE(SCAXIS),         /* */
    ORDERS(VSL1, VAD),                /* "UP 0-5" */
    /* JUSTOA */ ORDERS(UNIT, CALL),  /* */
    ADDRESS(TRG_STARNBSM),            /* */
    STCALL_D(24),                     /* "STAR VEC IN SM" */
    ADDRESS(AVEIT),                   /* "GO AVERAGE" */
};
const struct interpretive_code SURFSTAR_CODE = {015u, SURFSTAR, COUNT_OF(surfstar_words),
                                                surfstar_words};

/* YZCHK, YSZERO -- LM p. 251. "YROT ZERO AND IF SROT ZERO FORCE STAR ALONG
 * OPTIC AXIS". Back in bank 07. */
static const word yzchk_words[] = {
    /* YZCHK */ ORDERS(SLOAD_STAR, BZE), /* */
    ADDRESS_PLUS_ONE_X1(3),              /* */
    ADDRESS(YSZERO),                     /* */
    ORDERS(DLOAD, GOTO),                 /* */
    ADDRESS_PLUS_ONE_D(24),              /* */
    ADDRESS(JUSTZY),                     /* "SROT NOT ZERO -- CONTINUE NORMALLY" */
    /* YSZERO */ ORDERS(VLOAD, GOTO),    /* */
    ADDRESS_PLUS_ONE(SCAXIS),            /* */
    ADDRESS(JUSTOA),
};
const struct interpretive_code YZCHK_CODE = {07u, YZCHK, COUNT_OF(yzchk_words), yzchk_words};

/* ---- The marks ---- */

/* GETMKS -- LM p. 252.
 * "THE GETMKS ROUTINE INITIALIZES THE SIGHTING MARK PROCEDURE":
 * "INITIALIZE MARK ID REGISTER AND MARK CNT", "ZERO BITS10 TO 15 RETAINING
 * MKVAC ADR", and "DISPLAY VB54 INITIALLY" (PASTIT). */
static void getmks(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(XYMARK));
    ts(ADDRESS_OF(MARKCNTR));
    getmks_plus3();
}

/* GETMKS +3, where a surface recycle comes back (SURFAGAN). */
static void getmks_plus3(void)
{
    set_A(LOW9);
    mask_at(ADDRESS_OF(MARKSTAT));
    ts(ADDRESS_OF(MARKSTAT));
    set_A(MKVB54_STAR);
    pastit();
}

/* PASTIT -- LM p. 252. The mark verb in A, displayed through GOMARK4,
 * which waits for the crew. */
static void pastit(void)
{
    tc_at(PASTIT); /* TC BANKCALL; CADR GOMARK4 */
    continue_at(BANKCALL);
}

/* PASTIT +2: "V34 -- DOES GOTOPOOH". */
void pastit_after_terminate(void)
{
    killaot();
}

/* PASTIT +3: "VB33 -- PROCEED, GOT MARKS, COMPUTE LOS". */
void pastit_after_proceed(void)
{
    markchex();
}

/* PASTIT +4: "ENTER -- RECYCLE TO V01N71". */
void pastit_after_enter(void)
{
    getdat();
}

/* MARKCHEX, CNTCHK -- LM p. 252.
 * "SET BIT12 TO DISCOURAGE MARKRUPT", "JAM MARK VAC ADR IN XYMARK FOR
 * AVESTAR", "SET MKDEX ZERO FOR LOS VEC CNTR". Then "SEE IF LAST MK PART
 * COMPLETE" (bits 10 and 11 both set: "LAST PAIR COMPLETE -- TO COMPUTE
 * LOS"). If not, CNTCHK: "NO PAIR SHOWING -- SEE IF PAIR IN HOLD": a pair
 * made before ("PAIR BURIED -- DECREMENT COUNTER"), or "NO PAIR -- ALARM".
 * (MARKCNTR is never negative: negative would decrement too, -0 go
 * straight to AVESTAR.) */
static void markchex(void)
{
    cs(ADDRESS_OF(MARKSTAT));
    mask(BIT12);
    ads(ADDRESS_OF(MARKSTAT));
    mask(LOW9);
    ts(ADDRESS_OF(XYMARK));
    set_A(ZERO);
    ts(ADDRESS_OF(MKDEX));
    ca(ADDRESS_OF(MARKSTAT));
    mask(PRIO3);
    ts(L_ADDRESS);
    set_A(PRIO3);
    rxor_lchan();      /* EXTEND; RXOR LCHAN */
    if (A_is_zero()) { /* EXTEND; BZF AVESTAR */
        avestar();
        return;
    }
    switch (ccs_at(ADDRESS_OF(MARKCNTR))) { /* CNTCHK */
    case POSITIVE:                          /* TCF +2 */
    case NEGATIVE: ts(ADDRESS_OF(MARKCNTR)); break;
    case IS_PLUS_ZERO: mkalarm(); return;
    case IS_MINUS_ZERO: break;
    }
    avestar();
}

/* AVESTAR -- LM pp. 252-253.
 * One pair's star direction: "INITIALIZE MKDEX FOR STAR LOS COUNTER"
 * (BIT12 more each pair: MKDEX counts them, to average by), and "JAM --
 * CDU ADR OF X-MARK IN X1": the pair's place in the VAC area, six words
 * a pair ("GET C(L) = -6 MARKCNTR"), less the area's address. Then "SET PD
 * POINTER TO ZERO" and into AVESTAR_CODE. (An overflow in AD L would skip
 * the CA FIXLOC.) */
static void avestar(void)
{
    set_A(BIT12);
    ads(ADDRESS_OF(MKDEX));
    cs(ADDRESS_OF(MARKCNTR));
    mp(SIX);
    cs(ADDRESS_OF(XYMARK));
    ad_at(L_ADDRESS);
    if (!ts(indexed(ADDRESS_OF(X1), FIXLOC))) ca(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(PUSHLOC));
    tc_at(AVESTAR + 013u);
    intpret();
}

/* AVESTAR's interpretive code and AVEIT -- LM p. 253.
 * The two mark planes, "CONVERT IT TO STABLE MEMBER" each (TRG*NBSM, with
 * the mark's CDU angles in CDUSPOT), and the star along their cross
 * product. On the surface, SURFSTAR makes the vector instead. AVEIT: the
 * average of N vectors (N in MKDEX) is (N-1)/N times the average so far,
 * plus the new one over N. */
static const word avestar_words[] = {
    ORDERS(BON, VLOAD_STAR),         /* */
    SWITCH(BON, SURFFLAG),           /* "IF ON SURFACE COMPUTE VEC AT SURFSTAR" */
    ADDRESS(SURFSTAR),               /* */
    ADDRESS_PLUS_ONE_X1(1),          /* "PUT Y-MARK CDUS IN CDUSPOT FOR TRG*NBSM" */
    STOVL(CDUSPOT),                  /* */
    ADDRESS_PLUS_ONE_D(12),          /* "LOAD Y-PLANE VECTOR IN NG" */
    ORDER(CALL),                     /* */
    ADDRESS(TRG_STARNBSM),           /* "CONVERT IT TO STABLE MEMBER" */
    ORDERS(PUSH, VLOAD_STAR),        /* */
    ADDRESS_PLUS_ONE_X1(0),          /* "PUT X-MARK CDUS IN CDUSPOT FOR TRG*NBSM" */
    STOVL(CDUSPOT),                  /* */
    ADDRESS_PLUS_ONE_D(18),          /* "LOAD X-PLANE VECTOR IN NB" */
    ORDER(CALL),                     /* */
    ADDRESS(TRG_STARNBSM),           /* "CONVERT IT TO STABLE-MEMBER" */
    ORDERS(VXV, UNIT),               /* "UNIT(XPSM * YPSM)" */
    ORDER(STADR),                    /* */
    AFTER_STADR(STORE_D(24)),        /* */
    /* AVEIT */ ORDERS(SLOAD, PDVL), /* "N(NUMBER OF VECS) IN 0-1" */
    ADDRESS_PLUS_ONE(MKDEX),         /* */
    ADDRESS_PLUS_ONE_D(24),          /* "LOAD CURRENT VECTOR" */
    ORDERS(VSR3, V_SC),              /* */
    ADDRESS_PLUS_ONE_D(0),           /* */
    STODL_D(24),                     /* "VEC/N" */
    ADDRESS_PLUS_ONE_D(0),           /* */
    ORDERS(DSU, DDV),                /* */
    ADDRESS_PLUS_ONE(DP1_8),         /* "(N-1)/N" */
    ORDERS(VXSC, VAD),               /* */
    ADDRESS_PLUS_ONE_AT(STARAD, 6),  /* "ADD VEC TO PREVIOUSLY AVERAGED VECTOR" */
    ADDRESS_PLUS_ONE_D(24),          /* "(N-1)/N AVESTVEC + VEC/N" */
    STORE_AT(STARAD, 6),             /* "AVERAGE STAR VECTOR" */
    STORE(STARSAV2),                 /* */
    ORDER(EXIT),
};
const struct interpretive_code AVESTAR_CODE = {07u, AVESTAR + 014u, COUNT_OF(avestar_words),
                                               avestar_words};

/* AVEIT +017, after the EXIT: "SEE IF ANOTHER MARK PAIR IN MKVAC". "THERE
 * IS -- GO GET IT -- DECREMENT COUNTER" (TCF AVESTAR -1: TS MARKCNTR).
 * ENDMARKS: "NO MORE MARKS -- TERMINATE AOTMARK": MKRELEAS as a task in
 * 50 ms (FIVE), and ENDMARK. (MARKCNTR is never negative: negative would
 * skip the CAF FIVE, -0 the INHINT too.) */
void aveit_after_exit(void)
{
    switch (ccs_at(ADDRESS_OF(MARKCNTR))) {
    case POSITIVE:
        ts(ADDRESS_OF(MARKCNTR));
        avestar();
        return;
    case IS_PLUS_ZERO: set_A(FIVE); /* fall through */
    case NEGATIVE: inhint();        /* fall through */
    case IS_MINUS_ZERO: break;
    }
    tc_at(ENDMARKS + 2u); /* TC WAITLIST; 2CADR MKRELEAS */
    continue_at(WAITLIST);
}

/* ENDMARKS +5, where WAITLIST returns: TC ENDMARK. */
void endmarks_after_waitlist(void)
{
    tc_at(ENDMARKS + 5u);
    continue_at(ENDMARK);
}

/* MKALARM -- LM p. 253. "NOT A PAIR TO PROCESS -- DO GETMKS": alarm 111. */
void mkalarm(void)
{
    tc_at(MKALARM);
    continue_at(ALARM);
}

/* MKALARM +2, where ALARM returns: TCF GETMKS. */
void mkalarm_after_alarm(void)
{
    getmks();
}

/* ---- MARKRUPT: the mark buttons ---- */

/* MARKRUPT -- LM p. 254.
 * "MARKRUPT IS ENTERED FROM INTERRUPT LEAD-INS AND PROCESSES CHANNEL 16
 * CAUSED BY X,Y MARK OR MARK REJECT OR BY THE RATE OF DESCENT SWITCH."
 * "STORE CDUS AND TIME NOW -- THEN SEE IF WE NEED THEM", in ITEMP1-5;
 * BANKRUPT and QRUPT keep the interrupted program's BBANK (in A) and Q.
 * "SEE IF X OR Y MARK OR MKREJECT" (bits 3-5): if not, SOMEKEY. "ARE WE
 * ASKING FOR A MARK" (bit 12 clear): if not, "DON'T WANT MARK OR MKREJECT
 * -- DO NOTHING". An overflow in A or Q (never) makes its TS skip the CA
 * CDUY or the CAF OCT34. */
void markrupt(void)
{
    if (!ts(ADDRESS_OF(BANKRUPT))) ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(ITEMP3));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(ITEMP4));
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(ITEMP5));
    dca(ADDRESS_OF(TIME2)); /* EXTEND; DCA TIME2 */
    dxch(ADDRESS_OF(ITEMP1));
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(QRUPT))) set_A(OCT34);
    rand_channel(NAVKEYIN); /* EXTEND; RAND NAVKEYIN */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: /* TCF +2: "ITS A LIVE ONE -- SEE IF ITS WANTED" */
    case NEGATIVE: set_A(BIT12); break;
    case IS_PLUS_ZERO: somekey(); return; /* "ITS SOME OTHER KEY" */
    case IS_MINUS_ZERO: break;            /* (on with A +0) */
    }
    mask_at(ADDRESS_OF(MARKSTAT));
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* (a masked word: never negative) */
        tc_at(MARKRUPT + 025u);          /* TC RESUME */
        continue_at(RESUME);
        return;
    }
    markrupt_after_resume();
}

/* MARKRUPT +026: "ARE MARKS BEING ACCEPTED" (MARKSTAT's VAC area): "THEY
 * ARE -- WHICH ONE IS IT". "MARKS NOT BEING ACCEPTED -- DO ALARM" (112).
 * (MARKSTAT is never negative: -0 would go on to the TC RESUME, negative
 * run the OCT 112 as TC 112, not modelled.) */
void markrupt_after_resume(void)
{
    switch (ccs_at(ADDRESS_OF(MARKSTAT))) {
    case POSITIVE: findkey(); return;
    case IS_PLUS_ZERO:
        tc_at(MARKRUPT + 030u);
        continue_at(ALARM);
        return;
    case NEGATIVE: not_modelled(); return;
    case IS_MINUS_ZERO: markrupt_after_alarm(); return;
    }
}

/* MARKRUPT +032, where ALARM returns: TC RESUME. */
void markrupt_after_alarm(void)
{
    tc_at(MARKRUPT + 032u);
    continue_at(RESUME);
}

/* FINDKEY's scan, one key: CCS A on the last RAND's bit. Set (positive):
 * the key's routine. Clear (+0): CAF the next bit; EXTEND; RAND NAVKEYIN.
 * A bit of a channel is never negative or -0; negative would skip the CAF
 * (the RAND takes what CCS left), and -0 the EXTEND too: RAND NAVKEYIN
 * unextended is TC 2016 (at), CCS VAC2USE in MKVAC. True when the key is
 * set; false with the next RAND done, or the run gone into MKVAC. */
static bool key_set(word next_bit, word at, bool *into_mkvac)
{
    *into_mkvac = false;
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: return true;
    case IS_PLUS_ZERO: set_A(next_bit); /* fall through */
    case NEGATIVE: rand_channel(NAVKEYIN); return false;
    case IS_MINUS_ZERO: break;
    }
    tc_at(at);
    *into_mkvac = true;
    mkvac(2u);
    return false;
}

/* FINDKEY -- LM pp. 254-255. "SEE IF MARK REJECT", "SEE IF Y MARK", "SEE
 * IF X MARK": channel 16's bits 5, 4 and 3. None: SOMEKEY. */
void findkey(void)
{
    bool into_mkvac;

    set_A(BIT5);
    rand_channel(NAVKEYIN);
    if (key_set(BIT4, FINDKEY + 7u, &into_mkvac)) {
        mkrej(); /* "IT'S A MARK REJECT" */
        return;
    }
    if (!into_mkvac) findkey_plus10();
}

/* FINDKEY +010: the Y mark's bit. */
void findkey_plus10(void)
{
    bool into_mkvac;

    if (key_set(BIT3, FINDKEY + 014u, &into_mkvac)) {
        ymkrupt(); /* "IT'S A Y MARK" */
        return;
    }
    if (!into_mkvac) findkey_plus15();
}

/* FINDKEY +015: the X mark's bit. */
void findkey_plus15(void)
{
    bool into_mkvac;

    if (key_set(OCT140, SOMEKEY + 2u, &into_mkvac)) {
        xmkrupt(); /* "IT'S A X MARK" */
        return;
    }
    if (!into_mkvac) somekey_plus3();
}

/* SOMEKEY -- LM p. 255. "NOT MARK OR MKREJECT -- SEE IF DESCENT BITS"
 * (bits 6 and 7). */
static void somekey(void)
{
    set_A(OCT140);
    rand_channel(NAVKEYIN);
    somekey_plus3();
}

/* SOMEKEY +3: "IF DESCENT BITS", DESCBITS (through POSTJUMP); "IF NO
 * BITS", alarm 113: "NO INBITS IN CHANNEL 16." */
void somekey_plus3(void)
{
    if (A_is_zero()) { /* EXTEND; BZF +3 */
        somekey_after_postjump();
        return;
    }
    tc_at(SOMEKEY + 5u); /* TC POSTJUMP; CADR DESCBITS */
    continue_at(POSTJUMP);
}

/* SOMEKEY +7: TC ALARM; OCT 113. */
void somekey_after_postjump(void)
{
    tc_at(SOMEKEY + 7u);
    continue_at(ALARM);
}

/* SOMEKEY +011, where ALARM returns: TC RESUME. */
void somekey_after_alarm(void)
{
    tc_at(SOMEKEY + 011u);
    continue_at(RESUME);
}

/* XMKRUPT -- LM p. 255. "SET X MARK STORE INDEX TO ZERO" (RUPTREG1), and
 * BIT10 for the mark's identification. */
void xmkrupt(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RUPTREG1));
    set_A(BIT10);
    ymkrupt_plus3(); /* TCF +4 */
}

/* YMKRUPT -- LM p. 255. "SET Y MARK STORE INDEX TO ONE", and BIT11. */
static void ymkrupt(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(RUPTREG1));
    set_A(BIT11);
    ymkrupt_plus3();
}

/* YMKRUPT +3: "SET MARK IDENTIFICATION", and "SEE IF SURFACE MARK". */
static void ymkrupt_plus3(void)
{
    ts(ADDRESS_OF(XYMARK));
    tc_at(YMKRUPT + 4u); /* TC MARKTYPE */
    marktype_run();
}

/* YMKRUPT +5: "SURFACE MARK -- JUST STORE CDUS". */
void ymkrupt_after_marktype(void)
{
    surfstor();
}

/* YMKRUPT +6: in flight. "GOT A MARK -- SEE IF MARK PAIR MADE" (bit 14):
 * "NOT A PAIR, NORMAL PROCEDURE". "GOT A PAIR, SEE IF ANOTHER CAN BE MADE":
 * "HAVE FIVE MARK PAIRS -- DON'T ALLOW MARK"; else "OK FOR ANOTHER PAIR,
 * INCR POINTER" and "CLEAR BITS 10,11,14 FOR NEXT PAIR". */
static void ymkrupt_in_flight(void)
{
    set_A(BIT14);
    mask_at(ADDRESS_OF(MARKSTAT));
    if (A_is_zero()) { /* EXTEND; BZF VERIFYMK */
        verifymk();
        return;
    }
    cs(ADDRESS_OF(MARKCNTR));
    ad(FOUR);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF 5MKALARM */
        five_mkalarm();
        return;
    }
    incr(ADDRESS_OF(MARKCNTR));
    set_A(negate(PRIO23)); /* CS PRIO23 */
    mask_at(ADDRESS_OF(MARKSTAT));
    ts(ADDRESS_OF(MARKSTAT));
    verifymk();
}

/* VERIFYMK -- LM p. 255. Is this mark wanted (its bit, 10 or 11, still
 * clear in MARKSTAT)? "MARK DESIRED -- STORE CDUS"; "THIS MARK NOT
 * DESIRED": alarm 114. (-0 would run the OCT 114 as TC 114: not modelled.) */
static void verifymk(void)
{
    ca(ADDRESS_OF(XYMARK));
    mask_at(ADDRESS_OF(MARKSTAT));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: /* TCF +2 */
    case NEGATIVE:
        tc_at(VERIFYMK + 5u);
        continue_at(ALARM);
        return;
    case IS_PLUS_ZERO: vacstor(); return;
    case IS_MINUS_ZERO: not_modelled(); return;
    }
}

/* VERIFYMK +7: "RESUME -- DISPLAY UNCHANGED -- WAIT FOR ACTION". */
void verifymk_after_alarm(void)
{
    tc_at(VERIFYMK + 7u);
    continue_at(RESUME);
}

/* 5MKALARM -- LM p. 256. "ATTEMPTING TO MAKE MORE THAN 5 MK PAIRS": alarm
 * 107. */
void five_mkalarm(void)
{
    tc_at(FIVE_MKALARM);
    continue_at(ALARM);
}

/* 5MKALARM +2: "SEE IF SURFACE MARK". */
void five_mkalarm_after_alarm(void)
{
    tc_at(FIVE_MKALARM + 2u);
    marktype_run();
}

/* 5MKALARM +3: on the surface ("IT IS"), the V06N79 display again. */
void five_mkalarm_after_marktype(void)
{
    dspv6n79();
}

/* 5MKALARM +4: in flight, "DON'T CHANGE DISPLAY -- DO NOTHING". */
static void five_mkalarm_in_flight(void)
{
    tc_at(FIVE_MKALARM + 4u);
    continue_at(RESUME);
}

/* MKREJ -- LM p. 257. The mark reject button: "SEE IF SURFACE". */
void mkrej(void)
{
    tc_at(MKREJ);
    marktype_run();
}

/* MKREJ +1: "SURFACE -- JUST CHECK MARK COUNTER". */
void mkrej_after_marktype(void)
{
    surfrej();
}

/* MKREJ +2: "INFLIGHT -- SEE IF MARKS MADE" (bits 10-11): "MARKS MADE --
 * REJECT ONE". (A masked word is never negative.) */
static void mkrej_in_flight(void)
{
    set_A(PRIO3);
    mask_at(ADDRESS_OF(MARKSTAT));
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        reject();
        return;
    }
    rejalm();
}

/* REJALM -- LM p. 257. "NO MARK TO REJECT -- BAD PROCEDURE -- ALARM": 115. */
static void rejalm(void)
{
    tc_at(REJALM);
    continue_at(ALARM);
}

/* REJALM +2: "DESIRED ACTION DISPLAYED". */
void rejalm_after_alarm(void)
{
    tc_at(REJALM + 2u);
    continue_at(RESUME);
}

/* REJECT, RENEWMK, REJECT2 -- LM p. 257.
 * "ZERO BIT14, SHOW REJ., SEE IF MARK SINCE LAST REJECT" (bit 13 was
 * clear). "MARK MADE SINCE REJECT -- REJECT MARK IN 1D": clear its bit
 * (XYMARK's). "ANOTHER REJECT SET BIT 10+11 TO ZERO" ("ON SECOND REJECT --
 * DISPLAY VB53 AGAIN"). Either way RENEWMK: "GO REQUEST NEW MARK ACTION".
 * (A masked word is never negative: negative would skip the CS XYMARK, -0
 * the MASK too.) */
void reject(void)
{
    set_A(negate(PRIO30)); /* CS PRIO30 */
    mask_at(ADDRESS_OF(MARKSTAT));
    ad(BIT13);
    xch(ADDRESS_OF(MARKSTAT));
    mask(BIT13);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        set_A(negate(PRIO3)); /* REJECT2: CS PRIO3; TCF RENEWMK */
    else
        cs(ADDRESS_OF(XYMARK));
    mask_at(ADDRESS_OF(MARKSTAT)); /* RENEWMK */
    ts(ADDRESS_OF(MARKSTAT));
    remark();
}

/* SURFREJ -- LM p. 257. "IF MARK DECREMENT COUNTER"; "NO MARKS TO REJECT
 * -- ALARM". (MARKCNTR is never negative: negative would decrement too, -0
 * go straight to the TC RESUME.) */
static void surfrej(void)
{
    switch (ccs_at(ADDRESS_OF(MARKCNTR))) {
    case POSITIVE: /* TCF +2 */
    case NEGATIVE: ts(ADDRESS_OF(MARKCNTR)); break;
    case IS_PLUS_ZERO: rejalm(); return;
    case IS_MINUS_ZERO: break;
    }
    tc_at(SURFREJ + 4u);
    continue_at(RESUME);
}

/* ---- MARKTYPE and storing a mark ---- */

/* The words a TC MARKTYPE returns to, and what runs there. */
static void after_marktype(word at)
{
    switch (at) {
    case YMKRUPT + 5u: ymkrupt_after_marktype(); return;
    case YMKRUPT + 6u: ymkrupt_in_flight(); return;
    case FIVE_MKALARM + 3u: five_mkalarm_after_marktype(); return;
    case FIVE_MKALARM + 4u: five_mkalarm_in_flight(); return;
    case MKREJ + 1u: mkrej_after_marktype(); return;
    case MKREJ + 2u: mkrej_in_flight(); return;
    case VACSTOR + 027u: vacstor_after_marktype(); return;
    case VACSTOR + 030u:
    case VACSTOR + 031u:
    case VACSTOR + 032u: vacstor_in_flight(at); return;
    case CHANGEVB + 1u: changevb_after_marktype(); return;
    case CHANGEVB + 2u: changevb_in_flight(); return;
    default: return; /* (Z holds it) */
    }
}

/* MARKTYPE -- LM p. 258.
 * "MARKTYPE TESTS TO SEE IF LEM ON LUNAR SURFACE. IF IT IS RETURN TO
 * LOC+1": SURFFLAG (bit 8 of FLAGWRD8) set, back to the word after the TC
 * MARKTYPE; in flight, INCR Q, to the word after that. (CCS of one bit is
 * never negative.) Every TC MARKTYPE in the file goes on at its return. */
static void marktype_run(void)
{
    cs(ADDRESS_OF(FLAGWRD8));
    mask(BIT8);
    if (ccs_at(A_ADDRESS) == POSITIVE) incr(Q_ADDRESS);
    after_marktype(tc_q());
}

/* MARKTYPE entered on its own (the word after SURFREJ's TC RESUME), with
 * Q as it finds it: it goes on only to VACSTOR's returns (VACSTOR +027 to
 * +032, where VACSTOR's own TC MARKTYPE returns), and elsewhere stops with
 * Z at Q's address, as the faithful port's routine does. */
void marktype(void)
{
    cs(ADDRESS_OF(FLAGWRD8));
    mask(BIT8);
    if (ccs_at(A_ADDRESS) == POSITIVE) incr(Q_ADDRESS);
    if (Q >= VACSTOR + 027u && Q <= VACSTOR + 032u)
        after_marktype(tc_q());
    else
        tc_q();
}

/* SURFSTOR -- LM p. 258. "FOR SURFACE MARK ZERO MARK KIND INDEX", and "SET
 * BITS10,11 TO SHOW SURFACE MARK FOR MARKCHEX". */
static void surfstor(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RUPTREG1));
    cs(ADDRESS_OF(MARKSTAT));
    mask(PRIO3);
    ads(ADDRESS_OF(MARKSTAT));
    vacstor();
}

/* VACSTOR -- LM pp. 258-259.
 * "STORE MARK VAC ADR IN RUPTREG2", "PICK UP MARKTIME" and "STORE LAST
 * MARK TIME" (TSIGHT). The mark's place: six words a pair ("6 X MARKCNTR
 * FOR STORE INDEX", "GET INDEX FROM LOW ORDER PART"), in the VAC area
 * ("SET CDU STORE INDEX TO MARKVAC"), and one more for a Y mark
 * ("INCREMENT VAC PICKUP BY MARK FOR FLIGHT"), kept in MKDEX too ("STORE
 * HERE IN CASE OF SURFACE MARK"). There, "STORE CDUY", "STORE CDUZ",
 * "STORE CDUX" (every other word). Then "IF SURFACE MARK -- JUST DO
 * SURFJOB". (An overflow in ADS RUPTREG1 would skip the CA ITEMP3.) */
static void vacstor(void)
{
    set_A(LOW9);
    mask_at(ADDRESS_OF(MARKSTAT));
    ts(ADDRESS_OF(RUPTREG2));
    dca(ADDRESS_OF(ITEMP1)); /* EXTEND; DCA ITEMP1 */
    dxch(ADDRESS_OF(TSIGHT));
    ca(ADDRESS_OF(MARKCNTR));
    mp(SIX);
    xch(L_ADDRESS);
    ad_at(ADDRESS_OF(RUPTREG2));
    ads(ADDRESS_OF(RUPTREG1));
    if (!ts(ADDRESS_OF(MKDEX))) ca(ADDRESS_OF(ITEMP3));
    ts_indexed(TS_0, RUPTREG1);
    ca(ADDRESS_OF(ITEMP4));
    ts_indexed(TS_0 + 2u, RUPTREG1);
    ca(ADDRESS_OF(ITEMP5));
    ts_indexed(TS_0 + 4u, RUPTREG1);
    tc_at(VACSTOR + 026u); /* TC MARKTYPE */
    marktype_run();
}

/* VACSTOR +027: the surface mark's TCF SURFJOB. */
void vacstor_after_marktype(void)
{
    surfjob();
}

/* VACSTOR +030 on, in flight (entered at +031 or +032 only through
 * MARKTYPE's own entry). "CLEAR BIT13 TO SHOW MARK MADE", "SET MARK ID IN
 * MARKSTAT" (COM, MASK: clear bit 13 and the mark's bit; AD: set the
 * mark's bit), then "SEE IF X, Y MARK MADE": "NOT PAIR YET, DISPLAY MARK
 * ACTION"; "MARK PAIR COMPLETE -- SET BIT14" and "GO DISPLAY V54". (An
 * overflow in AD XYMARK would skip the MASK PRIO3, which makes the CCS's
 * other two ways: negative skips the CS, -0 the MASK too.) */
static void vacstor_in_flight(word at)
{
    if (at == VACSTOR + 030u) set_A(BIT13);
    if (at <= VACSTOR + 031u) ad_at(ADDRESS_OF(XYMARK));
    com();
    mask_at(ADDRESS_OF(MARKSTAT));
    ad_at(ADDRESS_OF(XYMARK));
    if (!ts(ADDRESS_OF(MARKSTAT))) mask(PRIO3);
    ts(L_ADDRESS);
    set_A(PRIO3); /* CA PRIO3 */
    rxor_lchan(); /* EXTEND; RXOR LCHAN */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: remark(); return;
    case IS_PLUS_ZERO: cs(ADDRESS_OF(MARKSTAT)); /* fall through */
    case NEGATIVE: mask(BIT14);                  /* fall through */
    case IS_MINUS_ZERO: ads(ADDRESS_OF(MARKSTAT)); break;
    }
    remark();
}

/* REMARK -- LM p. 260. The marks made, bits 10 and 11, "SHIFT MARK IDS TO
 * BE 0 TO 3 FOR INDEX": "STORE VERB INDEX" (MKDEX) for CHANGEVB. */
static void remark(void)
{
    set_A(PRIO3);
    mask_at(ADDRESS_OF(MARKSTAT));
    mp(BIT6);
    ts(ADDRESS_OF(MKDEX));
    surfjob();
}

/* SURFJOB -- LM p. 260. "ENTER JOB TO CHANGE DISPLAY TO REQUEST NEXT
 * ACTION": CHANGEVB, through NOVAC. */
static void surfjob(void)
{
    set_A(PRIO15);
    tc_at(SURFJOB + 1u); /* TC NOVAC; 2CADR CHANGEVB */
    continue_at(NOVAC);
}

/* SURFJOB +4, where NOVAC returns: TC RESUME. */
void surfjob_after_novac(void)
{
    tc_at(SURFJOB + 4u);
    continue_at(RESUME);
}

/* CHANGEVB -- LM p. 260. The job: in flight or on the surface? */
void changevb(void)
{
    tc_at(CHANGEVB);
    marktype_run();
}

/* CHANGEVB +1: "SURFACE -- DISPLAY V 06 N 79". */
void changevb_after_marktype(void)
{
    dspv6n79();
}

/* CHANGEVB +2: "INFLIGHT -- PICK UP MARK VB INDEX" and "PASTE UP NEXT MK
 * VERB DISPLAY": MKVB54, MKVB53, MKVB52 or MKVB54* by the marks made
 * ("THE FOUR MKVBS ARE INDEXED -- THEIR ORDER CANNOT BE CHANGED"). Not
 * modelled: an MKDEX past the four (REMARK makes it 0 to 3). */
static void changevb_in_flight(void)
{
    word at = indexed(ADDRESS_OF(MKVB54), MKDEX);

    if (at < ADDRESS_OF(MKVB54) || at > ADDRESS_OF(MKVB54) + 3u) not_modelled();
    set_A(fetch_fixed(at)); /* INDEX MKDEX; CAF MKVB54 */
    tc_at(CHANGEVB + 4u);   /* TC PASTIT */
    pastit();
}

/* ---- The surface: cursor and spiral ---- */

/* DSPV6N79 -- LM p. 261. "ROUTINE TO REQUEST CURSOR AND SPIRAL
 * MEASUREMENTS": "CURSOR -- SPIRAL DISPLAY", waiting for the crew. */
static void dspv6n79(void)
{
    set_A(V06N79_STAR);
    tc_at(DSPV6N79 + 1u); /* TC BANKCALL; CADR GOMARKF */
    continue_at(BANKCALL);
}

/* DSPV6N79 +3: "V34 -- DOES GOTOPOOH". */
void dspv6n79_after_terminate(void)
{
    killaot();
}

/* DSPV6N79 +4: "V33 -- PROCEED, END MARKING". */
void dspv6n79_after_proceed(void)
{
    surfend();
}

/* DSPV6N79 +5: "IF V32(OCT40) IN MPAC DO RECYCLE OTHERWISE IT IS LOAD VB
 * ENTER SO RE-DISPLAY V06N79". (A masked word is never negative.) */
void dspv6n79_after_enter(void)
{
    set_A(BIT6);
    mask_at(ADDRESS_OF(MPAC));
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        surfagan(); /* "VB32 -- RECYCLE" */
        return;
    }
    dspv6n79(); /* "ENTER" */
}

/* SURFEND -- LM p. 261. "SET BIT14 TO SHOW MARK END". (An overflow in AD
 * BIT14 would skip SURFAGAN's CA CURSOR.) */
static void surfend(void)
{
    set_A(negate(BIT14)); /* CS BIT14 */
    mask_at(ADDRESS_OF(MARKSTAT));
    ad(BIT14);
    if (ts(ADDRESS_OF(MARKSTAT))) {
        surfagan_plus1();
        return;
    }
    surfagan();
}

/* SURFAGAN -- LM p. 261. "STORE CURSOR SP 2COMP" and "STORE SPIRAL" beside
 * the mark's CDU angles (MKDEX "HOLDS VAC AREA POINTER FOR SURF
 * MARKING"). "IF BIT 14 SET -- END MARKING" (MARKCHEX). Else "THIS IS
 * RECYCLE -- SEE IF 5 MARKS ALREADY": "CAN'T RECYCLE -- TOO MANY MARKS --
 * ALARM"; "OK FOR RECYCLE -- INCR COUNTER" and "GO DISPLAY MARK VB". */
static void surfagan(void)
{
    ca(ADDRESS_OF(CURSOR));
    surfagan_plus1();
}

static void surfagan_plus1(void)
{
    ts_indexed(TS_0 + 1u, MKDEX);
    ca(ADDRESS_OF(SPIRAL));
    ts_indexed(TS_0 + 3u, MKDEX);
    cs(ADDRESS_OF(MARKSTAT));
    mask(BIT14);
    if (A_is_zero()) { /* EXTEND; BZF MARKCHEX */
        markchex();
        return;
    }
    ca(ADDRESS_OF(MARKCNTR));
    ad(ONE);
    com();
    ad(FIVE);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF 5MKALARM */
        five_mkalarm();
        return;
    }
    incr(ADDRESS_OF(MARKCNTR));
    getmks_plus3();
}

#endif
