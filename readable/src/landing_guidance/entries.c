/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 798-799 (Luminary099 only):
 * the tables a phase picks its routines from, and the entry points.
 * WCHPHASE, the phase, is -1 (IGNALG), 0 (BRAKQUAD), 1 (APPRQUAD) or 2
 * (VERTICAL), and each table is a TCF a phase, its second word
 * BRAKQUAD's. So far the tables the guidance's exits use (AFTRGUID,
 * WHATEXIT and WHATDISP) and GUIDSUB. The faithful C is
 * c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"

/* The tables' words, one TCF each: "POST GUIDANCE EQUATION
 * COMPUTATIONS" (AFTRGUID: CGCALC, or STEER? when VERTICAL), "WINDOW
 * VECTOR COMPUTATIONS" (WHATEXIT: EXGSUB, EXBRAK, EXNORM) and "DISPLAY
 * ROUTINES" (WHATDISP: P63DISPS, P64DISPS, VERTDISP, the last table of
 * three, with no IGNALG word). The tables follow each other, so a word
 * past one table is the next one's. Not modelled: a word more than one
 * before the table or two past it. */
void tcf_table_word(word table, word at)
{
    if (at < table - 1u || at > table + 2u) not_modelled();
    switch (at) {
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

/* GUIDSUB: "ON SUCCEEDING PASSES SKIP TTFINCR": NGUIDSUB gets A, the
 * passes the ignition algorithm still wants, and the guidance runs from
 * CALCRGVG. A TS that overflows skips into LUNLAND. */
void guidsub(void)
{
    if (ts(ADDRESS_OF(NGUIDSUB)))
        lunland();
    else
        calcrgvg();
}

/* LUNLAND: "NORMAL ENTRY: CONTROL COMES HERE FROM SERVOUT". Its first
 * PHASCHNG: "GROUP 5: RETAIN ONLY PIPA TASK". */
void lunland(void)
{
    tc_to(LUNLAND, PHASCHNG);
}

/* CALCRGVG (p. 806): "IN IGNALG, COMPUTE V FROM INTEGRATION", the
 * interpretive code's TC INTPRET. */
void calcrgvg(void)
{
    tc_at(CALCRGVG);
    intpret();
}

#endif
