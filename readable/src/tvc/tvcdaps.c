/* SPDX-License-Identifier: MIT */
/* TVCDAPS.agc (Comanche055 pp. 961-978, bank 20): the TVC DAP of the
 * pitch and yaw SPS gimbal actuators, "SELF-PERPETUATING T5 TASKS WHICH
 * GENERATE THE COMMAND SIGNALS" from the body-axis rate commands of
 * cross-product steering. PITCHDAP and YAWDAP call each other by T5, at
 * "1/2 THE DAP SAMPLE" time, and DAPINIT starts them.
 *
 * "THE PITCH AND YAW LOOPS ARE SEPARATE, BUT STRUCTURED IDENTICALLY", so
 * one set of steps runs both, each on its own erasables (struct axis),
 * named by the listing's labels. The file's subroutines (TCR, TC Q) are
 * calls: the Q each leaves, a later TC sets again. A pass: the body-axis rate from the CDU
 * rates; its error integrated (ERRBTMP) and limited (ERRORLIM); the
 * filter's present output (FWDFLTR, with the variable gain VARK); the CG
 * offset added and the actuator command limited (ACTLIM); its increment
 * to the outcounter; the filter's next pass worked out ahead (PRECOMP);
 * the offset tracker filter; and the copycycle, which stores it all,
 * restart-protected by TVCPHASE. The faithful C is c64-port/src/cm_tvc_daps.c. */
#include "tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* One loop's erasables. */
struct axis {
    unsigned errb;    /* PERRB, YERRB: the integrated body-axis rate error */
    unsigned omegac;  /* OMEGAYC, OMEGAZC: the rate command */
    unsigned tmp;     /* PTMP1, YTMP1: the filter's six double words */
    unsigned deloff;  /* PDELOFF, YDELOFF: the CG offset */
    unsigned cmd;     /* PCMD, YCMD: the actuator command */
    unsigned counter; /* TVCPITCH, TVCYAW: the outcounter */
    word release;     /* BIT11, BIT12: its "COUNT RELEASE" in channel 14 */
    unsigned delbar;  /* DELPBAR, DELYBAR: the offset tracker filter */
    unsigned needle;  /* AK1, AK2: the rate error for the needles */
    bool last;        /* yaw: the copycycle ends the DAP's cycle */
};

static const struct axis pitch = {
    .errb = ADDRESS_OF(PERRB),
    .omegac = ADDRESS_OF(OMEGAYC),
    .tmp = ADDRESS_OF(PTMP1),
    .deloff = ADDRESS_OF(PDELOFF),
    .cmd = ADDRESS_OF(PCMD),
    .counter = ADDRESS_OF(TVCPITCH),
    .release = BIT11,
    .delbar = ADDRESS_OF(DELPBAR),
    .needle = ADDRESS_OF(AK1),
    .last = false,
};

static const struct axis yaw = {
    .errb = ADDRESS_OF(YERRB),
    .omegac = ADDRESS_OF(OMEGAZC),
    .tmp = ADDRESS_OF(YTMP1),
    .deloff = ADDRESS_OF(YDELOFF),
    .cmd = ADDRESS_OF(YCMD),
    .counter = ADDRESS_OF(TVCYAW),
    .release = BIT12,
    .delbar = ADDRESS_OF(DELYBAR),
    .needle = ADDRESS_OF(AK2),
    .last = true,
};

static void dap_pass(const struct axis *axis);
static void forward(const struct axis *axis);
static void offset(const struct axis *axis);
static void out(const struct axis *axis);
static void delbar(const struct axis *axis);

/* The double word at x times the coefficient at c, into the double word
 * at into, whose upper word is +0: the lower word's product to into +1
 * (TS), then the upper word's added (DAS). Negated: CS for the CA, the
 * product's negative ("MULTIPLY OUTPUT BY"). */
static void multiply(unsigned into, unsigned x, unsigned c, bool negated)
{
    void (*load)(unsigned) = negated ? cs : ca;

    load(x + 1u);
    mp_at(c);
    ts(into + 1u);
    load(x);
    mp_at(c);
    das(into);
}

