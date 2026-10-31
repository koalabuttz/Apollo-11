/* SPDX-License-Identifier: MIT */
/* R63.agc (Luminary099 pages 338-341): Verb 89, the rendezvous final
 * attitude. Its interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/r63_routines.c), and its basic code. The faithful C is
 * c64-port/src/lm_r63.c.
 *
 * The steps, as the listing's header numbers them: "1. KEY IN V 89 E
 * ONLY IF IN PROG 00" (EXTENDED_VERBS's V89PERF checks that, CHKPOOH, and
 * starts V89CALL as a job); "2. ... DO IMU STATUS CHECK ROUTINE
 * (R02BOTH)"; "3. FLASH DISPLAY V 04 N 12. R2 INDICATES WHICH SPACECRAFT
 * AXIS IS TO BE POINTED AT CSM. INITIAL CHOICE IS PREFERRED (+Z) AXIS
 * (R2=1)" (the header says N 06; the code flashes N 12); "4. BOTH VEHICLE
 * STATE VECTORS UPDATED BY CONIC EQS."; "5. HALF MAGNITUDE UNIT LOS VECTOR
 * (IN STABLE MEMBER COORDINATES) AND HALF MAGNITUDE UNIT SPACECRAFT AXIS
 * VECTOR (IN BODY COORDINATES) PREPARED FOR VECPOINT"; "6. GIMBAL ANGLES
 * FROM VECPOINT TRANSFORMED INTO FDAI BALL ANGLES BY BALLANGS. FLASH
 * DISPLAY V 06 N 18 AND AWAIT RESPONSE"; "7. RECYCLE - RETURN TO STEP 4.
 * TERMINATE - EXIT R63. PROCEED - RESET 3AXISFLG AND CALL R60LEM FOR
 * ATTITUDE MANEUVER."
 *
 * Each call out of the file (BANKCALL, DOWNFLAG, ENDEXT) leaves Z at the
 * routine and Q at the word after the TC, where the routine comes back
 * to; GOFLASH comes back to one of the three words after its CADR, by the
 * astronaut's answer: terminate, proceed or enter (data in). */
#include "r63.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* The words of the basic code the C names. */
#define TC_BANKCALL_GOFLASH_V0412 (V89CALL + 7u)    /* TC BANKCALL; CADR GOFLASH */
#define TC_ENDEXT_V0412           (V89CALL + 011u)  /* TC ENDEXT "TERMINATE" */
#define TC_PLUS2_V0412            (V89CALL + 012u)  /* TC +2 "PROCEED" */
#define TC_MINUS5_V0412           (V89CALL + 013u)  /* TC -5 "DATA IN" */
#define TC_BANKCALL_BALLANGS      (V89CALL1 + 4u)   /* TC BANKCALL; CADR BALLANGS */
#define TC_BANKCALL_GOFLASH_V0618 (V89CALL1 + 7u)   /* TC BANKCALL; CADR GOFLASH */
#define TC_ENDEXT_V0618           (V89CALL1 + 011u) /* TC ENDEXT "TERMINATE" */
#define TC_PLUS2_V0618            (V89CALL1 + 012u) /* TC +2 "PROCEED" */
#define TC_V89RECL                (V89CALL1 + 013u) /* TC V89RECL "RECYCLE" */
#define TC_DOWNFLAG_3AXISFLG      (V89CALL1 + 014u) /* TC DOWNFLAG; ADRES 3AXISFLG */
#define TC_BANKCALL_R60LEM        (V89CALL1 + 016u) /* TC BANKCALL; CADR R60LEM */

/* V89CALL -- LM p. 339.
 * "IMU STATUS CHECK. RETURNS IF ORIENTATION KNOWN. ALARMS IF NOT."
 * (R02BOTH, through BANKCALL.) */
void v89call(void)
{
    tc_at(V89CALL);
    continue_at(BANKCALL); /* CADR R02BOTH */
}

/* V89CALL +7: flash V04 N12, the axis to choose (OPTION1 and OPTION2,
 * OPTIONX and OPTIONX +1). */
