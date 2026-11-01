/* SPDX-License-Identifier: MIT */
/* S-BAND_ANTENNA_FOR_LM.agc (Luminary099 pp. 486-489) and
 * S-BAND_ANTENNA_FOR_CM.agc (Comanche055 pp. 934-935). R05 "COMPUTES AND
 * DISPLAYS THE PITCH AND YAW ANTENNA GIMBAL ANGLES REQUIRED TO POINT THE
 * LM STEERABLE ANTENNA TOWARD THE CENTER OF THE EARTH" (the LM's header;
 * the CM's routine does the same for the CM's antenna). The crew keys
 * verb 64: EXTENDED_VERBS starts SBANDANT as a job.
 *
 * R02BOTH checks the IMU; then, interpretive, the spacecraft's position
 * now (LOADTIME, and LEMCONIC or CSMCONIC "USING CONIC INTEGRATION"), the
 * direction to the earth (from the moon's sphere through LUNPOS), in
 * stable member axes (REFSMMAT) and in navigation base axes (*SMNB*, after
 * CDUTRIG), and the two angles from it. The display (verb 06 noun 51,
 * through GOMARKFR) shows them, and recycle works them out again, while
 * EXTVBACT's bit 5 says the extended verb is still on.
 *
 * Its interpretive code is in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/sband_antenna_routines.c), and its basic code is below. The
 * faithful C is c64-port/src/lm_sband_antenna.c and cm_sband_antenna.c.
 *
 * Each call out of the file (BANKCALL, PRIOCHNG, BLANKET) leaves Z at the
 * routine and Q at the word after the TC, where the routine comes back
 * to; GOMARKFR comes back to one of the four words after its CADR, by the
 * astronaut's answer (terminate, proceed or recycle) or at once. ENDEXT,
 * B5OFF and ENDOFJOB don't come back. */
#include "sband_antenna.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* SBANDANT -- LM p. 486, CM p. 934. "CHECK IF IMU IS ON AND ALIGNED"
 * (R02BOTH, through BANKCALL). */
void sbandant(void)
{
    tc_at(SBANDANT);
    continue_at(BANKCALL); /* CADR R02BOTH */
}

/* SBANDANT +2 -- into the interpreter, at SBANDANT +3 (SBAND_CODE). */
void sbandant_after_r02both(void)
{
    tc_at(SBANDANT + 2u);
    intpret();
}

#if LUMINARY099

/* The words of the basic code after the EXIT. */
#define AFTER_EXIT           (SBANDEX + 1u)
#define TC_PRIOCHNG_PRIO5    (SBANDEX + 6u)   /* TC PRIOCHNG */
#define TC_BANKCALL_GOMARKFR (SBANDEX + 010u) /* TC BANKCALL; CADR GOMARKFR */
#define TC_B5OFF_TERMINATE   (SBANDEX + 012u) /* TC B5OFF "TERMINATE" */
#define TC_B5OFF_PROCEED     (SBANDEX + 013u) /* TC B5OFF "PROCEED" */
#define TC_ENDOFJOB_RECYCLE  (SBANDEX + 014u) /* TC ENDOFJOB "RECYCLE" */
#define TC_BLANKET           (SBANDEX + 016u)
#define TC_PRIOCHNG_PRIO4    (SBANDEX + 020u) /* TC PRIOCHNG */
#define TC_SBANDANT_PLUS2    (SBANDEX + 021u) /* TC SBANDANT +2 */

/* SBANDANT +3 -- LM pp. 487-488. The time now (RTB LOADTIME) in TDEC1,
 * and LEMCONIC takes the LM's state there: RATT, from the earth (X2 = 0)
 * or the moon (X2 = 2). In the earth's sphere, "UE = -UNIT(RATT)" (CONV4);
 * in the moon's, "UE = -UNIT((REM)(UEM) + RL)": LUNPOS gives the "UNIT
 * POSITION VECTOR FROM EARTH TO MOON", VMOON, times REMDIST, the "MEAN
 * DISTANCE FROM EARTH TO MOON", plus the LM's position from the moon.
 * REFSMMAT turns UE into the stable member's axes, *SMNB* (with CDUTRIG's
 * sines and cosines of the CDU angles) into the navigation base's, and
 * the NBSA matrix (1/SQRT(2): a turn of 45 degrees about Z) into the
 * antenna's: R.
 *
 * Then the angles. URP, the unit "PROJECTION OF R ONTO LM XZ PLANE" (none,
 * if R lies along Y: "EXIT ON OVERFLOW", the angles 0). Pitch is the angle
 * whose sine is the length of "UZ X URP", signed as its Y part:
 * "ASIN((SGN(X.UY))ABV(X))", from -90 to +90 degrees when URP.UZ is
 * positive, else 180 degrees less it (HIDPHALF - PITCHANG). Yaw is the
 * angle out of that plane: Z = "(UR X URP)" dotted with "(UX COS ALPHA) -
 * (UZ SIN ALPHA)", alpha the pitch, its ASIN. PITCHANG and YAWANG are
 * revolutions, B0. */