/* ---- PITCHDAP and YAWDAP -- pp. 963-967 ---- */

/* PSTROKER, YSTROKER: "CHECK FOR STROKE TEST" (STROKER): positive,
 * "TEST-START OR TEST-IN-PROGRESS", negative, "TEST-IN-PROGRESS": TC
 * HACK, the stroke test's waveform (true). +0, "NO-TEST", and -0: on. */
static bool stroke_test(word stroker)
{
    switch (ccs_at(ADDRESS_OF(STROKER))) {
    case POSITIVE: tc_to(stroker + 1u, HACK); return true;
    case NEGATIVE: tc_to(stroker + 3u, HACK); return true;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    return false;
}

/* PITCHDAP, the T5 interrupt: BANKRUPT and QRUPT kept; YAWDAP next by T5
 * (T5LOC YAWT5; TIME5 T5TVCDT); the stroke test; then PCDUDOTS. */
void pitchdap(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    set_A(YAWT5);
    ts(ADDRESS_OF(T5LOC));
    ca(ADDRESS_OF(T5TVCDT));
    ts(ADDRESS_OF(TIME5));
    if (!stroke_test(PSTROKER)) pcdudots();
}

/* PITCHDAP +011, after the first TC HACK: TCF +2, PCDUDOTS. */
void pitchdap_after_hack(void)
{
    pcdudots();
}

/* RLIMTEST: "TEST FOR EXCESSIVE CDU RATES (GREATER" "THAN 2.33 DEG IN ONE
 * SAMPLE PERIOD)": the difference in A (TTMP1) times 1/RTLIM; not +0,
 * the rate +0. */
static void rlimtest(void)
{
    ts(ADDRESS_OF(TTMP1));
    mp(ONE_OVER_RTLIM);
    if (!A_is_zero()) { /* EXTEND; BZF +3 */
        set_A(ZERO);
        ts(ADDRESS_OF(TTMP1));
    }
    ca(ADDRESS_OF(TTMP1));
}

/* A CDU's rate: the past value less the CDU (MSU), the CDU the new past
 * value, through RLIMTEST; the caller stores it, "(MINUS, SC.AT 1/2TVCDT
 * REVS/SEC)". */
static void cdu_rate(unsigned cdu, unsigned past)
{
    ca(cdu);
    xch(past);
    msu_at(past);
    rlimtest();
}

/* PCDUDOTS: "COMPUTE CDUYDOT (USED BY PITCH AND YAW)" (MCDUYDOT), then
 * CDUZDOT. */
void pcdudots(void)
{
    cdu_rate(ADDRESS_OF(CDUY), ADDRESS_OF(PCDUYPST));
    pcdudots_ydot();
}

/* PCDUDOTS +5, where RLIMTEST returns: MCDUYDOT; then "COMPUTE CDUZDOT". */
void pcdudots_ydot(void)
{
    ts(ADDRESS_OF(MCDUYDOT));
    cdu_rate(ADDRESS_OF(CDUZ), ADDRESS_OF(PCDUZPST));
    pcdudots_zdot();
}

/* PCDUDOTS +013, where RLIMTEST returns: MCDUZDOT. Then PINTEGRL:
 * "COMPUTE INTEGRAL OF BODY-AXIS PITCH-RATE" "ERROR, SC.AT B-1 REVS":
 * PERRB plus OMEGAYC, less OMEGAYB, the "BODY-AXIS PITCH RATE" from the
 * CDU rates (-COSCDUZ COSCDUX MCDUYDOT less MCDUZDOT SINCDUX, each
 * product doubled); then the pass. */
void pcdudots_zdot(void)
{
    ts(ADDRESS_OF(MCDUZDOT));
    dca(pitch.errb); /* PINTEGRL */
    dxch(ADDRESS_OF(ERRBTMP));
    dca(pitch.omegac);
    das(ADDRESS_OF(ERRBTMP));
    cs(ADDRESS_OF(COSCDUZ));
    mp_at(ADDRESS_OF(COSCDUX));
    ddoubl();
    mp_at(ADDRESS_OF(MCDUYDOT));
    ddoubl();
    dxch(ADDRESS_OF(OMEGAYB));
    cs(ADDRESS_OF(MCDUZDOT));
    mp_at(ADDRESS_OF(SINCDUX));
    ddoubl();
    das(ADDRESS_OF(OMEGAYB));
    dcs(ADDRESS_OF(OMEGAYB)); /* "PICK UP -OMEGAYB (SIGN CHNG, INTEGRATE)" */
    das(ADDRESS_OF(ERRBTMP));
    dap_pass(&pitch);
}

/* YAWDAP, the T5 interrupt: BANKRUPT and QRUPT kept; PITCHDAP next by T5
 * (T5LOC PITCHT5; TIME5 T5TVCDT); the stroke test; then YINTEGRL. */
void yawdap(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    set_A(PITCHT5);
    ts(ADDRESS_OF(T5LOC));
    ca(ADDRESS_OF(T5TVCDT));
    ts(ADDRESS_OF(TIME5));
    if (!stroke_test(YSTROKER)) yintegrl();
}

/* YAWDAP +011, after the first TC HACK: TCF +2, YINTEGRL. */
void yawdap_after_hack(void)
{
    yintegrl();
}

/* YINTEGRL: "USE BODY RATES FROM PITCHDAP (PCDUDOTS)". "COMPUTE INTEGRAL
 * OF BODY-AXIS YAW-RATE" "ERROR": YERRB plus OMEGAZC, less OMEGAZB
 * (COSCDUZ SINCDUX MCDUYDOT less MCDUZDOT COSCDUX, each product
 * doubled); then the pass. */
void yintegrl(void)
{
    dca(yaw.errb);
    dxch(ADDRESS_OF(ERRBTMP));
    dca(yaw.omegac);
    das(ADDRESS_OF(ERRBTMP));
    ca(ADDRESS_OF(COSCDUZ));
    mp_at(ADDRESS_OF(SINCDUX));
    ddoubl();
    mp_at(ADDRESS_OF(MCDUYDOT));
    ddoubl();
    dxch(ADDRESS_OF(OMEGAZB));
    cs(ADDRESS_OF(MCDUZDOT));
    mp_at(ADDRESS_OF(COSCDUX));
    ddoubl();
    das(ADDRESS_OF(OMEGAZB));
    dcs(ADDRESS_OF(OMEGAZB)); /* "PICK UP -OMEGAZB (SIGN CHNG, INTEGRATE)" */
    das(ADDRESS_OF(ERRBTMP));
    dap_pass(&yaw);
}

/* ---- The limiters -- p. 971 ---- */

/* After an MP by the limit's reciprocal: +0 in A (BZF +6), within the
 * limit. Otherwise the upper word at address the limit, with its sign
 * ("WRITES OVER UPPER WORD ONLY"). (The word cannot be +0 or -0 here:
 * the MP was of it.) */
static void limit(unsigned address, word limit_word)
{
    if (A_is_zero()) return;
    switch (ccs_at(address)) {
    case POSITIVE: set_A(limit_word); break;
    case NEGATIVE: set_A(negate(limit_word)); break; /* CS */
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break; /* (the CCS's +0) */
    }
    ts(address);
}

/* ERRORLIM: "CHECK FOR INPUT-ERROR LIMIT" of ERRBTMP, the "FILTER INPUT
 * LIMIT", ERRLIM; "CHECKS UPPER WORD ONLY". */
static void errorlim(void)
{
    ca(ADDRESS_OF(ERRBTMP));
    mp(ONE_OVER_ERRLIM);
    limit(ADDRESS_OF(ERRBTMP), ERRLIM);
}

/* ACTLIM: "ROUND UP FOR OUTPUT": CMDTMP +1 doubled to L, then CMDTMP
 * (an overflow at the TS L skips the CAF ZERO, so CMDTMP is added to the
 * +1 or -1 it leaves in A: the round up). Then "CHECK FOR ACTUATOR
 * COMMAND LIMIT", ACTSAT, of CMDTMP itself. */
static void actlim(void)
{
    ca(ADDRESS_OF(CMDTMP) + 1u);
    double_A();
    if (!ts(L_ADDRESS)) set_A(ZERO);
    ad_at(ADDRESS_OF(CMDTMP));
    mp(ONE_OVER_ACTSAT);
    limit(ADDRESS_OF(CMDTMP), ACTSAT);
}

/* ---- The filter -- pp. 972-977 ---- */

/* A cascade's present output: the input times its numerator gain, plus
 * the node worked out ahead (DXCH, DAS). */
static void forward_cascade(unsigned output, unsigned input, unsigned gain, unsigned node)
{
    multiply(output, input, gain, false);
    dxch(node);
    das(output);
}

/* FWDFLTR, "FILTER COMPUTATIONS FOR PRESENT OUTPUT": DAP1 to DAP3,
 * CMDTMP and DELBRTMP +0; the first and second cascades (N10, N20 at N10
 * +5); "TEST FOR LEM ON OR OFF" (bit 14 of DAPDATR1): on, the third
 * cascade (N30 at N10 +10D); off, DAP3 is DAP2. Then OPTVARK, the
 * "VARIABLE GAIN PACKAGE" "(ALSO, SIGN CHANGE IN FORWARD LOOP)": CMDTMP,
 * -DAP3 times VARK, "FIX UP SCALING" (DDOUBL twice). */
static void fwdfltr(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DAP1));
    ts(ADDRESS_OF(DAP2));
    ts(ADDRESS_OF(DAP3));
    ts(ADDRESS_OF(CMDTMP));
    ts(ADDRESS_OF(DELBRTMP));
    forward_cascade(ADDRESS_OF(DAP1), ADDRESS_OF(ERRBTMP), ADDRESS_OF(N10), ADDRESS_OF(TMP1));
    forward_cascade(ADDRESS_OF(DAP2), ADDRESS_OF(DAP1), ADDRESS_OF(N10) + 5u, ADDRESS_OF(TMP3));
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT14);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* (A BIT14 or +0) */
        forward_cascade(ADDRESS_OF(DAP3), ADDRESS_OF(DAP2), ADDRESS_OF(N10) + 10u,
                        ADDRESS_OF(TMP5));
    } else {
        dca(ADDRESS_OF(DAP2));
        dxch(ADDRESS_OF(DAP3));
    }
    multiply(ADDRESS_OF(CMDTMP), ADDRESS_OF(DAP3), ADDRESS_OF(VARK), true); /* OPTVARK */
    dxch(ADDRESS_OF(CMDTMP));
    ddoubl();
    ddoubl();
    dxch(ADDRESS_OF(CMDTMP));
}

