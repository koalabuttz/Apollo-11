/* SPDX-License-Identifier: MIT
 * Translation of public-domain Luminary099/EXECUTIVE.agc.
 * See docs/SOURCE_MAP.md for exact scope and stop boundaries. */
#include "executive.h"
void exec_init(executive *s) {
    uint8_t i, j;
    for (i=0; i<EXEC_CORE_COUNT; ++i) {
        for (j=0; j<EXEC_CORE_WORDS; ++j) s->core[i][j] = 0;
        s->core[i][EXEC_PRIORITY] = AGC_NEG_ZERO;
    }
    for (i=0; i<EXEC_VAC_COUNT; ++i) {
        for (j=0; j<EXEC_VAC_WORDS; ++j) s->vac[i][j] = 0;
        s->vac[i][0] = (agc_word)(EXEC_VAC1USE + i * EXEC_VAC_WORDS);
    }
    s->newjob=AGC_NEG_ZERO; s->newprio=0;
    s->newloc[0]=0; s->newloc[1]=0;
    s->locctr=0; s->fixloc=0; s->ovfind=0; s->alarm=0;
}
enum exec_status exec_admit(executive *s, agc_word priority,
                           agc_word location, agc_word bank, int needs_vac) {
    uint8_t i, candidate;
    agc_word scratch_l=0, scratch_a;
    agc_register difference;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (priority == 0 || (priority & UINT16_C(0140777)) != 0 ||
        location > AGC_MASK || bank > AGC_MASK ||
        (s->newjob != AGC_NEG_ZERO &&
         (s->newjob > 84u || s->newjob % EXEC_CORE_WORDS != 0)))
        return EXEC_OUT_OF_SCOPE;
    s->newprio=priority; s->newloc[0]=location; s->newloc[1]=bank;
    if (needs_vac) {
        /* FINDVAC2: scan actual use words in order. +address is free. */
        for (i=0; i<EXEC_VAC_COUNT; ++i)
            if (agc_classify(s->vac[i][0]) == AGC_POSITIVE) break;
        if (i == EXEC_VAC_COUNT) { s->alarm=01201; return EXEC_NO_VAC; }
        scratch_l=s->vac[i][0];
        /* CCS gives use-address-1; AD TWO gives VAC base. LXCH reserves. */
        s->newprio=agc_correct(agc_add_register(agc_extend(s->newprio),
                         agc_extend((agc_word)(s->vac[i][0]+1u))));
        s->vac[i][0]=0;
    } else {
        /* NOVAC uses MPAC-36, not a generic "no work area" bit. */
        s->newprio=agc_correct(agc_add_register(agc_extend(priority),
                                              agc_extend(EXEC_FAKEPRET)));
    }
    /* NOVAC2 / NOVAC3 / NEXTCORE: -0 only means available. */
    s->locctr=0;
    for (i=0; i<EXEC_ADMISSION_COUNT; ++i) {
        if (s->core[i][EXEC_PRIORITY] == AGC_NEG_ZERO) break;
        s->locctr=(agc_word)(s->locctr + EXEC_CORE_WORDS);
    }
    if (i == EXEC_ADMISSION_COUNT) {
        /* The assembly does NOT release a VAC acquired above before bailout. */
        s->alarm=01202; return EXEC_NO_CORE;
    }
    /* CORFOUND */
    s->core[i][EXEC_PRIORITY]=s->newprio;
    s->core[i][EXEC_PUSH]=(agc_word)(s->newprio & EXEC_LOW9);
    if (i == 0) {
        s->ovfind=0; s->fixloc=s->core[0][EXEC_PUSH];
        /* SPECTEST: idle -0 -> +0; a queued job keeps NEWJOB unchanged. */
        if (s->newjob == AGC_NEG_ZERO) {
            s->newjob=0;
            s->core[0][EXEC_LOC]=location; s->core[0][EXEC_BANK]=bank;
            s->newloc[0]=0; s->newloc[1]=scratch_l;
            return EXEC_OK;
        }
    }
    /* SETLOC: first DXCH puts A/L debris into NEWLOC; second leaves the
     * previous slot's 2CADR in A/L, not in NEWLOC. */
    scratch_a=i==0 ? agc_diminished_abs(s->newjob) : (agc_word)(s->locctr-1u);
    s->core[i][EXEC_LOC]=location; s->core[i][EXEC_BANK]=bank;
    s->newloc[0]=scratch_a; s->newloc[1]=scratch_l;
    candidate=s->newjob == AGC_NEG_ZERO ? 0 : (uint8_t)(s->newjob / EXEC_CORE_WORDS);
    difference=agc_add_register(agc_extend(agc_negate(s->core[candidate][EXEC_PRIORITY])),
                               agc_extend(s->newprio));
    /* BZMF examines accumulator sign and either zero, before overflow correction. */
    if (difference != 0 && difference != UINT16_C(0177777) &&
        (difference & UINT16_C(0100000)) == 0) s->newjob=s->locctr;
    return EXEC_OK;
}
int exec_endjob_select(executive *s) {
    uint8_t i;
    agc_word base;
    int selected=-1;
    agc_word highest=0;
    if (s->alarm != 0) return -1;
    /* ENDJOB1: release core zero, then its VAC if low bits exceed FAKEPRET. */
    base=(agc_word)(s->core[0][EXEC_PRIORITY] & EXEC_LOW9);
    s->core[0][EXEC_PRIORITY]=AGC_NEG_ZERO;
    if (base > EXEC_FAKEPRET) {
        for (i=0; i<EXEC_VAC_COUNT; ++i)
            if (base == (agc_word)(EXEC_VAC1USE + i * EXEC_VAC_WORDS + 1))
                s->vac[i][0]=(agc_word)(base - 1u);
    }
    /* EJSCAN / EJ1 / EJ2. Full packed priority comparison, first wins ties.
     * Valid admitted jobs have priority >=01000, so diminished abs stays >0. */
    for (i=1; i<EXEC_CORE_COUNT; ++i) {
        agc_word p=s->core[i][EXEC_PRIORITY];
        if (agc_classify(p) == AGC_POSITIVE && p > highest) {
            highest=p; selected=(int)i;
        }
    }
    if (selected >= 0) s->newjob=(agc_word)((uint16_t)selected * EXEC_CORE_WORDS);
    /* DUMMYJOB's -0 store belongs to the next boundary, not this one. */
    return selected;
}