static const word sband_words[] = {
    ORDERS(SETPD, RTB),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(LOADTIME), /* "PICK UP CURRENT TIME" */
    STCALL(TDEC1),     /* "ADVANCE INTEGRATION TO TIME IN TDEC1 */
    ADDRESS(LEMCONIC), /* USING CONIC INTEGRATION" */
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(X2), /* "X2 =0 EARTH SPHERE, X2 =2 MOON SPHERE" */
    ADDRESS(CONV4),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STODL(RLM),
    ADDRESS_PLUS_ONE(TAT),
    /* CONV3 */ ORDER(CALL),
    ADDRESS(LUNPOS), /* "UNIT POSITION VECTOR FROM EARTH TO MOON" */
    ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(VMOON),
    ADDRESS_PLUS_ONE(REMDIST), /* "MEAN DISTANCE FROM EARTH TO MOON" */
    ORDERS(VSL1, VAD),
    ADDRESS_PLUS_ONE(RLM),
    ORDER(GOTO),
    ADDRESS(CONV5),
    /* CONV4 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),          /* "UE = -UNIT(RATT)  EARTH SPHERE" */
    /* CONV5 */ ORDERS(SETPD, UNIT), /* "UE = -UNIT((REM)(UEM) + RL)  MOON SPHERE" */
    ADDRESS_PLUS_ONE_D(0),           /* "SET PL POINTER TO 0" */
    ORDERS(VCOMP, CALL),
    ADDRESS(CDUTRIG),           /* "COMPUTE SINES AND COSINES OF CDU ANGLES" */
    ORDERS(MXV, VSL1),          /* "TRANSFORM REF. COORDINATE SYSTEM TO */
    ADDRESS_PLUS_ONE(REFSMMAT), /* STABLE MEMBER  B-1 X B-1 X B+1 = B-1" */
    ORDERS(PUSH, DLOAD),        /* "8D" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STORE(PITCHANG),
    STOVL(YAWANG), /* "ZERO OUT ANGLES" */
    ORDER(CALL),
    ADDRESS(STAR_SMNB_STAR), /* *SMNB* */
    STODL(RLM),              /* "PRE-MULTIPLY RLM BY (NBSA) MATRIX(B0)" */
    ADDRESS_PLUS_ONE_AT(RLM, 2),
    ORDERS(PUSH, DSU),
    ADDRESS_PLUS_ONE(RLM),
    ORDER(DMP),
    ADDRESS_PLUS_ONE(1OVSQRT2),
    STODL_AT(RLM, 2),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(RLM),
    ADDRESS_PLUS_ONE(1OVSQRT2),
    STOVL(RLM), /* "R  B-1" */
    ADDRESS_PLUS_ONE(RLM),
    ORDERS(UNIT, PDVL),
    ADDRESS_PLUS_ONE(RLM),
    ORDERS(VPROJ, VSL2), /* "PROJECTION OF R ONTO LM XZ PLANE" */
    ADDRESS_PLUS_ONE(HIUNITY),
    ORDERS(BVSU, BOV), /* "CLEAR OVERFLOW INDICATOR IF ON" */
    ADDRESS_PLUS_ONE(RLM),
    ADDRESS(COVCNV),
    /* COVCNV */ ORDERS(UNIT, BOV), /* "EXIT ON OVERFLOW" */
    ADDRESS(SBANDEX),
    ORDERS(PUSH, VXV), /* "URP VECTOR  B-1" */
    ADDRESS_PLUS_ONE(HIUNITZ),
    ORDERS(VSL1, VCOMP), /* "UZ X URP = -(URP X UZ)" */
    STORE(RLM),          /* "X VEC  B-1" */
    ORDERS(DOT, PDVL),   /* "SGN(X.UY) UNSCALED" */
    ADDRESS_PLUS_ONE(HIUNITY),
    ADDRESS_PLUS_ONE(RLM),
    ORDERS(ABVAL, SIGN),
    ORDER(ASIN), /* "ASIN((SGN(X.UY))ABV(X)) REV B0" */
    STOVL(PITCHANG),
    ADDRESS_PLUS_ONE(URP),
    ORDERS(DOT, BPL),
    ADDRESS_PLUS_ONE(HIUNITZ),
    ADDRESS(NOADJUST), /* "YES, -90 TO +90" */
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(HIDPHALF),
    ADDRESS_PLUS_ONE(PITCHANG),
    STORE(PITCHANG),
    /* NOADJUST */ ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(UR), /* "Z = (UR X URP)" */
    ADDRESS_PLUS_ONE(URP),
    ORDER(VSL1),
    STODL(RLM), /* "Z VEC  B-1" */
    ADDRESS_PLUS_ONE(PITCHANG),
    ORDERS(SIN, VXSC),
    ADDRESS_PLUS_ONE(HIUNITZ),
    ORDERS(PDDL, COS),
    ADDRESS_PLUS_ONE(PITCHANG),
    ORDERS(VXSC, VSU),
    ADDRESS_PLUS_ONE(HIUNITX), /* "(UX COS ALPHA) - (UZ SIN ALPHA)" */
    ORDERS(DOT, PDVL),         /* "YAW.Z" */
    ADDRESS_PLUS_ONE(RLM),
    ADDRESS_PLUS_ONE(RLM),
    ORDERS(ABVAL, SIGN),
    ORDER(ASIN),
    STORE(YAWANG),
    /* SBANDEX */ ORDER(EXIT),
};

