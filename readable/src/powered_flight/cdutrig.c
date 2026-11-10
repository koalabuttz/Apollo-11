/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc, CDUTRIG to TR*GL**P (LM pp. 1259-1260,
 * CM pp. 1365-1366), bank 23.
 *
 * "CDUTRIG, CDUTRIG1, CDUTRIG2, AND CD*TR*GS ALL COMPUTE THE SINES AND
 * COSINES OF THREE 2'S COMPLEMENT ANGLES AND PLACE THE RESULT, DOUBLE
 * PRECISION, IN THE SAME ORDER AS THE INPUTS, AT SINCDU AND COSCDU. AN
 * ADDITIONAL OUTPUT IS THE 1'S COMPLEMENT ANGLES AT CDUSPOT." CDUTRIG and
 * CDUTRIGS take the angles from the CDU counters, "CD*TR*GS FINDS CDU
 * VALUES IN CDUSPOT"; CDUTRIG and CD*TR*G are called in interpretive,
 * CDUTRIGS and CD*TR*GS in basic. "SINCE AX*SR*T EXPECTS TO FIND THE SINES
 * AND COSINES IN THE ORDER Y Z X THE ANGLES MUST HAVE BEEN PLACED IN
 * CDUSPOT IN THIS ORDER."
 *
 * Each angle goes through three interpretive routines by USPRCADR
 * (CDULOGIC, COSINE and SINE +1), and the run leaves the file at each:
 * the routine after each USPRCADR's CADR is a function of its own. The
 * faithful C is c64-port/src/lm_powered_subs.c and cm_powered_subs.c. */
#include "powered_flight.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* TC at "at" to a routine outside the file, which reads the word after
 * the TC (USPRCADR's CADR). */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- The interpretive entries -- LM p. 1259 (CM p. 1365) ---- */

/* CDUTRIG and CD*TR*G: out of the interpreter to CDUTRIGS or CD*TR*GS, and
 * back in to return (RVQ). */
static const word cdutrig_words[] = {/* CDUTRIG */ ORDER(EXIT)};
static const word cdutrig_rvq_words[] = {ORDER(RVQ)};
static const word cd_star_tr_star_g_words[] = {/* CD*TR*G */ ORDER(EXIT)};
static const word cd_star_tr_star_g_rvq_words[] = {ORDER(RVQ)};

const struct interpretive_code CDUTRIG_CODE = {CODE_BANK(I_CDUTRIG), CODE_Z(I_CDUTRIG),
                                               COUNT(cdutrig_words), cdutrig_words};
const struct interpretive_code CDUTRIG_RVQ_CODE = {CODE_BANK(I_CDUTRIG),
                                                   (word)(CODE_Z(I_CDUTRIG) + 3u),
                                                   COUNT(cdutrig_rvq_words), cdutrig_rvq_words};
const struct interpretive_code CD_STAR_TR_STAR_G_CODE = {
    CODE_BANK(I_CD_STAR_TR_STAR_G), CODE_Z(I_CD_STAR_TR_STAR_G), COUNT(cd_star_tr_star_g_words),
    cd_star_tr_star_g_words};
const struct interpretive_code CD_STAR_TR_STAR_G_RVQ_CODE = {
    CODE_BANK(I_CD_STAR_TR_STAR_G), (word)(CODE_Z(I_CD_STAR_TR_STAR_G) + 3u),
    COUNT(cd_star_tr_star_g_rvq_words), cd_star_tr_star_g_rvq_words};

/* After CDUTRIG's EXIT: CDUTRIGS. */
void cdutrig_plus1(void)
{
    tc_at(CDUTRIG + 1u);
    cdutrigs();
}

/* CDUTRIGS's return: back into the interpreter, to the RVQ. */
void cdutrig_plus2(void)
{
    tc_at(CDUTRIG + 2u);
    intpret();
}

/* After CD*TR*G's EXIT: CD*TR*GS. */
void cd_star_tr_star_g_plus1(void)
{
    tc_at(CD_STAR_TR_STAR_G + 1u);
    cd_star_tr_star_gs();
}

/* CD*TR*GS's return: back into the interpreter, to the RVQ. */
void cd_star_tr_star_g_plus2(void)
{
    tc_at(CD_STAR_TR_STAR_G + 2u);
    intpret();
}

/* ---- The angles' sines and cosines -- LM pp. 1259-1260 (CM pp.
 * 1365-1366) ---- */

static void tr_star_gl_star_star_p(void);

/* CDUTRIGS: the CDU counters to CDUSPOT in the order Y, Z, X (the
 * counters themselves: "THE CONTENTS OF CDUTEMP, ETC., ARE NOT TOUCHED"),
 * then CD*TR*GS. */
void cdutrigs(void)
{
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUSPOT) + 4u);
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(CDUSPOT));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(CDUSPOT) + 2u);
    cd_star_tr_star_gs();
}

