/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 17 (Luminary099 pp. 599-604): R29's designate loop,
 * BEGDES29 and R29DLOOP (bank 24), and R29DODES (bank 32).
 *
 * "TASK RECURS EVERY .5 SEC UNTIL DESIGNATE IS CALLED OFF; IT MAY WAIT FOR
 * A CENTISECOND OR TWO IF IT COMES UP WHILE SETUP29D IS SUPPLYING NEW
 * DATA." Each time it starts the job R29DODES, which "DOES MUCH THE SAME
 * THING AS DODES, BUT A GREAT DEAL FASTER": from the line of sight in
 * stable member axes (LOSSM) and half a second's line of sight velocity
 * (LOSVDT/4) it develops the shaft and trunnion commands "USING SINGLE
 * PRECISION AS MUCH AS POSSIBLE, AND INTERPRETIVE NOT AT ALL".
 *
 * Two passes through the vector loop (TANG 1, then +0):
 * - The first takes the present line of sight, unit, to navigation base
 *   axes (AX*SR*T, with the CDU angles' sines and cosines from QUICTRIG),
 *   and compares it with the radar's axis: within about half a degree, the
 *   radar's tracking is enabled (channel 12 bit 14).
 * - The second advances LOSSM half a second and takes the line of sight a
 *   second ahead; R29DPAS2 makes the shaft and trunnion commands from it,
 *   "TO NULL HAVE THE ERROR IN HALF A SECOND".
 * With the radar's data good discrete, R29LOKON ends the designate and
 * starts the reading task (R29READ); else RROUT sends the commands. Either
 * way END29DOD clears LOSCMFLG and the job ends.
 *
 * The CCS orders on TANG take all four ways in the faithful port, and this
 * code takes them the same way, the odd ones as the listing runs them. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- BEGDES29 and R29DLOOP ---- */

/* BEGDES29 -- p. 599, the task: "START R29DODES JOB TWICE A SECOND." */
void begdes29(void)
{
    set_A(PRIO21);
    tc_to(BEGDES29 + 1u, NOVAC); /* 2CADR R29DODES */
}

/* R29DLOOP: half a second on. */
void r29dloop(void)
{
    set_A(HALF_SEC);
    tc_to(R29DLOOP + 1u, VARDELAY);
}

/* R29DLOOP +2: "QUIT IF DESIGNATION IS CALLED OFF." (RADMODES bit 10).
 * "BRANCH IF SETUP29D'S SUPPLYING NEW DATA." (LOSCMFLG set): "WAIT A
 * CENTISECOND FOR NEW DATA." Else "SET LOSCMFLG: SHOW THAT DES. LOOP IS
 * ON.", and BEGDES29 again. */
void r29dloop_after_vardelay(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(DESIGBIT);
    if (bit_set_in_A()) { /* CCS A */
        continue_at(TASKOVER);
        return;
    }
    cs(ADDRESS_OF(FLAGWRD2));
    mask(LOSCMBIT);
    if (A_is_zero()) { /* EXTEND; BZF +3 */
        set_A(ONE);
        tc_to(R29DLOOP + 1u, VARDELAY); /* TCF R29DLOOP +1 */
        return;
    }
    ads(ADDRESS_OF(FLAGWRD2));
    continue_at(BEGDES29); /* TCF BEGDES29 */
}

/* ---- The vector loop ---- */

/* The DCA after CCS TANG's +0 way: EXTEND; INDEX Q; DCA from the word at
 * address + Q. On TANG -0 the CCS skips the EXTEND and the INDEX, and the
 * DCA's word, which holds the address + 1, runs as CA of that: CA
 * LOSVDT/4 +1. Not modelled: TANG negative, where the DCA's word runs
 * after an INDEX with no EXTEND. */
static void dca_by_tang(enum sign tang, unsigned address)
{
    switch (tang) {
    case IS_PLUS_ZERO: dca(indexed(address, fetch(Q_ADDRESS))); return;
    case IS_MINUS_ZERO: ca(address + 1u); return;
    default: not_modelled();
    }
}

/* R29DVBEG -- p. 600, A 5: "COUNT DOWN BY TWO'S IN VECTOR LOOP." (Q 4, 2
 * and 0, the vectors' X, Y and Z). On the first pass (TANG positive) each
 * LOSSM word to MPAC +1; on the second "ADVANCE LOS VECTOR 1/2 SECOND." and
 * "PROJECT LOS 1 SECOND AHEAD (2ND PASS).". Then the vector, from MPAC +1,
 * to MPAC (DXCH MPAC +1; DXCH MPAC); "WITH FIXLOC ARMED FOR LENGTH AND
 * LENGTH SQUARED, BORROW UNITIZING ROUTINE." (USPRCADR, CADR UNIT). (A is
 * 5, 3 or 1 at CCS A, and Q 4, 2 or 0 at CCS Q: the CCS orders go one way,
 * and no TS skips.) */
static void r29dvbeg(void)
{
    enum sign tang;

    for (;;) {
        if (ccs_at(A_ADDRESS) != POSITIVE) not_modelled(); /* CCS A */
        ts(Q_ADDRESS);
        tang = ccs_at(ADDRESS_OF(TANG));
        if (tang != POSITIVE) { /* (TCF R29DPAS1 "ON 1ST PASS THRU LOOP.") */
            dca_by_tang(tang, ADDRESS_OF(LOSVDT_4));
            das(indexed(ADDRESS_OF(LOSSM), fetch(Q_ADDRESS))); /* INDEX Q; DAS LOSSM */
        }
        /* R29DPAS1: "MOVE CURRENT LOS (1ST PASS) OR LOS PRO- JECTED 1/2
         * SEC AHEAD (2ND PASS)." */
        dca(indexed(ADDRESS_OF(LOSSM), fetch(Q_ADDRESS)));
        dxch(indexed(ADDRESS_OF(MPAC) + 1u, fetch(Q_ADDRESS)));
        tang = ccs_at(ADDRESS_OF(TANG));
        if (tang != POSITIVE) { /* ("BUG OUT HERE IN 1ST PASS.") */
            dca_by_tang(tang, ADDRESS_OF(LOSVDT_4));
            das(indexed(ADDRESS_OF(MPAC) + 1u, fetch(Q_ADDRESS))); /* INDEX Q; DAS MPAC +1 */
        }
        /* R29DVEND: "BRANCH TO CONTINUE VECTOR LOOP." */
        switch (ccs_at(Q_ADDRESS)) {
        case POSITIVE: continue;
        case IS_PLUS_ZERO: break;
        default: not_modelled();
        }
        break;
    }
    dxch(ADDRESS_OF(MPAC) + 1u);
    dxch(ADDRESS_OF(MPAC));
    set_A(R29FXLOC); /* "= ADRES INTB15 + -34D" */
    ts(ADDRESS_OF(FIXLOC));
    tc_to(R29DODES + 036u, USPRCADR); /* CADR UNIT */
}

/* R29DODES -- p. 600, the job: "INDICATE 1ST PASS THRU VECTOR LOOP." */
void r29dodes(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(TANG));
    set_A(FIVE);
    r29dvbeg();
}