/* SBANDEX +1 -- LM pp. 488-489. "IS BIT5 STILL ON" in EXTVBACT: if not,
 * the crew has answered, and ENDEXT ends the verb; if so, the job's
 * priority 5 for the display. */
void sbandant_after_exit(void)
{
    ca(ADDRESS_OF(EXTVBACT));
    mask(BIT5);
    if (A_is_zero()) { /* EXTEND; BZF ENDEXT: "NO" */
        continue_at(ENDEXT);
        return;
    }
    set_A(PRIO5);
    tc_at(TC_PRIOCHNG_PRIO5);
    continue_at(PRIOCHNG);
}

/* SBANDEX +7 -- "DISPLAY ANGLES": verb 06 noun 51, through GOMARKFR. */
void sbandant_after_prio5(void)
{
    set_A(V06N51);
    tc_at(TC_BANKCALL_GOMARKFR);
    continue_at(BANKCALL); /* CADR GOMARKFR */
}

/* SBANDEX +012 -- "TERMINATE". */
void sbandant_terminate(void)
{
    tc_at(TC_B5OFF_TERMINATE);
    continue_at(B5OFF);
}

/* SBANDEX +013 -- "PROCEED". */
void sbandant_proceed(void)
{
    tc_at(TC_B5OFF_PROCEED);
    continue_at(B5OFF);
}

/* SBANDEX +014 -- "RECYCLE": the job ends; the display's recycle starts
 * it again. */
void sbandant_recycle(void)
{
    tc_at(TC_ENDOFJOB_RECYCLE);
    continue_at(ENDOFJOB);
}

/* SBANDEX +015 -- GOMARKFR's "IMMEDIATE RETURN": "BLANK R3". */
void sbandant_immediate_return(void)
{
    set_A(BIT3);
    tc_at(TC_BLANKET);
    continue_at(BLANKET);
}

/* SBANDEX +017 -- back to priority 4. */
void sbandant_after_blanket(void)
{
    set_A(PRIO4);
    tc_at(TC_PRIOCHNG_PRIO4);
    continue_at(PRIOCHNG);
}

/* SBANDEX +021 -- "YES, CONTINUE DISPLAYING ANGLES": the angles again,
 * from SBANDANT +2. */
void sbandant_after_prio4(void)
{
    tc_at(TC_SBANDANT_PLUS2);
    sbandant_after_r02both();
}

#else

/* The words of the basic code after the EXIT (SBANDANT +077: the CM's
 * has no label). */
#define AFTER_EXIT           (SBANDANT + 077u)
#define TC_BANKCALL_GOMARKFR (AFTER_EXIT + 5u)   /* TC BANKCALL; CADR GOMARKFR */
#define TC_B5OFF_TERMINATE   (AFTER_EXIT + 7u)   /* TC B5OFF "TERMINATE" */
#define TC_B5OFF_PROCEED     (AFTER_EXIT + 010u) /* TC B5OFF */
#define TC_ENDOFJOB_RECYCLE  (AFTER_EXIT + 011u) /* TC ENDOFJOB "RECYCLE" */
#define TC_BLANKET           (AFTER_EXIT + 013u)
#define TC_BANKCALL_DELAYJOB (AFTER_EXIT + 015u) /* TC BANKCALL; CADR DELAYJOB */

