/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 15 (Luminary099 pp. 588-593): LRS24.1, the RR search
 * routine (R24).
 *
 * LRS24.1 "DRIVES THE RENDEZVOUS RADAR IN A HEXAGONAL SEARCH PATTERN ABOUT
 * THE LOS TO THE CSM (COMPUTED FROM THE CSM AND LM STATE VECTORS) CHECKING
 * FOR THE DATA GOOD DISCRETE AND MONITORING THE ANGLE BETWEEN THE RADAR
 * BORESIGHT AND THE LM +Z AXIS". Point 0 is the line of sight itself;
 * points 1 to 6 lie round it, each 60 degrees on from the last, offset by
 * the tangent of 3.25 degrees (OFFSTFAC). NSRCHPNT counts the points.
 *
 * Each point is a job: it designates the radar (RRDESSM), shows OMEGA, the
 * angle between the radar's axis and the LM's +Z axis (OMEGDISP), and
 * schedules a task (CALLDGCH) for 6 seconds later. That task starts a job,
 * DATGDCHK, which looks for the data good discrete: there, 11111 to
 * DATAGOOD for the display; not yet, the next point (after point 6, point
 * 0 again). With the search option (SRCHOPT) off, the job ends.
 *
 * As in the other parts, a branch or CALL to a place the faithful port
 * keys a routine at ends the pass there. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- LRS24.1 and CHKSRCH ---- */

/* CHKSRCH -- pp. 588-589: "ISSUE AUTO TRACK ENABLE TO RADAR" (channel 12
 * bit 14). "CHECK IF SEARCH STILL REQUESTED" (SRCHOPT): "NO-TERMINATE JOB".
 * Yes: "SCHEDULE TASK TO DRIVE RADAR TO NEXT PT." (CALLDGCH) "IN 6
 * SECONDS". */
static void chksrch(void)
{
    set_A(BIT14);
    wor_channel(CHAN12);
    set_A(SRCHOBIT);
    mask_at(ADDRESS_OF(FLAGWRD2));
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    set_A(SIX_SECONDS);
    inhint();
    tc_to(LRS24_1 + 013u, WAITLIST); /* 2CADR CALLDGCH */
}

/* LRS24.1 -- p. 588: "SET SEARCH PATTERN POINT COUNTER TO ZERO", and
 * CHKSRCH. (A +0: the TS does not skip.) */
void lrs24_1(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(NSRCHPNT));
    chksrch();
}

/* LRS24.1 +016: "IS REMODE IN PROGRESS" (RADMODES bit 14): "YES -- WAIT
 * SIX SECONDS", the job ends. No: the interpreter. */
void lrs24_1_after_waitlist(void)
{
    relint();
    cs(ADDRESS_OF(RADMODES));
    mask(REMODBIT);
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    tc_at(LRS24_1 + 023u);
    intpret();
}

/* LRS24.1 +024 -- pp. 589-590: "COMPUTE LOS AT PRESENT TIME + 1.5 SEC.":
 * the LM's state vector to then ("EXTRAPOLATE LM STATE VECTOR", its
 * position to RLMSRCH and its velocity to SAVLEMV), and the CSM's. "LOS
 * VECTOR = R(CSM) - R(LM)", a unit vector to LOSDESRD; "UNIT(V(CM) CROSS
 * R(CM))" to VXRCM; the CSM's velocity less the LM's, "CONVERT FROM
 * REFERENCE TO STABLE MEMBER", to SAVLEMV ("VLC = V(CSM) - V(LM)").
 *
 * "CHECK IF N=0": "YES -- DESIGNATE ALONG LOS" (DESGLOS). "IS N=1": "YES --
 * CALCULATE X AND Y AXES OF SEARCH PATTERN COORDINATE SYSTEM" (CALCXY).
 * "NO -- ROTATE X-Y AXES TO NEXT SEARCH POINT", by 60 degrees: "SAVE
 * ORIGINAL X AND Y VECTORS" (UXVECTPR, UYVECTPR, push-down words 12D and
 * 18D), "UX = (COS 60)UXPR + (SIN 60)UYPR" and "UY = (-SIN 60)UXPR + (COS
 * 60)UYPR", each a unit vector.
 *
 * OFFCALC -- p. 590: "OFFSET VECTOR = K(UY)"; with the line of sight it
 * defines the "DESIRED POINT IN SEARCH PATTERN", a unit vector, "CONVERT TO
 * STABLE MEMBER COORDINATES". CONTDESG: to RRTARGET, and SAVLEMV to LOSVEL.
 * EXIT. */
