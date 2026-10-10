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
    agc_word scratch_l=s->newloc[1], scratch_a;
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
static int basic_slot(const executive *s, uint8_t slot) {
    return agc_classify(s->core[slot][EXEC_PRIORITY]) == AGC_POSITIVE &&
           s->core[slot][EXEC_LOC] > 0 && s->core[slot][EXEC_LOC] <= 07777 &&
           s->core[slot][EXEC_BANK] <= AGC_MASK;
}
static void exchange_basic(executive *s, uint8_t slot, exec_address outgoing,
                           exec_address *next) {
    uint8_t j;
    agc_word debris_bank=s->core[0][EXEC_BANK];
    /* CHANJOB+4: outgoing LOC/BBCON arrive in A/L; current LOC/BANKSET
     * are scratch, and need not equal that pair. */
    next->location=s->core[slot][EXEC_LOC]; next->bank=s->core[slot][EXEC_BANK];
    s->core[slot][EXEC_LOC]=outgoing.location;
    s->core[slot][EXEC_BANK]=outgoing.bank;
    for (j=0; j<8; ++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    if (s->ovfind != 0 && s->ovfind != AGC_NEG_ZERO)
        s->core[0][EXEC_PUSH]=agc_negate(s->core[0][EXEC_PUSH]);
    for (j=EXEC_PUSH; j<=EXEC_PRIORITY; ++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    s->fixloc=(agc_word)(s->core[0][EXEC_PRIORITY] & EXEC_LOW9);
    s->ovfind=0;
    if ((s->core[0][EXEC_PUSH] & AGC_SIGN) != 0) {
        s->core[0][EXEC_PUSH]=agc_negate(s->core[0][EXEC_PUSH]); s->ovfind=1;
    }
    s->newjob=0;
    /* ENDPRCHG DXCH LOC takes the selected pair into A/L for DTCB.
     * A=+0 and L=the old current BANKSET at this boundary. */
    s->core[0][EXEC_LOC]=0; s->core[0][EXEC_BANK]=debris_bank;
}
enum exec_status exec_change_basic(executive *s, exec_address resume,
                                  exec_address *next) {
    uint8_t slot;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || s->newjob == 0 || s->newjob > 84u ||
        s->newjob % EXEC_CORE_WORDS != 0 ||
        resume.location == 0 || resume.location > 07777 || resume.bank > AGC_MASK ||
        agc_classify(s->core[0][EXEC_PRIORITY]) != AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    slot=(uint8_t)(s->newjob / EXEC_CORE_WORDS);
    if (!basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    exchange_basic(s,slot,resume,next);
    return EXEC_OK;
}
enum exec_status exec_complete_basic(executive *s, agc_word superbank,
                                    exec_address *next) {
    uint8_t i,slot=0;
    agc_word highest=0;
    exec_address outgoing;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || (superbank & (agc_word)~UINT16_C(0160)) != 0 ||
        agc_classify(s->core[0][EXEC_PRIORITY]) != AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    /* Preflight unsupported interpreter dispatch without partially completing
     * a job. The mutation below still uses the original EJSCAN algorithm. */
    for (i=1; i<EXEC_CORE_COUNT; ++i)
        if (agc_classify(s->core[i][EXEC_PRIORITY]) == AGC_POSITIVE &&
            s->core[i][EXEC_PRIORITY] > highest) {
            highest=s->core[i][EXEC_PRIORITY]; slot=i;
        }
    if (slot != 0 && !basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    outgoing.location=s->core[0][EXEC_LOC];
    outgoing.bank=(agc_word)(s->core[0][EXEC_BANK] | superbank);
    if (exec_endjob_select(s) < 0) {
        s->newjob=AGC_NEG_ZERO; next->location=0; next->bank=0;
        return EXEC_IDLE;
    }
    exchange_basic(s,slot,outgoing,next);
    return EXEC_OK;
}
enum exec_status exec_start_basic(executive *s, exec_address *next) {
    uint8_t slot;
    exec_address outgoing;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || s->newjob > 84u || s->newjob % EXEC_CORE_WORDS != 0)
        return EXEC_OUT_OF_SCOPE;
    slot=(uint8_t)(s->newjob / EXEC_CORE_WORDS);
    if (!basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    outgoing.location=s->core[0][EXEC_LOC]; outgoing.bank=s->core[0][EXEC_BANK];
    s->core[0][EXEC_LOC]=2; s->core[0][EXEC_BANK]=0;
    if (slot != 0) exchange_basic(s,slot,outgoing,next);
    else *next=outgoing;
    return EXEC_OK;
}
static int lifecycle_bank(agc_word bank) {
    return (bank & (agc_word)~UINT16_C(076167)) == 0;
}
static uint8_t lifecycle_candidate(const executive *s, agc_word threshold) {
    uint8_t i,slot=0;
    agc_word highest=threshold;
    for (i=1;i<EXEC_CORE_COUNT;++i)
        if (agc_classify(s->core[i][EXEC_PRIORITY]) == AGC_POSITIVE &&
            s->core[i][EXEC_PRIORITY] > highest) {
            highest=s->core[i][EXEC_PRIORITY]; slot=i;
        }
    return slot;
}
enum exec_status exec_sleep_basic(executive *s, agc_word cadr, agc_word bank,
                                 exec_address *next) {
    uint8_t slot;
    exec_address outgoing;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || cadr > AGC_MASK || !lifecycle_bank(bank) ||
        agc_classify(s->core[0][EXEC_PRIORITY]) != AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    slot=lifecycle_candidate(s,0);
    if (slot != 0 && !basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    /* JOBSLEEP/JOBSLP1. FBANK has changed, but BBANK's low bits survive. */
    s->core[0][EXEC_LOC]=cadr;
    s->core[0][EXEC_PRIORITY]=agc_negate(s->core[0][EXEC_PRIORITY]);
    s->core[0][EXEC_BANK]=(agc_word)(bank & 0177);
    if (slot == 0) {
        s->newjob=AGC_NEG_ZERO; next->location=0; next->bank=0;
        return EXEC_IDLE;
    }
    s->newjob=(agc_word)(slot*EXEC_CORE_WORDS);
    outgoing.location=cadr; outgoing.bank=s->core[0][EXEC_BANK];
    exchange_basic(s,slot,outgoing,next);
    return EXEC_OK;
}
enum exec_status exec_wake_basic(executive *s, agc_word cadr) {
    uint8_t slot,candidate;
    agc_word location,bank,scratch_a;
    agc_register difference;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (cadr > AGC_MASK || (s->newjob != AGC_NEG_ZERO &&
        (s->newjob > 84u || s->newjob % EXEC_CORE_WORDS != 0)))
        return EXEC_OUT_OF_SCOPE;
    for (slot=0;slot<EXEC_CORE_COUNT;++slot) {
        agc_word loc=s->core[slot][EXEC_LOC];
        if (agc_classify(s->core[slot][EXEC_PRIORITY]) == AGC_NEGATIVE &&
            (loc==cadr || ((loc==0 || loc==AGC_NEG_ZERO) &&
                           (cadr==0 || cadr==AGC_NEG_ZERO)))) break;
    }
    if (slot<EXEC_CORE_COUNT &&
        (!lifecycle_bank(s->core[slot][EXEC_BANK]) ||
         s->core[slot][EXEC_BANK]>0177 || (slot==0 && s->newjob==0)))
        return EXEC_OUT_OF_SCOPE;
    s->newloc[0]=cadr;
    if (slot==EXEC_CORE_COUNT) {
        s->locctr=UINT16_C(077776); /* CS ONE: -1, not free-core -0. */
        return EXEC_NOT_FOUND;
    }
    s->locctr=(agc_word)(slot*EXEC_CORE_WORDS);
    s->newprio=agc_negate(s->core[slot][EXEC_PRIORITY]);
    s->core[slot][EXEC_PRIORITY]=s->newprio;
    location=(agc_word)((cadr & 01777)+02000);
    bank=agc_add_word((agc_word)(cadr & 076000),s->core[slot][EXEC_BANK]);
    s->newloc[0]=location; s->newloc[1]=bank;
    if (slot==0 && s->newjob==AGC_NEG_ZERO) {
        /* WAKETEST enters SPECTEST directly, so FIXLOC/OVFIND are retained. */
        s->newjob=0; s->core[0][EXEC_LOC]=location; s->core[0][EXEC_BANK]=bank;
        s->newloc[0]=0; s->newloc[1]=0;
        return EXEC_OK;
    }
    /* SETLOC with the adapted caller L=+0. CCS supplies displacement-1. */
    scratch_a=agc_diminished_abs(slot==0 ? s->newjob : s->locctr);
    s->core[slot][EXEC_LOC]=location; s->core[slot][EXEC_BANK]=bank;
    s->newloc[0]=scratch_a; s->newloc[1]=0;
    candidate=s->newjob==AGC_NEG_ZERO ? 0 : (uint8_t)(s->newjob/EXEC_CORE_WORDS);
    difference=agc_add_register(agc_extend(agc_negate(s->core[candidate][EXEC_PRIORITY])),
                               agc_extend(s->newprio));
    if (difference!=0 && difference!=UINT16_C(0177777) &&
        (difference & UINT16_C(0100000))==0) s->newjob=s->locctr;
    return EXEC_OK;
}
enum exec_status exec_priority_basic(executive *s, agc_word priority,
                                    exec_address resume, exec_address *next) {
    uint8_t slot;
    agc_word packed;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next==0 || priority==0 || (priority & UINT16_C(0140777))!=0 ||
        resume.location==0 || resume.location>07777 || !lifecycle_bank(resume.bank) ||
        agc_classify(s->core[0][EXEC_PRIORITY])!=AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    packed=(agc_word)(priority+(s->core[0][EXEC_PRIORITY] & EXEC_LOW9));
    /* Initial BUF+1=-packed; EJ1 compares CCS's candidate priority-1.
     * Thus a candidate only one raw unit above current does not displace it. */
    slot=lifecycle_candidate(s,(agc_word)(packed+1u));
    if (slot!=0 && !basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    s->newprio=priority;
    s->core[0][EXEC_PRIORITY]=packed;
    s->core[0][EXEC_LOC]=resume.location;
    s->core[0][EXEC_BANK]=(agc_word)(resume.bank & 076007);
    if (slot==0) {
        s->newjob=0; s->core[0][EXEC_LOC]=0; s->core[0][EXEC_BANK]=0;
        *next=resume; /* Channel 7 persists; it is not in the stored BBANK word. */
        return EXEC_OK;
    }
    s->newjob=(agc_word)(slot*EXEC_CORE_WORDS);
    exchange_basic(s,slot,resume,next);
    return EXEC_OK;
}
