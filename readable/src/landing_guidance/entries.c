/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 798-799 (Luminary099 only):
 * the tables a phase picks its routines from, and the entry points.
 * WCHPHASE, the phase, is -1 (IGNALG), 0 (BRAKQUAD), 1 (APPRQUAD) or 2
 * (VERTICAL), and each table is a TCF a phase, its second word
 * BRAKQUAD's. The faithful C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* The tables' words, one TCF each: "ROUTINES FOR STARTING NEW GUIDANCE
 * PHASES" (NEWPHASE: TTFINCR, STARTP64 or P65START), "PRE-GUIDANCE
 * COMPUTATIONS" (PREGUIDE: CALCRGVG, RGVGCALC or REDESIG), "GUIDANCE
 * EQUATIONS" (WHATGUID: TTF/8CL, or VERTGUID), "POST GUIDANCE EQUATION
 * COMPUTATIONS" (AFTRGUID: CGCALC, or STEER?), "WINDOW VECTOR
 * COMPUTATIONS" (WHATEXIT: EXGSUB, EXBRAK, EXNORM), "DISPLAY ROUTINES"
 * (WHATDISP: P63DISPS, P64DISPS, VERTDISP, with no IGNALG word) and
 * "ALARM ROUTINE FOR TTF COMPUTATION" (WHATALM: 1406P00, or 1406ALM, with
 * no VERTICAL word). The tables follow each other, so a word past one
 * table is the next one's. Not modelled: a word more than one before the
 * table or two past it, and WHATALM's word for VERTICAL (TARGTDEX's
 * first, OCT 0). */
void tcf_table_word(word table, word at)
{
    if (at < table - 1u || at > table + 2u) not_modelled();
    switch (at) {
    case NEWPHASE - 1u:
    case NEWPHASE: ttfincr(); return; /* IGNALG and BRAKQUAD */
    case NEWPHASE + 1u: startp64(); return;
    case NEWPHASE + 2u: p65start(); return;
    case PREGUIDE - 1u: calcrgvg(); return;
    case PREGUIDE: rgvgcalc(); return;
    case PREGUIDE + 1u: redesig(); return;
    case PREGUIDE + 2u: rgvgcalc(); return;
    case WHATGUID - 1u:
    case WHATGUID:
    case WHATGUID + 1u: ttf_8cl(); return; /* IGNALG, BRAKQUAD and APPRQUAD */
    case WHATGUID + 2u: vertguid(); return;
    case AFTRGUID - 1u:
    case AFTRGUID:
    case AFTRGUID + 1u: cgcalc(); return; /* IGNALG, BRAKQUAD and APPRQUAD */
    case AFTRGUID + 2u: steer(); return;  /* VERTICAL */
    case WHATEXIT - 1u: exgsub(); return;
    case WHATEXIT: exbrak(); return;
    case WHATEXIT + 1u: exnorm(); return;
    case WHATDISP: p63disps(); return;
    case WHATDISP + 1u: p64disps(); return;
    case WHATDISP + 2u: vertdisp(); return;
    case WHATALM - 1u: p00_1406(); return;
    case WHATALM:
    case WHATALM + 1u: alm1406(); return;
    default: not_modelled();
    }
}

/* INDEX at; TCF table: the table's word for the phase at holds. The AGC
 * adds the index to the whole TCF; not modelled: an index that makes it
 * another instruction. */
void tcf_by_phase(word table, unsigned at)
{
    word instruction = indexed_instruction((word)(010000u | table), fetch(at));

    if (OPERATION_CODE(instruction) != 1u) not_modelled();
    tcf_table_word(table, ADDRESS_FIELD(instruction));
}

/* ?GUIDSUB: "IGNITION ALGORITHM ENTRY: DELIVERS N PASSES OF QUADRATIC
 * GUIDANCE". The ignition algorithm calls it from interpretive code, so
 * it starts with an EXIT. */
static const word q_guidsub_words[] = {
    /* ?GUIDSUB */ ORDER(EXIT),
};

const struct interpretive_code Q_GUIDSUB_CODE = {031u, Q_GUIDSUB, COUNT(q_guidsub_words),
                                                 q_guidsub_words};

/* ?GUIDSUB +1: NGUIDSUB gets two ("N = 3"), and the pass starts at
 * GUILDRET +2, past the R13 monitor. (A, a constant, never overflows: TS NGUIDSUB
 * does not skip.) */
void q_guidsub_plus1(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(NGUIDSUB)); /* "N = 3" */
    guildret_plus2();
}

/* GUIDSUB: "ON SUCCEEDING PASSES SKIP TTFINCR": NGUIDSUB gets A, the
 * passes the ignition algorithm still wants, and the guidance runs from
 * CALCRGVG. (EXGSUB's CCS NGUIDSUB leaves A one less than a positive
 * count, which never overflows: TS NGUIDSUB does not skip into LUNLAND.) */
void guidsub(void)
{
    ts(ADDRESS_OF(NGUIDSUB));
    calcrgvg();
}

/* LUNLAND: "NORMAL ENTRY: CONTROL COMES HERE FROM SERVOUT". Its first
 * PHASCHNG: "GROUP 5: RETAIN ONLY PIPA TASK". */
void lunland(void)
{
    tc_to(LUNLAND, PHASCHNG);
}

/* Its second: "GROUP 3: PROTECT GUIDANCE WITH PRIO 21", "JUST HIGHER
 * THAN SERVICER'S PRIORITY"; then GUILDEN (guildenstern.c). */
void lunland_plus2(void)
{
    tc_to(LUNLAND + 2u, PHASCHNG);
}

#endif