static const word lrs24_1_words[] = {
    ORDERS(RTB, DAD), /* "COMPUTE LOS AT PRESENT TIME + 1.5 SEC." */
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(1_5SECS),
    /* LRS24.11 */ STCALL(TDEC1),
    ADDRESS(LEMCONIC), /* "EXTRAPOLATE LM STATE VECTOR" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(RLMSRCH), /* "SAVE LEM POSITION" */
    ADDRESS_PLUS_ONE(VATT),
    STODL(SAVLEMV), /* "SAVE LEM VELOCITY" */
    ADDRESS_PLUS_ONE(TAT),
    STCALL(TDEC1),
    ADDRESS(CSMCONIC),  /* "EXTRAPOLATE CSM STATE VECTOR" */
    ORDERS(VLOAD, VSU), /* "LOS VECTOR = R(CSM) - R(LM)" */
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS_PLUS_ONE(RLMSRCH),
    ORDER(UNIT),
    STOVL(LOSDESRD),        /* "STORE DESIRED LOS" */
    ADDRESS_PLUS_ONE(VATT), /* "COMPUTE UNIT(V(CM) CROSS R(CM))" */
    ORDERS(UNIT, VXV),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(UNIT),
    STORE(VXRCM),
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS_PLUS_ONE(SAVLEMV),
    ORDERS(MXV, VSL1), /* "CONVERT FROM REFERENCE TO STABLE MEMBER" */
    ADDRESS_PLUS_ONE(REFSMMAT),
    STORE(SAVLEMV),     /* "VLC = V(CSM) - V(LM)" */
    ORDERS(SLOAD, BZE), /* "CHECK IF N=0" */
    ADDRESS_PLUS_ONE(NSRCHPNT),
    ADDRESS(DESGLOS), /* "YES -- DESIGNATE ALONG LOS" */
    ORDERS(DSU, BZE), /* "IS N=1" */
    ADDRESS_PLUS_ONE(ONEOCT),
    ADDRESS(CALCXY),
    ORDER(VLOAD), /* "NO -- ROTATE X-Y AXES TO NEXT SEARCH POINT" */
    ADDRESS_PLUS_ONE(UXVECT),
    STOVL(UXVECTPR),          /* "SAVE ORIGINAL X AND Y VECTORS" */
    ADDRESS_PLUS_ONE(UYVECT), /* "UXPRIME = ORIGINAL UX" */
    STORE(UYVECTPR),          /* "UYPRIME = ORIGINAL UY" */
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(SIN60DEG), /* "UX = (COS 60)UXPR + (SIN 60)UYPR" */
    STOVL(UXVECT),
    ADDRESS_PLUS_ONE(UXVECTPR),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(COS60DEG),
    ADDRESS_PLUS_ONE(UXVECT),
    ORDER(UNIT),
    STOVL(UXVECT),
    ADDRESS_PLUS_ONE(UXVECTPR), /* "UY = (-SIN 60)UXPR + (COS 60)UYPR" */
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(SIN60DEG),
    STOVL(UYVECT),
    ADDRESS_PLUS_ONE(UYVECTPR),
    ORDERS(VXSC, VSU),
    ADDRESS_PLUS_ONE(COS60DEG),
    ADDRESS_PLUS_ONE(UYVECT),
    ORDER(UNIT),
    STORE(UYVECT),
    /* OFFCALC */ ORDERS(VXSC, VAD), /* "OFFSET VECTOR = K(UY)" */
    ADDRESS_PLUS_ONE(OFFSTFAC),
    ADDRESS_PLUS_ONE(LOSDESRD), /* "DESIRED POINT IN SEARCH PATTERN" */
    ORDERS(UNIT, MXV),
    ADDRESS_PLUS_ONE(REFSMMAT), /* "CONVERT TO STABLE MEMBER COORDINATES" */
    ORDER(VSL1),
    /* CONTDESG */ STOVL(RRTARGET),
    ADDRESS_PLUS_ONE(SAVLEMV),
    STORE(LOSVEL),
    ORDER(EXIT),
};