/* A cascade's precomputation: its two nodes for the next pass from the
 * input and output, by the coefficients at c (Nx1/2, Nx2, Dx1/2, Dx2).
 * The first node: the old second node plus twice (input Nx1/2 less
 * output Dx1/2); the second: input Nx2 less output Dx2. */
static void precompute_cascade(unsigned input, unsigned output, unsigned c, unsigned node)
{
    set_A(ZERO);
    ts(ADDRESS_OF(TTMP1));
    ts(ADDRESS_OF(TTMP2));
    multiply(ADDRESS_OF(TTMP1), input, c, false);
    multiply(ADDRESS_OF(TTMP2), output, c + 2u, true);
    dxch(ADDRESS_OF(TTMP2));
    das(ADDRESS_OF(TTMP1));
    dxch(ADDRESS_OF(TTMP1));
    ddoubl();
    das(node + 2u);
    dxch(node + 2u);
    dxch(node);
    set_A(ZERO);
    ts(ADDRESS_OF(TTMP1));
    ts(node + 2u);
    multiply(ADDRESS_OF(TTMP1), input, c + 1u, false);
    multiply(node + 2u, output, c + 3u, true);
    dxch(ADDRESS_OF(TTMP1));
    das(node + 2u);
}

/* PRECOMP, "FILTER PRECOMPUTATIONS FOR NEXT PASS": the "FIRST CASCADE
 * FILTER" (TMP1, TMP2) and the "SECOND CASCADE FILTER" (TMP3, TMP4);
 * then "EXIT IF LEM OFF" (bit 13 of DAPDATR1); on, the "THIRD CASCADE
 * FILTER" (TMP5, TMP6). */
