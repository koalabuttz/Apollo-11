/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc, the interface routines, from the CM's
 * NBSM to *NBSM* (LM pp. 1262-1263, CM pp. 1368-1369), bank 23.
 *
 * "THESE INTERFACE ROUTINES MAKE IT POSSIBLE TO CALL AX*SR*T, ETC., IN
 * INTERPRETIVE." The vector in MPAC goes to VBUF (MPACVBUF), AX*SR*T turns
 * it, and VLOAD brings it back to MPAC; the return is the interpreter's
 * (RVQ). "TRG*NBSM DOES THE NB TO SM TRANSFORMATION; TRG*SMNB, VICE VERSA",
 * with the sines and cosines from the angles at CDUSPOT (CD*TR*GS); CDU*NBSM
 * and CDU*SMNB from the CDU counters (CDUTRIGS); *NBSM* and *SMNB* with the
 * sines and cosines already at SINCDU and COSCDU. AX*SR*T takes the
 * direction in A: +3 from NB to SM, -3 from SM to NB.
 *
 * The CM's NBSM, older ("NBSM WILL BE THE FIRST TO GO. IT SHOULD NOT BE
 * USED."), takes the angles' address in S1 and the vector in 32D, and
 * calls TRG*NBSM.
 *
 * The run leaves the file at each TC MPACVBUF; the word after it is a
 * function of its own. The faithful C is c64-port/src/lm_powered_subs.c
 * and cm_powered_subs.c. */
#include "powered_flight.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* ---- The interpretive words ---- */

#if COMANCHE055
/* NBSM -- CM p. 1368: the return into X2, the angles from the address in
 * S1 ("BASE ADDRESS OF THE CDU ANGLES IS IN S1") to CDUSPOT, the vector
 * from 32D, TRG*NBSM; the result into 32D, and back by X2 ("SINCE THERE'S
 * NO STGOTO"). */
static const word nbsm_words[] = {
    /* NBSM */ ORDER(STQ),
    ADDRESS(X2),
    ORDERS(LXC_1, VLOAD_STAR),
    ADDRESS(S1),
    ADDRESS_PLUS_ONE_X1(0),
    STOVL(CDUSPOT),
    ADDRESS_PLUS_ONE_D(32), /* "VECTOR TO BE TRANSFORMED IS IN 32D" */
    ORDER(CALL),
    ADDRESS(TRG_STAR_NBSM),
    STCALL_D(32),
    ADDRESS(X2),
};

const struct interpretive_code NBSM_CODE = {CODE_BANK(I_NBSM), CODE_Z(I_NBSM), COUNT(nbsm_words),
                                            nbsm_words};
#endif

/* Each entry's EXIT, and C*MM*N2's way back: the vector from VBUF to MPAC
 * and the return. */
static const word cdu_star_smnb_words[] = {/* CDU*SMNB */ ORDER(EXIT)};
static const word trg_star_smnb_words[] = {/* TRG*SMNB */ ORDER(EXIT)};
static const word c_star_mm_star_n2_words[] = {ORDERS(VLOAD, RVQ), ADDRESS_PLUS_ONE(VBUF)};
static const word cdu_star_nbsm_words[] = {/* CDU*NBSM */ ORDER(EXIT)};
static const word trg_star_nbsm_words[] = {/* TRG*NBSM */ ORDER(EXIT)};
static const word star_smnb_star_words[] = {/* *SMNB* */ ORDER(EXIT)};
static const word star_nbsm_star_words[] = {/* *NBSM* */ ORDER(EXIT)};

const struct interpretive_code CDU_STAR_SMNB_CODE = {
    CODE_BANK(I_CDU_STAR_SMNB), CODE_Z(I_CDU_STAR_SMNB), COUNT(cdu_star_smnb_words),
    cdu_star_smnb_words};
const struct interpretive_code TRG_STAR_SMNB_CODE = {
    CODE_BANK(I_TRG_STAR_SMNB), CODE_Z(I_TRG_STAR_SMNB), COUNT(trg_star_smnb_words),
    trg_star_smnb_words};
const struct interpretive_code C_STAR_MM_STAR_N2_CODE = {
    CODE_BANK(I_TRG_STAR_SMNB), (word)(CODE_Z(I_TRG_STAR_SMNB) + 6u),
    COUNT(c_star_mm_star_n2_words), c_star_mm_star_n2_words};