/* ---- The designate ---- */

/* LRS24.1 +0131: "KILL ANY PRESENTLY WAITLISTED TASK WHICH WOULD DESIGNATE
 * TO THE LAST POINT IN THE PATTERN" (DESLOOP +2). */
void lrs24_1_after_exit(void)
{
    inhint();
    tc_to(CONTDES2 - 2u, KILLTASK); /* CADR DESLOOP +2 */
}

/* CONTDES2 -- p. 590: "SET BIT 15 OF RADMODES TO INDICATE A CONTINUOUS
 * DESIGNATE WANTED." (Bits 1-14 plus bit 15: the AD does not overflow, and
 * the TS does not skip.) Then the interpreter. */
void contdes2(void)
{
    set_A(negate(CDESBIT));
    mask_at(ADDRESS_OF(RADMODES));
    ad(CDESBIT);
    ts(ADDRESS_OF(RADMODES));
    tc_at(CONTDES2 + 4u);
    intpret();
}

/* CONTDES2 +5 -- pp. 590-591: "DESIGNATE RADAR TO RRTARGET VECTOR". */
static const word contdes2_words[] = {
    ORDER(CALL),
    ADDRESS(RRDESSM), /* "DESIGNATE RADAR TO RRTARGET VECTOR" */
    ORDER(EXIT),
};

/* LIMALARM -- p. 593: "ISSUE ALARM 527 -- LOS NOT IN MODE2 COVERAGE IN P22
 * OR VEHICLE MANEUVER REQUIRED IN P20". */
static void limalarm(void)
{
    tc_to(LIMALARM, ALARM); /* OCT 527 */
}

/* RRDESSM's returns -- p. 591: "LOS NOT IN MODE 2 COVERAGE (P22)" and
 * "VEHICLE MANEUVER REQUIRED (P20)", LIMALARM either way. */
void contdes2_not_in_mode_2(void)
{
    tc_at(OMEGCALC - 2u);
    limalarm();
}

void contdes2_maneuver_required(void)
{
    tc_at(OMEGCALC - 1u);
    limalarm();
}

/* LIMALARM +2: "KILL WAITLIST CALL FOR NEXT POINT IN SEARCH PATTERN"
 * (CALLDGCH), and the job ends. */
void limalarm_after_alarm(void)
{
    inhint();
    tc_to(LIMALARM + 3u, KILLTASK); /* CADR CALLDGCH */
}

void limalarm_after_killtask(void)
{
    tc_to(LIMALARM + 5u, ENDOFJOB);
}

/* ---- OMEGCALC ---- */

/* OMEGCALC -- p. 591: the radar's angles (CDUT, CDUS) to TANGNB, and the
 * interpreter. */
void omegcalc(void)
{
    dca(ADDRESS_OF(CDUT)); /* EXTEND; DCA CDUT */
    dxch(ADDRESS_OF(TANGNB));
    tc_at(OMEGCALC + 3u);
    intpret();
}

/* OMEGCALC +4: "OMEGA IS ARCCOSINE OF Z-COMPONENT OF VECTOR COMPUTED BY
 * RRNB (LEFT AT 32D)", "STORE FOR DISPLAY IN R2". EXIT. */
static const word omegcalc_words[] = {
    ORDER(CALL),
    ADDRESS(RRNB),
    ORDERS(DLOAD, ACOS), /* "OMEGA IS ARCCOSINE OF Z-COMPONENT OF" */
    ADDRESS_PLUS_ONE_D(36),
    STORE(OMEGDISP), /* "STORE FOR DISPLAY IN R2" */
    ORDER(EXIT),
};

/* OMEGCALC +012: the job ends. */
void omegcalc_after_exit(void)
{
    tc_to(OMEGCALC + 012u, ENDOFJOB);
}

/* CALCXY -- p. 592: "CALCULATE X AND Y VECTORS FOR SEARCH PATTERN
 * COORDINATE SYSTEM.": "UX = (VLM X RLM) X LOS" and "UY = LOS X UX", unit
 * vectors; OFFCALC.
 *
 * DESGLOS: "WHEN N= 0,DESIGNATE ALONG LOS": "CONVERT LOS FROM REFERENCE TO
 * SM COORDS", and CONTDESG. */
