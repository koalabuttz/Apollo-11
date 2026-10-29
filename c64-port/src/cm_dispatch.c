/* SPDX-License-Identifier: MIT
 * Comanche055 Executive context translation; see COMANCHE_DISPATCH.md. */
#include "cm_dispatch.h"
static int basic_slot(const cm_executive *s, uint8_t slot) {
    return agc_classify(s->core[slot][CM_EXEC_PRIORITY]) == AGC_POSITIVE &&
           s->core[slot][CM_EXEC_LOC] > 0 && s->core[slot][CM_EXEC_LOC] <= 07777 &&
           s->core[slot][CM_EXEC_BANK] <= AGC_MASK;
}
static void exchange_basic(cm_executive *s, uint8_t slot, exec_address outgoing,
                           exec_address *next) {
    uint8_t j;
    agc_word debris_bank=s->core[0][CM_EXEC_BANK];
    /* CHANJOB+4: outgoing LOC/BBCON arrive in A/L; current LOC/BANKSET
     * are scratch, and need not equal that pair. */
    next->location=s->core[slot][CM_EXEC_LOC]; next->bank=s->core[slot][CM_EXEC_BANK];
    s->core[slot][CM_EXEC_LOC]=outgoing.location;
    s->core[slot][CM_EXEC_BANK]=outgoing.bank;
    for (j=0; j<8; ++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    if (s->ovfind != 0 && s->ovfind != AGC_NEG_ZERO)
        s->core[0][CM_EXEC_PUSH]=agc_negate(s->core[0][CM_EXEC_PUSH]);
    for (j=CM_EXEC_PUSH; j<=CM_EXEC_PRIORITY; ++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    s->fixloc=(agc_word)(s->core[0][CM_EXEC_PRIORITY] & CM_EXEC_LOW9);
    s->ovfind=0;
    if ((s->core[0][CM_EXEC_PUSH] & AGC_SIGN) != 0) {
        s->core[0][CM_EXEC_PUSH]=agc_negate(s->core[0][CM_EXEC_PUSH]); s->ovfind=1;
    }
    s->newjob=0;
    /* ENDPRCHG DXCH LOC takes the selected pair into A/L for DTCB.
     * A=+0 and L=the old current BANKSET at this boundary. */
    s->core[0][CM_EXEC_LOC]=0; s->core[0][CM_EXEC_BANK]=debris_bank;
}
enum exec_status cm_exec_change_basic(cm_executive *s, exec_address resume,
                                  exec_address *next) {
    uint8_t slot;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || s->newjob == 0 || s->newjob > 72u ||
        s->newjob % CM_EXEC_CORE_WORDS != 0 ||
        resume.location == 0 || resume.location > 07777 || resume.bank > AGC_MASK ||
        agc_classify(s->core[0][CM_EXEC_PRIORITY]) != AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    slot=(uint8_t)(s->newjob / CM_EXEC_CORE_WORDS);
    if (!basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    exchange_basic(s,slot,resume,next);
    return EXEC_OK;
}
enum exec_status cm_exec_change_interpretive(cm_executive *s, agc_word superbank,
                                         exec_interpreter *control,
                                         exec_transfer *next) {
    uint8_t slot,interpretive;
    exec_address outgoing,incoming;
    if (s->alarm!=0) return EXEC_BAILOUT_PENDING;
    if (control==0 || next==0 || control->edop!=0 ||
        (superbank & (agc_word)~UINT16_C(0160))!=0 || s->newjob==0 ||
        s->newjob>72u || s->newjob%CM_EXEC_CORE_WORDS!=0 ||
        s->core[0][CM_EXEC_LOC]==0 || s->core[0][CM_EXEC_LOC]>07777 ||
        (s->core[0][CM_EXEC_BANK] & (agc_word)~UINT16_C(076167))!=0 ||
        agc_classify(s->core[0][CM_EXEC_PRIORITY])!=AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    slot=(uint8_t)(s->newjob/CM_EXEC_CORE_WORDS);
    incoming.location=s->core[slot][CM_EXEC_LOC];
    incoming.bank=s->core[slot][CM_EXEC_BANK];
    interpretive=(uint8_t)((incoming.location & AGC_SIGN)!=0);
    if (agc_classify(s->core[slot][CM_EXEC_PRIORITY])!=AGC_POSITIVE ||
        (incoming.bank & (agc_word)~UINT16_C(076167))!=0 ||
        (!interpretive && (incoming.location==0 || incoming.location>07777)) ||
        (interpretive && agc_negate(incoming.location)>=07777))
        return EXEC_OUT_OF_SCOPE;
    /* CHANG2 complements LOC; CHANJOB-1 combines BANKSET with SUPERBNK. */
    outgoing.location=agc_negate(s->core[0][CM_EXEC_LOC]);
    outgoing.bank=(agc_word)(s->core[0][CM_EXEC_BANK] | superbank);
    exchange_basic(s,slot,outgoing,&incoming);
    next->interpretive=interpretive;
    if (interpretive) {
        /* ENDPRCHG COM / AD ONE / TS LOC, then INTRSM's LXCH BBANK and
         * INTPRET+3. EDOP's right-shift edit strips BIT15 to +0. */
        incoming.location=agc_add_word(agc_negate(incoming.location),1);
        s->core[0][CM_EXEC_LOC]=incoming.location;
        s->core[0][CM_EXEC_BANK]=(agc_word)(incoming.bank & 076007);
        control->intbit15=(agc_word)(incoming.bank & 040000);
        control->edop=0;
    }
    next->address=incoming;
    return EXEC_OK;
}
enum exec_status cm_exec_complete_basic(cm_executive *s, agc_word superbank,
                                    exec_address *next) {
    uint8_t i,slot=0;
    agc_word highest=0;
    exec_address outgoing;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || (superbank & (agc_word)~UINT16_C(0160)) != 0 ||
        agc_classify(s->core[0][CM_EXEC_PRIORITY]) != AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    /* Preflight unsupported interpreter dispatch without partially completing
     * a job. The mutation below still uses the original EJSCAN algorithm. */
    for (i=1; i<CM_EXEC_CORE_COUNT; ++i)
        if (agc_classify(s->core[i][CM_EXEC_PRIORITY]) == AGC_POSITIVE &&
            s->core[i][CM_EXEC_PRIORITY] > highest) {
            highest=s->core[i][CM_EXEC_PRIORITY]; slot=i;
        }
    if (slot != 0 && !basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    outgoing.location=s->core[0][CM_EXEC_LOC];
    outgoing.bank=(agc_word)(s->core[0][CM_EXEC_BANK] | superbank);
    if (cm_exec_endjob_select(s) < 0) {
        s->newjob=AGC_NEG_ZERO; next->location=0; next->bank=0;
        return EXEC_IDLE;
    }
    exchange_basic(s,slot,outgoing,next);
    return EXEC_OK;
}
enum exec_status cm_exec_start_basic(cm_executive *s, exec_address *next) {
    uint8_t slot;
    exec_address outgoing;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || s->newjob > 72u || s->newjob % CM_EXEC_CORE_WORDS != 0)
        return EXEC_OUT_OF_SCOPE;
    slot=(uint8_t)(s->newjob / CM_EXEC_CORE_WORDS);
    if (!basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    outgoing.location=s->core[0][CM_EXEC_LOC]; outgoing.bank=s->core[0][CM_EXEC_BANK];
    s->core[0][CM_EXEC_LOC]=2; s->core[0][CM_EXEC_BANK]=0;
    if (slot != 0) exchange_basic(s,slot,outgoing,next);
    else *next=outgoing;
    return EXEC_OK;
}
static int lifecycle_bank(agc_word bank) {
    return (bank & (agc_word)~UINT16_C(076167)) == 0;
}
static uint8_t lifecycle_candidate(const cm_executive *s, agc_word threshold) {
    uint8_t i,slot=0;
    agc_word highest=threshold;
    for (i=1;i<CM_EXEC_CORE_COUNT;++i)
        if (agc_classify(s->core[i][CM_EXEC_PRIORITY]) == AGC_POSITIVE &&
            s->core[i][CM_EXEC_PRIORITY] > highest) {
            highest=s->core[i][CM_EXEC_PRIORITY]; slot=i;
        }
    return slot;
}
enum exec_status cm_exec_sleep_basic(cm_executive *s, agc_word cadr, agc_word bank,
                                 exec_address *next) {
    uint8_t slot;
    exec_address outgoing;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next == 0 || cadr > AGC_MASK || !lifecycle_bank(bank) ||
        agc_classify(s->core[0][CM_EXEC_PRIORITY]) != AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    slot=lifecycle_candidate(s,0);
    if (slot != 0 && !basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    /* JOBSLEEP/JOBSLP1. FBANK has changed, but BBANK's low bits survive. */
    s->core[0][CM_EXEC_LOC]=cadr;
    s->core[0][CM_EXEC_PRIORITY]=agc_negate(s->core[0][CM_EXEC_PRIORITY]);
    s->core[0][CM_EXEC_BANK]=(agc_word)(bank & 0177);
    if (slot == 0) {
        s->newjob=AGC_NEG_ZERO; next->location=0; next->bank=0;
        return EXEC_IDLE;
    }
    s->newjob=(agc_word)(slot*CM_EXEC_CORE_WORDS);
    outgoing.location=cadr; outgoing.bank=s->core[0][CM_EXEC_BANK];
    exchange_basic(s,slot,outgoing,next);
    return EXEC_OK;
}
enum exec_status cm_exec_wake_basic(cm_executive *s, agc_word cadr) {
    uint8_t slot,candidate;
    agc_word location,bank,scratch_a;
    agc_register difference;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (cadr > AGC_MASK || (s->newjob != AGC_NEG_ZERO &&
        (s->newjob > 72u || s->newjob % CM_EXEC_CORE_WORDS != 0)))
        return EXEC_OUT_OF_SCOPE;
    for (slot=0;slot<CM_EXEC_CORE_COUNT;++slot) {
        agc_word loc=s->core[slot][CM_EXEC_LOC];
        if (agc_classify(s->core[slot][CM_EXEC_PRIORITY]) == AGC_NEGATIVE &&
            (loc==cadr || ((loc==0 || loc==AGC_NEG_ZERO) &&
                           (cadr==0 || cadr==AGC_NEG_ZERO)))) break;
    }
    if (slot<CM_EXEC_CORE_COUNT &&
        (!lifecycle_bank(s->core[slot][CM_EXEC_BANK]) ||
         s->core[slot][CM_EXEC_BANK]>0177 || (slot==0 && s->newjob==0)))
        return EXEC_OUT_OF_SCOPE;
    s->newloc[0]=cadr;
    if (slot==CM_EXEC_CORE_COUNT) {
        s->locctr=UINT16_C(077776); /* CS ONE: -1, not free-core -0. */
        return EXEC_NOT_FOUND;
    }
    s->locctr=(agc_word)(slot*CM_EXEC_CORE_WORDS);
    s->newprio=agc_negate(s->core[slot][CM_EXEC_PRIORITY]);
    s->core[slot][CM_EXEC_PRIORITY]=s->newprio;
    location=(agc_word)((cadr & 01777)+02000);
    bank=agc_add_word((agc_word)(cadr & 076000),s->core[slot][CM_EXEC_BANK]);
    s->newloc[0]=location; s->newloc[1]=bank;
    if (slot==0 && s->newjob==AGC_NEG_ZERO) {
        /* WAKETEST enters SPECTEST directly, so FIXLOC/OVFIND are retained. */
        s->newjob=0; s->core[0][CM_EXEC_LOC]=location; s->core[0][CM_EXEC_BANK]=bank;
        s->newloc[0]=0; s->newloc[1]=0;
        return EXEC_OK;
    }
    /* SETLOC with the adapted caller L=+0. CCS supplies displacement-1. */
    scratch_a=agc_diminished_abs(slot==0 ? s->newjob : s->locctr);
    s->core[slot][CM_EXEC_LOC]=location; s->core[slot][CM_EXEC_BANK]=bank;
    s->newloc[0]=scratch_a; s->newloc[1]=0;
    candidate=s->newjob==AGC_NEG_ZERO ? 0 : (uint8_t)(s->newjob/CM_EXEC_CORE_WORDS);
    difference=agc_add_register(agc_extend(agc_negate(s->core[candidate][CM_EXEC_PRIORITY])),
                               agc_extend(s->newprio));
    if (difference!=0 && difference!=UINT16_C(0177777) &&
        (difference & UINT16_C(0100000))==0) s->newjob=s->locctr;
    return EXEC_OK;
}
enum exec_status cm_exec_priority_basic(cm_executive *s, agc_word priority,
                                    exec_address resume, exec_address *next) {
    uint8_t slot;
    agc_word packed;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (next==0 || priority==0 || (priority & UINT16_C(0140777))!=0 ||
        resume.location==0 || resume.location>07777 || !lifecycle_bank(resume.bank) ||
        agc_classify(s->core[0][CM_EXEC_PRIORITY])!=AGC_POSITIVE)
        return EXEC_OUT_OF_SCOPE;
    packed=(agc_word)(priority+(s->core[0][CM_EXEC_PRIORITY] & CM_EXEC_LOW9));
    /* Initial BUF+1=-packed; EJ1 compares CCS's candidate priority-1.
     * Thus a candidate only one raw unit above current does not displace it. */
    slot=lifecycle_candidate(s,(agc_word)(packed+1u));
    if (slot!=0 && !basic_slot(s,slot)) return EXEC_OUT_OF_SCOPE;
    s->newprio=priority;
    s->core[0][CM_EXEC_PRIORITY]=packed;
    s->core[0][CM_EXEC_LOC]=resume.location;
    s->core[0][CM_EXEC_BANK]=(agc_word)(resume.bank & 076007);
    if (slot==0) {
        s->newjob=0; s->core[0][CM_EXEC_LOC]=0; s->core[0][CM_EXEC_BANK]=0;
        *next=resume; /* Channel 7 persists; it is not in the stored BBANK word. */
        return EXEC_OK;
    }
    s->newjob=(agc_word)(slot*CM_EXEC_CORE_WORDS);
    exchange_basic(s,slot,resume,next);
    return EXEC_OK;
}