/* SBANDANT +3 -- CM pp. 934-935. The time now (RTB LOADTIME, "SCALED
 * B-28") in TDEC1, and CSMCONIC takes the CM's state there: RATT, from the
 * earth (X2 = 0) or the moon (X2 = 2). "EARTH, R= -RCM" (EISOI); moon, "R=
 * -(REM+RCM) = NEG. OF S/C POS. VEC", the moon's position (LUNPOS, at the
 * "ORIGINAL TIME" TAT) added. REFSMMAT turns R into the stable member's
 * axes, *SMNB* (with CDUTRIG's sines and cosines of the CDU angles) into
 * the navigation base's: R ("SAVE NAV. BASE COORDINATES").
 *
 * Then the angles, in revolutions, B0. URP, the unit projection of R "INTO
 * CM XY-PLANE, R-(R.UZ)UZ" (none, if R lies along Z: "TEST OVERFLOW FOR
 * INDICATION OF NULL VECTOR", yaw 0). Yaw, "ACOS (URP.UX)", from 0 to 180
 * degrees when URP.UY is positive, else "2 PI MINUS YAW ANGLE"
 * (DPPOSMAX - yaw). Pitch, "ACOS (UR.UZ) - PI/2" (HIDP1/4). They go to
 * RHOSB (pitch) and GAMMASB (yaw), the antenna's. YAWANG, PITCHANG, R and
 * the rest are push-down words here. */
static const word sband_words[] = {
    ORDERS(RTB, CALL),
    ADDRESS(LOADTIME),    /* "PICKUP CURRENT TIME SCALED B-28" */
    ADDRESS(CDUTRIG),     /* "COMPUTE SINES AND COSINES OF CDU ANGLES" */
    STCALL(TDEC1),        /* "ADVANCE INTEGRATION TO TIME IN TDEC1 */
    ADDRESS(CSMCONIC),    /* USING CONIC INTEGRATION" */
    ORDERS(SLOAD, BHIZ),  /* "ORIGIN OF REFERENCE INERTIAL SYSTEM IS */
    ADDRESS_PLUS_ONE(X2), /* EARTH = 0, MOON = 2" */
    ADDRESS(EISOI),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STORE(RCM),            /* "MOVE RATT TO PREVENT WIPEOUT" */
    ORDERS(DLOAD, CALL),   /* "MOON, PUSH ON" */
    ADDRESS_PLUS_ONE(TAT), /* "GET ORIGINAL TIME" */
    ADDRESS(LUNPOS),       /* "COMPUTE POSITION VECTOR OF MOON" */
    ORDERS(VAD, VCOMP),    /* "R= -(REM+RCM) = NEG. OF S/C POS. VEC" */
    ADDRESS_PLUS_ONE(RCM),
    ORDER(GOTO),
    ADDRESS_AT(EISOI, 2),
    /* EISOI */ ORDERS(VLOAD, VCOMP), /* "EARTH, R= -RCM" */
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(SETPD, MXV),         /* "RCS TO STABLE MEMBER- B-1X B-29X B+1" */
    ADDRESS_PLUS_ONE_D(2),      /* "2D" */
    ADDRESS_PLUS_ONE(REFSMMAT), /* "STABLE MEMBER.  B-1X B-29X B+1= B-29" */
    ORDERS(VSL1, PDDL),         /* "8D" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STOVL(YAWANG),         /* "ZERO OUT YAWANG, SET UP FOR SMNB */
    ADDRESS_PLUS_ONE(RCM), /* TRANSFORMATION.  SM COORD.  SCALED B-29" */
    ORDER(CALL),
    ADDRESS(STAR_SMNB_STAR), /* *SMNB* */
    STORE(R),                /* "SAVE NAV. BASE COORDINATES" */
    ORDERS(UNIT, PDVL),      /* "14D" */
    ADDRESS_PLUS_ONE(R),
    ORDERS(VPROJ, VSL2),       /* "COMPUTE PROJECTION OF VECTOR INTO CM */
    ADDRESS_PLUS_ONE(HIUNITZ), /* XY-PLANE, R-(R.UZ)UZ" */
    ORDERS(BVSU, BOV),         /* "CLEAR OVERFLOW INDICATOR IF SET" */
    ADDRESS_PLUS_ONE(R),
    ADDRESS(COVCNV),
    /* COVCNV */ ORDERS(UNIT, BOV), /* "TEST OVERFLOW FOR INDICATION OF NULL */
    ADDRESS(NOADJUST),              /* VECTOR" */
    ORDERS(PUSH, DOT),              /* "20D" */
    ADDRESS_PLUS_ONE(HIUNITX),      /* "COMPUTE YAW ANGLE = ACOS (URP.UX)" */
    ORDERS(SL1, ACOS),              /* "REVOLUTIONS SCALED B0" */
    ORDERS(PDVL, DOT),              /* "22D YAWANG" */
    ADDRESS_PLUS_ONE(URP),
    ADDRESS_PLUS_ONE(HIUNITY),         /* "COMPUTE FOLLOWING- URP.UY" */
    ORDERS(SL1, BPL),                  /* "POSITIVE" */
    ADDRESS(NOADJUST),                 /* "YES, 0- 180 DEGREES" */
    ORDERS(DLOAD, DSU),                /* "NO, 181-360 DEGREES 20D" */
    ADDRESS_PLUS_ONE(DPPOSMAX),        /* "COMPUTE 2 PI MINUS YAW ANGLE" */
    ORDER(PUSH),                       /* "22D YAWANG" */
    /* NOADJUST */ ORDERS(VLOAD, DOT), /* "COMPUTE PITCH ANGLE" */
    ADDRESS_PLUS_ONE(UR),              /* "ACOS (UR.UZ) - PI/2" */
    ADDRESS_PLUS_ONE(HIUNITZ),
    ORDERS(SL1, ACOS), /* "REVOLUTIONS B0" */
    ORDER(DSU),
    ADDRESS_PLUS_ONE(HIDP1_4), /* HIDP1/4 */
    STODL(RHOSB),
    ADDRESS_PLUS_ONE(YAWANG),
    STORE(GAMMASB), /* "PATCH FOR CHECKOUT" */
    ORDER(EXIT),
};