static const word calcxy_words[] = {
    ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(VXRCM),
    ADDRESS_PLUS_ONE(LOSDESRD),
    ORDER(UNIT),
    STOVL(UXVECT), /* "UX = (VLM X RLM) X LOS" */
    ADDRESS_PLUS_ONE(LOSDESRD),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UXVECT),
    STORE(UYVECT), /* "UY = LOS X UX" */
    ORDER(GOTO),
    ADDRESS(OFFCALC),
    /* DESGLOS */ ORDERS(VLOAD, MXV), /* "WHEN N= 0,DESIGNATE ALONG LOS" */
    ADDRESS_PLUS_ONE(LOSDESRD),
    ADDRESS_PLUS_ONE(REFSMMAT), /* "CONVERT LOS FROM REFERENCE TO SM COORDS" */
    ORDERS(VSL1, GOTO),
    ADDRESS(CONTDESG),
};

/* ---- CALLDGCH and DATGDCHK ---- */

/* CALLDGCH -- p. 592, the task: "IS RENDEZVOUS FLAG SET": "NO -- EXIT
 * R24". "YES -- SCHEDULE JOB TO DRIVE RADAR TO NEXT PONT IN SEARCH
 * PATTERN" (DATGDCHK). */
void calldgch(void)
{
    ca(ADDRESS_OF(FLAGWRD0)); /* CAE FLAGWRD0 */
    mask(RNDVZBIT);
    if (A_is_zero()) { /* EXTEND; BZF TASKOVER */
        continue_at(TASKOVER);
        return;
    }
    set_A(PRIO25);
    tc_to(CALLDGCH + 5u, FINDVAC); /* 2CADR DATGDCHK */
}

void calldgch_after_findvac(void)
{
    tc_to(CALLDGCH + 010u, TASKOVER);
}

/* STORE1S -- pp. 592-593: "STORE 11111 FOR DISPLAY IN R1" (DATAGOOD), and
 * "DELETE DESIGNATE TASK FROM WAITLIST USING KILLTASK". (ALL1S is
 * positive: the TS does not skip.) */
static void store1s(void)
{
    set_A(ALL1S);
    ts(ADDRESS_OF(DATAGOOD));
    inhint();
    tc_to(DATGDCHK + 016u, KILLTASK); /* CADR DESLOOP +2 */
}

/* DATGDCHK -- p. 592, the job: "CHECK IF DATA GOOD DISCRETE PRESENT"
 * (channel 33 bit 4, clear when it is): "YES -- GO TO STORE 11111 FOR
 * DISPLAY IN R1". Else "IS N GREATER THAN 6": "YES -- RESET N = 0 AND
 * START AROUND AGAIN" (the pass ends at LRS24.1); "NO -- SET N = N+1 AND
 * GO TO NEXT POINT IN PATTERN" (CHKSRCH). */
void datgdchk(void)
{
    set_A(BIT4);
    rand_channel(CHAN33);
    if (A_is_zero()) { /* EXTEND; BZF STORE1S */
        store1s();
        return;
    }
    set_A(negate(SIX)); /* CS SIX */
    ad_at(ADDRESS_OF(NSRCHPNT));
    if (A_is_zero()) { /* EXTEND; BZF LRS24.1 */
        continue_at(LRS24_1);
        return;
    }
    incr(ADDRESS_OF(NSRCHPNT));
    chksrch(); /* TCF CHKSRCH */
}

void datgdchk_after_killtask(void)
{
    tc_to(DATGDCHK + 020u, ENDOFJOB);
}

/* ---- The interpretive code ---- */

const struct interpretive_code LRS24_1_CODE = {026u, LRS24_1 + 024u, COUNT(lrs24_1_words),
                                               lrs24_1_words};
const struct interpretive_code CONTDES2_CODE = {026u, CONTDES2 + 5u, COUNT(contdes2_words),
                                                contdes2_words};
const struct interpretive_code OMEGCALC_CODE = {026u, OMEGCALC + 4u, COUNT(omegcalc_words),
                                                omegcalc_words};
const struct interpretive_code CALCXY_CODE = {026u, CALCXY, COUNT(calcxy_words), calcxy_words};

#endif