static void precomp(void)
{
    precompute_cascade(ADDRESS_OF(ERRBTMP), ADDRESS_OF(DAP1), ADDRESS_OF(N10) + 1u,
                       ADDRESS_OF(TMP1));
    precompute_cascade(ADDRESS_OF(DAP1), ADDRESS_OF(DAP2), ADDRESS_OF(N10) + 6u, ADDRESS_OF(TMP3));
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT13);
    if (ccs_register(A_ADDRESS) == POSITIVE) return; /* (A BIT13 or +0) */
    precompute_cascade(ADDRESS_OF(DAP2), ADDRESS_OF(DAP3), ADDRESS_OF(N10) + 11u, ADDRESS_OF(TMP5));
}

/* ---- The pass and the copycycles -- pp. 963-969 ---- */

/* The filter's odd or even nodes (TMP1, 3, 5 or TMP2, 4, 6) from the
 * axis's (PTMP1 on or YTMP1 on): "PREPARE THE FILTER STORAGE". */
static void prepare_nodes(const struct axis *axis, unsigned first)
{
    unsigned node;

    for (node = first; node < 12u; node += 4u) {
        dca(axis->tmp + node);
        dxch(ADDRESS_OF(TMP1) + node);
    }
}

/* PCOPY +1, YCOPY +1, the copycycle after its "RESTART-PROTECT" (INCR
 * TVCPHASE, 1 or 3): the six nodes back to the axis's; ERRBTMP, the
 * "RATE-ERROR INTEGRATOR", to PERRB or YERRB, its upper word to the
 * needles' AK1 or AK2; CMDTMP to PCMD or YCMD; DELBRTMP to DELPBAR or
 * DELYBAR. Then "PITCH COPYCYCLE COMPLETED" (INCR TVCPHASE, 2), or the
 * yaw one's, "RESET TVCPHASE" (+0). */