static void flash_v04n12(void)
{
    set_A(VB04N12);
    tc_at(TC_BANKCALL_GOFLASH_V0412);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* V89CALL +2 -- LM p. 339. After R02BOTH: "ALLOW ASTRONAUT TO SELECT
 * DESIRED TRACKING ATTITUDE AXIS." OPTION1 (OPTIONX) gets 3, OPTION2 (+1)
 * 1: the +Z axis offered first. */
void v89call_after_r02both(void)
{
    set_A(THREE);
    ts(ADDRESS_OF(OPTIONX));
    set_A(ONE);
    ts(ADDRESS_OF(OPTIONX) + 1u);
    flash_v04n12(); /* "V 04 N 12" */
}

/* V89CALL +011 -- LM p. 339. "TERMINATE". */
void v89call_after_terminate(void)
{
    tc_at(TC_ENDEXT_V0412);
    continue_at(ENDEXT);
}

/* V89CALL +012 -- LM p. 339. "PROCEED": TC +2, to V89RECL. */
void v89call_after_proceed(void)
{
    tc_at(TC_PLUS2_V0412);
    v89recl();
}

/* V89CALL +013 -- LM p. 339. "DATA IN. OPTION1+1 = 1 FOR Z AXIS, = 2 FOR
 * X AXIS": the astronaut loaded R2; TC -5 flashes V04 N12 again. */
void v89call_after_data_in(void)
{
    tc_at(TC_MINUS5_V0412);
    flash_v04n12();
}

/* V89RECL -- LM p. 339. Where RECYCLE comes back to: into the
 * interpreter, for V89RECL_CODE. */
void v89recl(void)
{
    tc_at(V89RECL);
    intpret();
}

/* V89RECL +1 -- LM p. 339.
 * Both state vectors to a minute from now. The time ("READ PRESENT TIME":
 * RTB LOADTIME, TIME2 and TIME1 to MPAC) plus DP1MIN (6000 centiseconds)
 * is kept in TSTART82 "FOR LEMCONIC CALL" and stored in TDEC1, the time
 * CSMCONIC brings the CSM's state vector to; it leaves the position in
 * RATT, saved in RONE. The same time again for LEMCONIC, the LM's. "CSM
 * POSITION - LEM POSITION = LOS": the line of sight from the LM to the
 * CSM, in reference coordinates; "(REFSMAT X LOS). TRANSFORMS LOS FROM
 * REFERENCE COORD TO STAB MEMB COORD", and NORMUNIT makes it a half unit
 * vector, POINTVSM: "LOS FOR VECPOINT CALCULATION". */
static const word v89recl_words[] = {
    ORDERS(RTB, DAD),
    ADDRESS(LOADTIME), /* "READ PRESENT TIME" */
    ADDRESS_PLUS_ONE(DP1MIN),
    STORE(TSTART82),   /* "SAVE TIME FOR LEMCONIC CALL" */
    STCALL(TDEC1),     /* "STORE TIME FOR CSMCONIC CALL" */
    ADDRESS(CSMCONIC), /* "CSM STATE VECTOR UPDATE" */
    ORDER(VLOAD),      /* "CSMCONIC LEFT R VECTOR IN RATT" */
    ADDRESS_PLUS_ONE(RATT),
    STODL(RONE), /* "SAVE FOR LINE OF SIGHT (LOS) COMPUTATION" */
    ADDRESS_PLUS_ONE(TSTART82),
    STCALL(TDEC1),          /* "STORE TIME FOR LEMCONIC CALL" */
    ADDRESS(LEMCONIC),      /* "LEM STATE VECTOR UPDATE" */
    ORDERS(VLOAD, VSU),     /* "CSM POSITION - LEM POSITION = LOS" */
    ADDRESS_PLUS_ONE(RONE), /* "LOS VECTOR LEFT IN MPAC" */
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(MXV, RTB),           /* "(REFSMAT X LOS). TRANSFORMS LOS FROM REFERENCE */
    ADDRESS_PLUS_ONE(REFSMMAT), /* COORD TO STAB MEMB COORD." */
    ADDRESS(NORMUNIT),
    STORE(POINTVSM), /* "STORE LOS FOR VECPOINT CALCULATION" */
    ORDER(EXIT),
};

/* V89RECL +025 -- LM pp. 339-340. After the EXIT: which axis. "1 FOR Z
 * AXIS. 2 FOR X AXIS.": -OPTION2 + 1 is zero (BZF) for 1, ALINEZ; for any
 * other answer ALINEX. Each goes into the interpreter. */
void v89recl_after_exit(void)
{
    cs(ADDRESS_OF(OPTIONX) + 1u);
    ad(ONE);
    if (A_is_zero()) { /* EXTEND; BZF ALINEZ */
        tc_at(ALINEZ);
    } else {
        tc_at(ALINEX); /* "X AXIS ALIGNMENT" */
    }
    intpret();
}

/* ALINEX, V89CALL1 -- LM pp. 339-340.
 * The axis to point (".5, 0, 0": "READ (.5, 0, 0)", UNITX) to SCAXIS,
 * "STORE SELECTED ALIGNMENT AXIS"; VECPOINT "PUTS DESIRED GIM ANG
 * (OG,IG,MG) IN TMPAC", the gimbal angles that point it along POINTVSM,
 * and they go to CPHI (CPHI, CTHETA, CPSI) "FOR BALLANGS CALL". ALINEZ
 * comes in at V89CALL1 with UNITZ. */
static const word alinex_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNITX),       /* "READ (.5, 0, 0)" */
    /* V89CALL1 */ STCALL(SCAXIS), /* "STORE SELECTED ALIGNMENT AXIS" */
    ADDRESS(VECPOINT),             /* "PUTS DESIRED GIM ANG (OG,IG,MG) IN TMPAC" */
    STORE(CPHI),                   /* "STORE GIMBAL ANGLES FOR BALLANGS CALL." */
    ORDER(EXIT),
};