/* R29DODES +040 -- p. 601: "MOVE UNIT(LOS) TO AX*SR*T ARG AREA." */
void r29dodes_after_unit(void)
{
    tc_to(R29DODES + 040u, MPACVBUF);
}

/* R29DODES +041: "GET CDU ANGLES ONLY AFTER 1ST PASS." (TANG +0 goes to
 * GOTANGLS). "ENSURE CONSISTENT CDU READINGS." (TANG -0 skips the INHINT,
 * as the listing runs it): the trunnion and shaft angles to SAVECDUT, and
 * the IMU's "IN FUNNY ORDER FOR AX*SR*T." (Y, Z, X) to CDUSPOT; then "GET
 * SINES AND COSINES OF CDU ANGLES." (QUICTRIG). */
void r29dodes_after_mpacvbuf(void)
{
    switch (ccs_at(ADDRESS_OF(TANG))) {
    case IS_PLUS_ZERO: continue_at(GOTANGLS); return; /* TCF GOTANGLS */
    case IS_MINUS_ZERO: break;
    default: inhint(); /* (TCF +2) */
    }
    dca(ADDRESS_OF(CDUT)); /* EXTEND; DCA CDUT */
    dxch(ADDRESS_OF(SAVECDUT));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(CDUSPOT));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(CDUSPOT) + 2u);
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUSPOT) + 4u);
    tc_to(R29DODES + 056u, BANKCALL); /* CADR QUICTRIG */
}

/* GOTANGLS: "TRANSFORM UNIT LOS TO NB AXES (ULOSNB)." */
void gotangls(void)
{
    set_A(negate(THREE));
    tc_to(GOTANGLS + 1u, BANKCALL); /* CADR AX*SR*T */
}

/* ---- R29DPAS2 ---- */