static void copycycle(const struct axis *axis)
{
    unsigned node;

    for (node = 0; node < 12u; node += 2u) {
        dca(ADDRESS_OF(TMP1) + node);
        dxch(axis->tmp + node);
    }
    dca(ADDRESS_OF(ERRBTMP));
    ts(axis->needle);
    dxch(axis->errb);
    ca(ADDRESS_OF(CMDTMP));
    ts(axis->cmd);
    dca(ADDRESS_OF(DELBRTMP));
    dxch(axis->delbar);
    if (axis->last) {
        set_A(ZERO);
        ts(ADDRESS_OF(TVCPHASE));
    } else {
        incr(ADDRESS_OF(TVCPHASE));
    }
}

/* The rest of a pass, from the integrated error in ERRBTMP: ERRORLIM,
 * then PFORWARD or YFORWARD. Each step goes on to the next; each starts
 * at the word one of the file's subroutines returns to (TC Q), where a
 * copycycle's TC Q can also go on. */
static void dap_pass(const struct axis *axis)
{
    errorlim();
    forward(axis);
}

/* PFORWARD, YFORWARD: the odd nodes, and FWDFLTR, "GO COMPUTE PRESENT
 * OUTPUT". */
static void forward(const struct axis *axis)
{
    prepare_nodes(axis, 0);
    fwdfltr();
    offset(axis);
}

/* POFFSET, YOFFSET: the CG offset added, "NOW SCALED AT B+0 ASCREV";
 * ACTLIM. */