/* V89CALL1 +4 -- LM p. 340. After the EXIT: BALLANGS "PUTS DESIRED BALL
 * ANGLE IN FDAIX,Y,Z". */
void v89call1_after_exit(void)
{
    tc_at(TC_BANKCALL_BALLANGS);
    continue_at(BANKCALL); /* CADR BALLANGS */
}

/* V89CALL1 +6 -- LM p. 340. "V 06 N 18": "NOUN 18 REFERS TO FDAIX,Y,Z",
 * flashed for the crew's answer. */
void v89call1_after_ballangs(void)
{
    set_A(VB06N18);
    tc_at(TC_BANKCALL_GOFLASH_V0618);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* V89CALL1 +011 -- LM p. 340. "TERMINATE". */
void v89call1_after_terminate(void)
{
    tc_at(TC_ENDEXT_V0618);
    continue_at(ENDEXT);
}

/* V89CALL1 +012 -- LM p. 340. "PROCEED": TC +2, on to clear the flag. */
void v89call1_after_proceed(void)
{
    tc_at(TC_PLUS2_V0618);
    v89call1_after_v89recl();
}

/* V89CALL1 +013 -- LM p. 340. "RECYCLE": TC V89RECL, the state vectors
 * and the angles again. */
void v89call1_after_recycle(void)
{
    tc_at(TC_V89RECL);
    v89recl();
}

/* V89CALL1 +014 -- LM p. 340. "RESET 3 AXIS FLAG" ("RESET BIT6 FLAG WORD
 * 5"): DOWNFLAG reads its flag from the word after the TC. */
void v89call1_after_v89recl(void)
{
    tc_at(TC_DOWNFLAG_3AXISFLG);
    continue_at(DOWNFLAG); /* ADRES 3AXISFLG */
}

/* V89CALL1 +016 -- LM p. 340. R60LEM "PERFORMS LEM MANEUVER TO ALIGN
 * SELECTED SPACECRAFT AXIS TO CSM." */
void v89call1_after_downflag(void)
{
    tc_at(TC_BANKCALL_R60LEM);
    continue_at(BANKCALL); /* CADR R60LEM */
}

/* V89CALL1 +020 -- LM p. 340. "TERMINATE R63". */
void v89call1_after_r60lem(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* ALINEZ -- LM p. 340. "Z AXIS ALIGNMENT": (0, 0, .5), UNITZ, and on at
 * V89CALL1. */
static const word alinez_words[] = {
    ORDERS(VLOAD, GOTO),
    ADDRESS_PLUS_ONE(UNITZ), /* "READ (0, 0, .5)" */
    ADDRESS(V89CALL1),
};

const struct interpretive_code V89RECL_CODE = {026u, V89RECL + 1u, COUNT(v89recl_words),
                                               v89recl_words};
const struct interpretive_code ALINEX_CODE = {026u, ALINEX + 1u, COUNT(alinex_words), alinex_words};
const struct interpretive_code ALINEZ_CODE = {026u, ALINEZ + 1u, COUNT(alinez_words), alinez_words};

#endif