/* R29DPAS2 -- p. 603: "COMPUTE SHAFT AND TRUNNION COMMANDS TO NULL HAVE
 * THE ERROR IN HALF A SECOND." The raw shaft command, "ULOSNB' . (COS S, 0,
 * - SIN S)", in TANG; the raw trunnion command, "ULOSNB'. (SIN S SIN T, COS
 * T, SIN S COS T)", in MPAC; each times RR29GAIN, "REFINED", the trunnion's
 * to TANG and the shaft's to TANG +1. (PUSHLOC holds cos T, MODE -sin T,
 * SAVECDUT sin S and SAVECDUT +1 cos S. MP leaves no overflow: the TS does
 * not skip.)
 *
 * DGOOD? -- p. 604: "GET RR DATA-GOOD BIT." "BRANCH IF DATA-GOOD IS
 * PRESENT." (R29LOKON); "DATA-GOOD IS ABSENT, SO SEND COMMANDS." (RROUT). */
static void r29dpas2(void)
{
    ca(ADDRESS_OF(SAVECDUT) + 1u);
    mp_at(ADDRESS_OF(VBUF)); /* "FORM COS S ULOSNB'X." */
    dxch(ADDRESS_OF(TANG));
    cs(ADDRESS_OF(SAVECDUT));
    mp_at(ADDRESS_OF(VBUF) + 4u); /* "FORM - SIN S ULOSNB'Z." */
    das(ADDRESS_OF(TANG));
    cs(ADDRESS_OF(MODE));
    mp_at(ADDRESS_OF(SAVECDUT));
    mp_at(ADDRESS_OF(VBUF)); /* "FORM SIN T SIN S ULOSNB'X." */
    dxch(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(PUSHLOC));
    mp_at(ADDRESS_OF(VBUF) + 2u); /* "FORM COS T ULOSNB'Y." */
    das(ADDRESS_OF(MPAC));
    cs(ADDRESS_OF(MODE));
    mp_at(ADDRESS_OF(SAVECDUT) + 1u);
    mp_at(ADDRESS_OF(VBUF) + 4u); /* "FORM SIN T COS S ULOSNB'Z." */
    das(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(MPAC));
    mp(RR29GAIN);
    xch(ADDRESS_OF(TANG)); /* "STORE REFINED T CMD, GET RAW S CMD." */
    mp(RR29GAIN);
    ts(ADDRESS_OF(TANG) + 1u); /* "STORE REFINED S CMD." */
    /* DGOOD? */
    set_A(BIT4);
    rand_channel(CHAN33); /* "GET RR DATA-GOOD BIT." */
    inhint();             /* "(MAINLY FOR RROUT)." */
    if (A_is_zero()) {    /* EXTEND; BZF R29LOKON */
        continue_at(R29LOKON);
        return;
    }
    tc_to(R29DPAS2 + 045u, BANKCALL); /* CADR RROUT */
}

/* GOTANGLS +3 -- p. 601: after the first pass, the cosine of the angle
 * between the line of sight and the radar's axis; after the second
 * (TANG +0), R29DPAS2. "INDICATE 2ND PASS THRU VECTOR LOOP." (TANG -0
 * skips the INHINT; A, the CCS's, does not overflow.) Then cos T. */
void gotangls_after_axsrt(void)
{
    switch (ccs_at(ADDRESS_OF(TANG))) {
    case IS_PLUS_ZERO: r29dpas2(); return; /* TCF R29DPAS2 */
    case IS_MINUS_ZERO: break;
    default: inhint(); /* (TCF +2) */
    }
    ts(ADDRESS_OF(TANG)); /* "INDICATE 2ND PASS THRU VECTOR LOOP." */
    ca(ADDRESS_OF(SAVECDUT));
    tc_to(GOTANGLS + 011u, SPCOS);
}

/* ---- The cosine of the error ---- */

/* GOTANGLS +012 -- p. 602: "PUSHLOC = COS T."; then sin T. (An A that
 * overflows skips the CS, as the listing runs it.) */
void gotangls_after_cos_t(void)
{
    if (!ts(ADDRESS_OF(PUSHLOC))) cs(ADDRESS_OF(SAVECDUT));
    tc_to(GOTANGLS + 014u, SPSIN);
}

/* GOTANGLS +015: "MODE = -SIN T.", "FORM - SIN T ULOSNBY." in MPAC; then
 * sin S. Not modelled: an A that overflows, where the TS would skip. */
void gotangls_after_sin_t(void)
{
    if (ts(ADDRESS_OF(MODE))) not_modelled();
    mp_at(ADDRESS_OF(VBUF) + 2u); /* "FORM - SIN T ULOSNBY." */
    dxch(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(SAVECDUT) + 1u);
    tc_to(GOTANGLS + 022u, SPSIN);
}