/* CD*TR*GS: the return into TEM2, and the loop from CDUSPOT +4 down. */
void cd_star_tr_star_gs(void)
{
    qxch(ADDRESS_OF(TEM2));
    set_A(FOUR);
    tr_star_gl_star_star_p();
}

/* TR*GL**P: the angle's offset ("MAKE IT EVEN AND SMALLER") into TEM3;
 * the angle into MPAC ("STORING 2'S COMP ANGLE, LOADING MPAC") and MPAC,
 * kept, in VBUF +4 ("STORING MPAC FOR LATER RESTORATION"); then CDULOGIC,
 * by USPRCADR, makes the angle one's complement. */
static void tr_star_gl_star_star_p(void)
{
    mask(SIX);
    ts(ADDRESS_OF(TEM3));
    ca(indexed(ADDRESS_OF(CDUSPOT), TEM3));
    dxch(ADDRESS_OF(MPAC));
    dxch(VBUF_AT(4u));
    tc_to(TR_STAR_GL_STAR_STAR_P + 6u, USPRCADR); /* CADR CDULOGIC */
}

/* After CDULOGIC: the one's complement angle back to CDUSPOT ("STORING 1'S
 * COMPLEMENT ANGLE"), and its COSINE by USPRCADR. */
void tr_star_gl_star_star_p_after_cdulogic(void)
{
    dca(ADDRESS_OF(MPAC));
    dxch(indexed(ADDRESS_OF(CDUSPOT), TEM3));
    tc_to(TR_STAR_GL_STAR_STAR_P + 014u, USPRCADR); /* CADR COSINE */
}

/* After COSINE: the cosine to COSCDU ("STORING COSINE"), and the angle
 * again for SINE +1 by USPRCADR ("SINE +1 EXPECTS ARGUMENT IN A AND L"). */
void tr_star_gl_star_star_p_after_cosine(void)
{
    dxch(ADDRESS_OF(MPAC));
    dxch(indexed(ADDRESS_OF(COSCDU), TEM3));
    dca(indexed(ADDRESS_OF(CDUSPOT), TEM3));
    tc_to(TR_STAR_GL_STAR_STAR_P + 024u, USPRCADR); /* CADR SINE +1 */
}

/* After SINE: MPAC back as it was ("BRINGING UP PRIOR MPAC TO BE
 * RESTORED"), the sine to SINCDU; then the next angle (CCS TEM3 leaves A
 * one less, which MASK SIX makes the next offset down), or, after the
 * last, back by TEM2. Not modelled: TEM3 negative, which TR*GL**P's MASK
 * never leaves. */
void tr_star_gl_star_star_p_after_sine(void)
{
    word to;

    dxch(VBUF_AT(4u));
    dxch(ADDRESS_OF(MPAC));
    dxch(indexed(ADDRESS_OF(SINCDU), TEM3));
    switch (ccs_at(ADDRESS_OF(TEM3))) {
    case POSITIVE: tr_star_gl_star_star_p(); return;
    case IS_PLUS_ZERO:
        to = TEM2;
        if (to < 010u || to > 07777u) not_modelled();
        tc_through(ADDRESS_OF(TEM2));
        return;
    default: not_modelled();
    }
}