static void offset(const struct axis *axis)
{
    dca(axis->deloff);
    das(ADDRESS_OF(CMDTMP));
    actlim();
    out(axis);
}

/* POUT, YOUT: the "INCREMENTAL" command (CMDTMP less PCMD or YCMD) added
 * to the outcounter, released (WOR channel 14); the even nodes, and
 * PRECOMP. */
static void out(const struct axis *axis)
{
    cs(axis->cmd);
    ad_at(ADDRESS_OF(CMDTMP));
    ads(axis->counter);
    set_A(axis->release);
    wor_channel(CHAN14);
    prepare_nodes(axis, 2u);
    precomp();
    delbar(axis);
}

/* DELBARP, DELBARY: the "OFFSET-TRACKER-FILTER", DELBRTMP, the old one
 * times E(-AT) plus CMDTMP times 1-E(-AT); the copycycle, PCOPY or YCOPY;
 * then PDAPEND or YDAPEND. */
static void delbar(const struct axis *axis)
{
    multiply(ADDRESS_OF(DELBRTMP), axis->delbar, ADDRESS_OF(E_MINUS_AT), false);
    ca(ADDRESS_OF(CMDTMP));
    mp_at(ADDRESS_OF(ONE_MINUS_E_MINUS_AT));
    das(ADDRESS_OF(DELBRTMP));
    incr(ADDRESS_OF(TVCPHASE));
    copycycle(axis);
    tc_at(Q_ADDRESS); /* its TC Q, which leaves Q 3 */
    dapend();
}

/* PDAPEND, YDAPEND: "DAP COMPLETED", RESUME. */
void dapend(void)
{
    continue_at(RESUME);
}

/* The steps, as the words they start at. */
void pforward(void)
{
    forward(&pitch);
}

void poffset(void)
{
    offset(&pitch);
}

void pout(void)
{
    out(&pitch);
}

void delbarp(void)
{
    delbar(&pitch);
}

void yforward(void)
{
    forward(&yaw);
}

void yoffset(void)
{
    offset(&yaw);
}

void yout(void)
{
    out(&yaw);
}

void delbary(void)
{
    delbar(&yaw);
}

/* The TC Q at a copycycle's end, re-entered: Q 3 (EBANK's word) the
 * faithful port refuses here. */
static void copycycle_return(void)
{
    if (overflow_corrected(Q) == Q_ADDRESS + 1u) not_modelled();
    tc_q();
}

/* PCOPY +1, where the "TVC RESTART PACKAGE" (FINCOPY) re-enters the
 * pitch copycycle: the copycycle, TC Q. */
void pcopy_reentry(void)
{
    copycycle(&pitch);
    copycycle_return();
}

/* YCOPY +1, the yaw copycycle re-entered: the copycycle, TC Q. */
void ycopy_reentry(void)
{
    copycycle(&yaw);
    copycycle_return();
}

/* ---- DAPINIT -- p. 970 ---- */

/* DAPINIT, the T5 interrupt TVCINIT4 calls: BANKRUPT kept; PITCHDAP by T5
 * "IN TVCDT SECS" (TIME5 -1 plus T5TVCDT twice plus NEGMAX, "(T5TVCDT =
 * POSMAX - TVCDT/2 +1)"; T5LOC PITCHT5: an overflow at the TS TIME5
 * skips the CAF PITCHT5, so T5LOC takes the +1 or -1 it leaves in A);
 * the "CDUS FOR DIFFERENTIATOR" "PAST-VALUES"; NOQRSM. */
void dapinit(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    set_A(NEGONE);
    ad_at(ADDRESS_OF(T5TVCDT));
    ad(NEGMAX);
    ad_at(ADDRESS_OF(T5TVCDT));
    if (!ts(ADDRESS_OF(TIME5))) set_A(PITCHT5);
    ts(ADDRESS_OF(T5LOC));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(PCDUYPST));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(PCDUZPST));
    continue_at(NOQRSM);
}

#endif