/* SBANDANT +077 -- CM p. 935. "IS BIT 5 STILL ON" in EXTVBACT: if not,
 * "WE HAVE BEEN ANSWERED", and ENDEXT ends the verb; if so, "DISPLAY
 * ANGLES": verb 06 noun 51, through GOMARKFR. */
void sbandant_after_exit(void)
{
    ca(ADDRESS_OF(EXTVBACT));
    mask(BIT5);
    if (A_is_zero()) { /* EXTEND; BZF ENDEXT: "NO" */
        continue_at(ENDEXT);
        return;
    }
    set_A(V06N51);
    tc_at(TC_BANKCALL_GOMARKFR);
    continue_at(BANKCALL); /* CADR GOMARKFR */
}

/* SBANDANT +0106 -- "TERMINATE". */
void sbandant_terminate(void)
{
    tc_at(TC_B5OFF_TERMINATE);
    continue_at(B5OFF);
}

/* SBANDANT +0107 -- proceed. */
void sbandant_proceed(void)
{
    tc_at(TC_B5OFF_PROCEED);
    continue_at(B5OFF);
}

/* SBANDANT +0110 -- "RECYCLE": the job ends; the display's recycle
 * starts it again. */
void sbandant_recycle(void)
{
    tc_at(TC_ENDOFJOB_RECYCLE);
    continue_at(ENDOFJOB);
}

/* SBANDANT +0111 -- GOMARKFR's "IMMEDIATE RETURN": "BLANK R3". */
void sbandant_immediate_return(void)
{
    set_A(BIT3);
    tc_at(TC_BLANKET);
    continue_at(BLANKET);
}

/* SBANDANT +0113 -- "DELAY MINIMUM TIME TO ALLOW DISPLAY IN" (DELAYJOB,
 * a delay of 1). */
void sbandant_after_blanket(void)
{
    set_A(BIT1);
    tc_at(TC_BANKCALL_DELAYJOB);
    continue_at(BANKCALL); /* CADR DELAYJOB */
}

/* SBANDANT +0116 -- the angles again, from SBANDANT +2 (TCF). */
void sbandant_after_delayjob(void)
{
    sbandant_after_r02both();
}

#endif

const struct interpretive_code SBAND_CODE = {042u, SBANDANT + 3u, COUNT(sband_words), sband_words};