const struct interpretive_code CDU_STAR_NBSM_CODE = {
    CODE_BANK(I_CDU_STAR_NBSM), CODE_Z(I_CDU_STAR_NBSM), COUNT(cdu_star_nbsm_words),
    cdu_star_nbsm_words};
const struct interpretive_code TRG_STAR_NBSM_CODE = {
    CODE_BANK(I_TRG_STAR_NBSM), CODE_Z(I_TRG_STAR_NBSM), COUNT(trg_star_nbsm_words),
    trg_star_nbsm_words};
const struct interpretive_code STAR_SMNB_STAR_CODE = {
    CODE_BANK(I_STAR_SMNB_STAR), CODE_Z(I_STAR_SMNB_STAR), COUNT(star_smnb_star_words),
    star_smnb_star_words};
const struct interpretive_code STAR_NBSM_STAR_CODE = {
    CODE_BANK(I_STAR_NBSM_STAR), CODE_Z(I_STAR_NBSM_STAR), COUNT(star_nbsm_star_words),
    star_nbsm_star_words};

/* ---- From SM to NB -- LM p. 1262 (CM p. 1368) ---- */

static void c_star_mm_star_n2(void);

/* After CDU*SMNB's EXIT: the sines and cosines of the CDU counters'
 * angles (CDUTRIGS), */
void cdu_star_smnb_plus1(void)
{
    tc_at(CDU_STAR_SMNB + 1u);
    cdutrigs();
}

/* and on at C*MM*N1. */
void cdu_star_smnb_plus2(void)
{
    c_star_mm_star_n1();
}

/* After TRG*SMNB's EXIT: those of CDUSPOT's angles (CD*TR*GS), on at
 * C*MM*N1. */
void trg_star_smnb_plus1(void)
{
    tc_at(TRG_STAR_SMNB + 1u);
    cd_star_tr_star_gs();
}

/* C*MM*N1: "AX*SR*T EXPECTS VECTOR IN VBUF" (MPACVBUF), */
void c_star_mm_star_n1(void)
{
    tc_at(C_STAR_MM_STAR_N1);
    continue_at(MPACVBUF);
}

/* then -3: "SIGNAL FOR SM TO NB TRANSFORMATION." */
void c_star_mm_star_n1_plus1(void)
{
    set_A(negate(THREE));
    c_star_mm_star_n2();
}

/* C*MM*N2: AX*SR*T, which goes back to the word after the TC, */
static void c_star_mm_star_n2(void)
{
    tc_at(C_STAR_MM_STAR_N2);
    ax_star_sr_star_t();
}

/* and back into the interpreter: VLOAD VBUF, RVQ. */
void c_star_mm_star_n2_plus1(void)
{
    tc_at(C_STAR_MM_STAR_N2 + 1u);
    intpret();
}

/* ---- From NB to SM -- LM pp. 1262-1263 (CM pp. 1368-1369) ---- */

/* After CDU*NBSM's EXIT: CDUTRIGS, */
void cdu_star_nbsm_plus1(void)
{
    tc_at(CDU_STAR_NBSM + 1u);
    cdutrigs();
}

/* and on at C*MM*N3. */
void cdu_star_nbsm_plus2(void)
{
    c_star_mm_star_n3();
}

/* After TRG*NBSM's EXIT: CD*TR*GS, on at C*MM*N3. */
void trg_star_nbsm_plus1(void)
{
    tc_at(TRG_STAR_NBSM + 1u);
    cd_star_tr_star_gs();
}

/* C*MM*N3: the vector to VBUF "FOR AX*SR*T", */
void c_star_mm_star_n3(void)
{
    tc_at(C_STAR_MM_STAR_N3);
    continue_at(MPACVBUF);
}

/* then +3, "SIGNAL FOR NB TO SM TRANSFORMATION", and C*MM*N2. */
void c_star_mm_star_n3_plus1(void)
{
    set_A(THREE);
    c_star_mm_star_n2();
}

/* ---- With the sines and cosines as they are -- LM p. 1263 (CM p.
 * 1369) ---- */

/* After *SMNB*'s EXIT: C*MM*N1. */
void star_smnb_star_plus1(void)
{
    c_star_mm_star_n1();
}

/* After *NBSM*'s EXIT: C*MM*N3. */
void star_nbsm_star_plus1(void)
{
    c_star_mm_star_n3();
}
