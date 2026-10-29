/* SPDX-License-Identifier: MIT
 * Bounded translations of public-domain Luminary099 alarm/restart routines. */
#include "restart.h"
enum rs_status rs_record(restart_state *s, enum rs_alarm_entry entry,
                         agc_word code, exec_address caller) {
    uint8_t i;
    if (entry>RS_POODOO1 || entry<RS_ALARM1 || code>037777 ||
        caller.location>AGC_MASK || caller.bank>AGC_MASK ||
        s->failreg[0]>037777 || s->failreg[1]>037777 || s->failreg[2]>AGC_MASK)
        return RS_OUT_OF_SCOPE;
    s->almcadr[0]=caller.location; s->almcadr[1]=caller.bank;
    for(i=0;i<2;++i) if(s->failreg[i]==0) break;
    if(i<2) {
        s->failreg[i]=code;
        if(i==0) s->dsptab[11]=(agc_word)(s->dsptab[11] | 040400);
    } else if((s->failreg[2] & 037777)==0) s->failreg[2]=code;
    else s->failreg[2]=agc_correct(agc_add_register(agc_extend(code),agc_extend(040000)));
    return RS_OK;
}
void rs_cleanup(restart_state *s, executive *e, waitlist *w) {
    uint8_t i;
    w->time3=037777; s->word[RS_TIME4]=037775; s->word[RS_TIME5]=037774;
    s->word[RS_RCSFLAGS]&=067777;
    s->word[RS_T6NEXT]=037777;
    if((s->channel13 & 010000)!=0) s->trap31b=1;
    if((s->channel13 & 020000)!=0) s->trap32=1;
    s->trap31a=1; /* Later WOR BIT12 always enables the channel-31 trap. */
    s->channel13 &= 003777; /* Channel-13 trap enables do not read back. */
    s->word[RS_NXT6ADR]=0; s->word[RS_NEXTP]=0;
    s->word[RS_DAPBOOLS]&=077773;
    s->word[RS_T5ADR]=02024; s->word[RS_T5ADR1]=034066;
    s->channel11 &= 030001;
    s->word[RS_FLAGWRD3]&=077377;
    s->word[RS_RADMODES]&=(s->word[RS_FLAGWRD3] & 02000)!=0 ? 045776 : 044776;
    s->channel12 &= 027470;
    s->word[RS_FLAGWRD5]&=075767;
    s->channel13 &= 074160;
    s->channel14 &= 040;
    for(i=0;i<WL_INTERVAL_COUNT;++i) w->interval[i]=WL_DEFAULT_INTERVAL;
    for(i=0;i<WL_TASK_COUNT;++i) {
        w->task[i].location=WL_END_LOCATION; w->task[i].bank=WL_END_BANK;
    }
    for(i=0;i<EXEC_CORE_COUNT;++i) e->core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
    s->word[RS_DSRUPTSW]=AGC_NEG_ZERO; e->newjob=AGC_NEG_ZERO;
    for(i=0;i<EXEC_VAC_COUNT;++i) e->vac[i][0]=(agc_word)(EXEC_VAC1USE+i*EXEC_VAC_WORDS);
    for(i=0;i<11;++i) s->dsptab[i]=073777;
    e->core[0][EXEC_MPAC]=0; /* Last CCS MPAC in DSPOFF. */
    for(i=RS_DELAYLOC;i<=RS_FLAGWRD4;++i) s->word[i]=0;
    s->word[RS_NOUT]=013; s->word[RS_SAMPLIM]=077776;
    s->word[RS_IMODES33]=(agc_word)((s->word[RS_IMODES33] & 040)+016000);
    s->word[RS_SELFRET]=03336; s->word[RS_DSPCOUNT]=077754;
    /* Native markers are outside AGC erasable memory. */
    e->alarm=0; w->alarm=0;
}
enum rs_status rs_validate(executive *e, const restart_phases *p) {
    uint8_t i=6;
    while(i!=0) {
        --i; e->core[0][5]=i;
        if((agc_word)(p->complement[i] ^ p->phase[i])!=AGC_NEG_ZERO)
            return RS_PHASE_BAD;
    }
    e->core[0][6]=0;
    return RS_OK;
}
static int job_supported(const restart_phases *p,uint8_t i) {
    agc_word phase=p->phase[i], priority=p->priority[i];
    agc_word corrected=(priority & AGC_SIGN)!=0 ? agc_negate(priority) : priority;
    return phase<=01777 && (phase & 01407)==00402 && corrected!=0 &&
        (corrected & 0140777)==0 && p->address[i].location>0 &&
        p->address[i].location<=07777 && p->address[i].bank<=AGC_MASK;
}
enum exec_status rs_job(executive *e, const restart_phases *p, uint8_t group) {
    agc_word priority;
    if(group>=6 || !job_supported(p,group)) return EXEC_OUT_OF_SCOPE;
    if(e->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(e->core[0][EXEC_PRIORITY]==AGC_NEG_ZERO && e->newjob!=AGC_NEG_ZERO)
        return EXEC_OUT_OF_SCOPE; /* SPECTEST would take CCSHOLE, outside this route. */
    e->core[0][0]=p->phase[group]; e->core[0][1]=(agc_word)(group*2u);
    e->core[0][3]=03763; e->core[0][5]=group;
    e->vac[4][21]=p->address[group].location;
    e->vac[4][22]=p->address[group].bank; e->vac[4][23]=04631;
    priority=p->priority[group];
    e->vac[4][20]=(priority & AGC_SIGN)!=0 ? 05072 : 05105;
    return exec_admit(e,(priority & AGC_SIGN)!=0 ? agc_negate(priority) : priority,
                      p->address[group].location,p->address[group].bank,
                      (priority & AGC_SIGN)==0);
}
enum rs_status rs_recover(restart_state *s, executive *e, waitlist *w,
                          const restart_phases *p, exec_address caller) {
    uint8_t i; agc_word code=e->alarm!=0 ? e->alarm : w->alarm;
    enum rs_status status;
    if(e->alarm!=0 && w->alarm!=0) return RS_OUT_OF_SCOPE; /* One pending boundary per recovery. */
    if(code!=01201 && code!=01202 && code!=01203) return RS_OUT_OF_SCOPE;
    for(i=0;i<6;++i) if(p->phase[i]!=0 && !job_supported(p,i)) return RS_OUT_OF_SCOPE;
    status=rs_record(s,RS_BAILOUT1,code,caller);
    if(status!=RS_OK) return status;
    rs_cleanup(s,e,w); status=rs_validate(e,p);
    if(status!=RS_OK) return status;
    i=6;
    while(i!=0) {
        enum exec_status admitted;
        --i;
        if(p->phase[i]==0) continue;
        admitted=rs_job(e,p,i);
        if(admitted!=EXEC_OK) return RS_RESOURCE_ALARM; /* Exact admission state retained. */
    }
    return RS_OK;
}