/* GOTANGLS +023: "SAVECDUT NOW = SIN S.", "FORM SIN S COS T ULOSNBX." added
 * in; then cos S. Not modelled: an A that overflows. */
void gotangls_after_sin_s(void)
{
    if (ts(ADDRESS_OF(SAVECDUT))) not_modelled();
    mp_at(ADDRESS_OF(PUSHLOC));
    mp_at(ADDRESS_OF(VBUF)); /* "FORM SIN S COS T ULOSNBX." */
    das(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(SAVECDUT) + 1u);
    tc_to(GOTANGLS + 032u, SPCOS);
}

/* GOTANGLS +033: "SAVECDUT +1 NOW = COS S.", "FORM COS S COS T ULOSNBZ.":
 * "COS(ERROR) = ULOSNB . (SIN S COS T, -SIN T, COS S COS T)." TESTCOS
 * doubles it ("ULOSNB IN VBUF WAS A HALF-UNIT VECTOR"), and "TEST FOR +
 * OVERFLOW, NONE, OR MINUS.": "IF PLUS OVERFLOW, SET SELFTRACK ENABLE."
 * "MAKE 2ND PASS THRU VECTOR LOOP." (R29DVBEG -1). Not modelled: an A that
 * overflows at the TS. */
void gotangls_after_cos_s(void)
{
    if (ts(ADDRESS_OF(SAVECDUT) + 1u)) not_modelled();
    mp_at(ADDRESS_OF(PUSHLOC));
    mp_at(ADDRESS_OF(VBUF) + 4u); /* "FORM COS S COS T ULOSNBZ." */
    das(ADDRESS_OF(MPAC));
    dca(ADDRESS_OF(MPAC));       /* EXTEND; DCA MPAC */
    das(ADDRESS_OF(MPAC));       /* TESTCOS */
    switch (ccs_at(A_ADDRESS)) { /* (DAS leaves the overflow, +1, +0 or -1, in A) */
    case POSITIVE: set_A(BIT14); break;
    case IS_PLUS_ZERO:
    case NEGATIVE: break; /* (NOOP; A +0) */
    case IS_MINUS_ZERO: not_modelled();
    }
    wor_channel(CHAN12); /* "IF PLUS OVERFLOW, SET SELFTRACK ENABLE." */
    relint();
    set_A(FIVE); /* TCF R29DVBEG -1: CA FIVE */
    r29dvbeg();
}

/* ---- R29LOKON and END29DOD ---- */

/* DGOOD? +010: after RROUT, END29DOD. */
void r29dpas2_after_rrout(void)
{
    continue_at(END29DOD); /* TCF END29DOD */
}

/* R29LOKON -- p. 604: "SHOW THAT DESIGNATION IS OVER." (the TS does not
 * skip), "DISABLE RR ERROR COUNTERS.", "SHOW THAT READING HAS BEEN
 * REQUESTED." (READRFLG). "SEE IF TASK SHOULD BE OFFSET 1 SEC." by PIPCTR:
 * positive, "-96D + 100D = 4." centiseconds; +0, "0 + 100D = 100D.";
 * negative, the CCS's |PIPCTR| - 1. "START READING TASK AND JOB."
 * (R29READ). Not modelled: PIPCTR -0, which runs the 2CADR as an
 * instruction. */
void r29lokon(void)
{
    set_A(negate(DESIGBIT));
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES)); /* "SHOW THAT DESIGNATION IS OVER." */
    set_A(negate(BIT2));
    wand_channel(CHAN12); /* "DISABLE RR ERROR COUNTERS." */
    set_A(READRBIT);
    ads(ADDRESS_OF(FLAGWRD3)); /* "SHOW THAT READING HAS BEEN REQUESTED." */
    switch (ccs_at(ADDRESS_OF(PIPCTR))) {
    case POSITIVE:
        set_A(negate(SUPER110)); /* "-96D + 100D = 4." */
        ad(ONE_SEC);
        break;
    case IS_PLUS_ZERO: ad(ONE_SEC); break; /* "0 + 100D = 100D." */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    tc_to(R29LOKON + 013u, WAITLIST); /* 2CADR R29READ */
}

/* END29DOD: "ALWAYS CLEAR LOSCMFLG." (the TS does not skip), and the job
 * ends. */
void end29dod(void)
{
    set_A(negate(LOSCMBIT));
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2)); /* "ALWAYS CLEAR LOSCMFLG." */
    continue_at(ENDOFJOB);    /* TCF ENDOFJOB */
}

#endif
