/* SPDX-License-Identifier: MIT */
#include "cm_native_dispatch.h"
static cm_exec_continuation lookup_kind(const cm_exec_native *r, exec_address a,
                                     enum cm_exec_context_kind kind) {
    uint8_t i;
    if (r->context_mapping!=0) {
        for(i=0;i<r->count;++i)
            if(r->context_mapping[i].address.location==a.location &&
               r->context_mapping[i].address.bank==a.bank &&
               r->context_mapping[i].kind==kind) return r->context_mapping[i].call;
        return 0;
    }
    if(kind!=CM_EXEC_CONTEXT_BASIC) return 0;
    for (i=0; i<r->count; ++i)
        if (r->mapping[i].address.location == a.location &&
            r->mapping[i].address.bank == a.bank) return r->mapping[i].call;
    return 0;
}
static cm_exec_continuation lookup(const cm_exec_native *r,exec_address a) {
    return lookup_kind(r,a,r->current_kind);
}
int cm_exec_native_is_mapped(const cm_exec_native *r,exec_address a,
                          enum cm_exec_context_kind kind) {
    return lookup_kind(r,a,kind)!=0;
}
static enum exec_status settle(cm_exec_native *r, enum exec_status status) {
    if (status == EXEC_IDLE) { r->running=0; return status; }
    if (status == EXEC_OK && lookup(r,r->current) == 0) status=EXEC_UNMAPPED;
    if (status != EXEC_OK) { r->fault=status; r->running=0; }
    return status;
}
enum exec_status cm_exec_native_init(cm_exec_native *r, cm_executive *s,
                                 const cm_exec_mapping *m, uint8_t count, void *user) {
    uint8_t i,j;
    if (r == 0 || s == 0 || m == 0 || count == 0) return EXEC_OUT_OF_SCOPE;
    r->state=s; r->mapping=m; r->count=count; r->user=user;
    r->requested_priority=0; r->in_callback=0;
    r->context_mapping=0; r->current_kind=CM_EXEC_CONTEXT_BASIC;
    r->interpreter_control.intbit15=0; r->interpreter_control.edop=0;
    r->current.location=0; r->current.bank=0; r->running=0; r->fault=EXEC_OK;
    for (i=0; i<count; ++i) {
        if (m[i].call == 0 || m[i].address.location == 0 || m[i].address.location > 07777 ||
            m[i].address.bank > AGC_MASK) return settle(r,EXEC_OUT_OF_SCOPE);
        for (j=0; j<i; ++j)
            if (m[i].address.location == m[j].address.location &&
                m[i].address.bank == m[j].address.bank)
                return settle(r,EXEC_OUT_OF_SCOPE);
    }
    return EXEC_OK;
}
enum exec_status cm_exec_native_context_init(cm_exec_native *r,cm_executive *s,
                                         const cm_exec_context_mapping *m,
                                         uint8_t count,void *user) {
    uint8_t i,j;
    if(r==0 || s==0 || m==0 || count==0) return EXEC_OUT_OF_SCOPE;
    r->state=s; r->mapping=0; r->context_mapping=m; r->count=count; r->user=user; r->requested_priority=0; r->in_callback=0;
    r->current.location=0; r->current.bank=0; r->running=0; r->fault=EXEC_OK;
    r->current_kind=CM_EXEC_CONTEXT_BASIC;
    r->interpreter_control.intbit15=0; r->interpreter_control.edop=0;
    for(i=0;i<count;++i) {
        if(m[i].call==0 || m[i].address.location==0 || m[i].address.location>07777 ||
           (m[i].address.bank & (agc_word)~UINT16_C(076167))!=0 ||
           (m[i].kind!=CM_EXEC_CONTEXT_BASIC && m[i].kind!=CM_EXEC_CONTEXT_INTERPRETIVE))
            return settle(r,EXEC_OUT_OF_SCOPE);
        for(j=0;j<i;++j)
            if(m[i].address.location==m[j].address.location &&
               m[i].address.bank==m[j].address.bank && m[i].kind==m[j].kind)
                return settle(r,EXEC_OUT_OF_SCOPE);
    }
    return EXEC_OK;
}
static enum exec_status fail(cm_exec_native *r, enum exec_status status) {
    if (status != EXEC_OK) { r->fault=status; r->running=0; }
    return status;
}
static uint8_t candidate(const cm_executive *s,agc_word threshold) {
    uint8_t i,slot=0;
    for(i=1;i<CM_EXEC_CORE_COUNT;++i)
        if(agc_classify(s->core[i][CM_EXEC_PRIORITY])==AGC_POSITIVE &&
           s->core[i][CM_EXEC_PRIORITY]>threshold) {
            threshold=s->core[i][CM_EXEC_PRIORITY]; slot=i;
        }
    return slot;
}
static enum exec_status saved_target(const cm_exec_native *r,uint8_t slot,
                                     exec_address *a,enum cm_exec_context_kind *kind) {
    const cm_executive *s=r->state;
    a->location=s->core[slot][CM_EXEC_LOC]; a->bank=s->core[slot][CM_EXEC_BANK];
    *kind=(a->location & AGC_SIGN) ? CM_EXEC_CONTEXT_INTERPRETIVE : CM_EXEC_CONTEXT_BASIC;
    if(agc_classify(s->core[slot][CM_EXEC_PRIORITY])!=AGC_POSITIVE ||
       (a->bank & (agc_word)~UINT16_C(076167))!=0) return EXEC_OUT_OF_SCOPE;
    if(*kind==CM_EXEC_CONTEXT_INTERPRETIVE) {
        if(agc_negate(a->location)>=07777) return EXEC_OUT_OF_SCOPE;
        a->location=agc_add_word(agc_negate(a->location),1);
    } else if(a->location==0 || a->location>07777) return EXEC_OUT_OF_SCOPE;
    return lookup_kind(r,*a,*kind)==0 ? EXEC_UNMAPPED : EXEC_OK;
}
/* CHANJOB+4 and ENDPRCHG/INTRSM, shared by mixed basic dispatch boundaries.
 * Preflight is complete before this mutates the saved contexts. */
static void exchange_saved(cm_exec_native *r,uint8_t slot,exec_address outgoing,
                            exec_address incoming,enum cm_exec_context_kind kind) {
    cm_executive *s=r->state;
    agc_word debris=s->core[0][CM_EXEC_BANK];
    uint8_t j;
    s->core[slot][CM_EXEC_LOC]=outgoing.location;
    s->core[slot][CM_EXEC_BANK]=outgoing.bank;
    for(j=0;j<8;++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    if(s->ovfind!=0 && s->ovfind!=AGC_NEG_ZERO)
        s->core[0][CM_EXEC_PUSH]=agc_negate(s->core[0][CM_EXEC_PUSH]);
    for(j=CM_EXEC_PUSH;j<=CM_EXEC_PRIORITY;++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    s->fixloc=(agc_word)(s->core[0][CM_EXEC_PRIORITY]&CM_EXEC_LOW9);
    s->ovfind=0;
    if(s->core[0][CM_EXEC_PUSH]&AGC_SIGN) {
        s->core[0][CM_EXEC_PUSH]=agc_negate(s->core[0][CM_EXEC_PUSH]); s->ovfind=1;
    }
    s->newjob=0;
    s->core[0][CM_EXEC_LOC]=0; s->core[0][CM_EXEC_BANK]=debris;
    if(kind==CM_EXEC_CONTEXT_INTERPRETIVE) {
        s->core[0][CM_EXEC_LOC]=incoming.location;
        s->core[0][CM_EXEC_BANK]=(agc_word)(incoming.bank&076007);
        r->interpreter_control.intbit15=(agc_word)(incoming.bank&040000);
        r->interpreter_control.edop=0;
    }
    r->current=incoming; r->current_kind=kind;
}
static enum exec_status change_mixed_basic(cm_exec_native *r,exec_address resume) {
    uint8_t slot;
    exec_address incoming;
    enum cm_exec_context_kind kind;
    enum exec_status status;
    if(r->state->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(r->state->core[0][CM_EXEC_PRIORITY])!=AGC_POSITIVE ||
       r->state->newjob==0 || r->state->newjob>72u ||
       r->state->newjob%CM_EXEC_CORE_WORDS!=0) return EXEC_OUT_OF_SCOPE;
    slot=(uint8_t)(r->state->newjob/CM_EXEC_CORE_WORDS);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    exchange_saved(r,slot,resume,incoming,kind);
    return EXEC_OK;
}
static enum exec_status complete_mixed(cm_exec_native *r) {
    cm_executive *s=r->state;
    uint8_t slot=candidate(s,0);
    exec_address incoming,outgoing;
    enum cm_exec_context_kind kind;
    enum exec_status status;
    if(s->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(s->core[0][CM_EXEC_PRIORITY])!=AGC_POSITIVE) return EXEC_OUT_OF_SCOPE;
    if(slot==0) return cm_exec_complete_basic(s,(agc_word)(r->current.bank&0160),&r->current);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    outgoing.location=s->core[0][CM_EXEC_LOC];
    outgoing.bank=(agc_word)(s->core[0][CM_EXEC_BANK] | (r->current.bank&0160));
    (void)cm_exec_endjob_select(s);
    exchange_saved(r,slot,outgoing,incoming,kind);
    return EXEC_OK;
}
static enum exec_status sleep_mixed(cm_exec_native *r,agc_word cadr) {
    cm_executive *s=r->state;
    uint8_t slot=candidate(s,0);
    exec_address incoming,outgoing;
    enum cm_exec_context_kind kind;
    enum exec_status status;
    if(s->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(s->core[0][CM_EXEC_PRIORITY])!=AGC_POSITIVE) return EXEC_OUT_OF_SCOPE;
    if(slot==0) return cm_exec_sleep_basic(s,cadr,r->current.bank,&r->current);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    s->core[0][CM_EXEC_LOC]=cadr;
    s->core[0][CM_EXEC_PRIORITY]=agc_negate(s->core[0][CM_EXEC_PRIORITY]);
    s->core[0][CM_EXEC_BANK]=(agc_word)(r->current.bank&0177);
    s->newjob=(agc_word)(slot*CM_EXEC_CORE_WORDS);
    outgoing.location=cadr; outgoing.bank=s->core[0][CM_EXEC_BANK];
    exchange_saved(r,slot,outgoing,incoming,kind);
    return EXEC_OK;
}
static enum exec_status priority_mixed(cm_exec_native *r,exec_address resume) {
    cm_executive *s=r->state;
    agc_word packed=(agc_word)(r->requested_priority+
                               (s->core[0][CM_EXEC_PRIORITY]&CM_EXEC_LOW9));
    uint8_t slot=candidate(s,(agc_word)(packed+1u));
    exec_address incoming;
    enum cm_exec_context_kind kind;
    enum exec_status status;
    if(s->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(s->core[0][CM_EXEC_PRIORITY])!=AGC_POSITIVE) return EXEC_OUT_OF_SCOPE;
    if(slot==0) return cm_exec_priority_basic(s,r->requested_priority,resume,&r->current);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    s->newprio=r->requested_priority; s->core[0][CM_EXEC_PRIORITY]=packed;
    s->core[0][CM_EXEC_LOC]=resume.location;
    s->core[0][CM_EXEC_BANK]=(agc_word)(resume.bank&076007);
    s->newjob=(agc_word)(slot*CM_EXEC_CORE_WORDS);
    exchange_saved(r,slot,resume,incoming,kind);
    return EXEC_OK;
}
static enum exec_status start_owned(cm_exec_native *r) {
    enum exec_status status;
    if(r->state->alarm!=0) return fail(r,EXEC_BAILOUT_PENDING);
    if (r->state->newjob == AGC_NEG_ZERO) return EXEC_IDLE;
    if(r->context_mapping!=0 && r->state->newjob!=0 &&
       r->state->newjob<=72u && r->state->newjob%CM_EXEC_CORE_WORDS==0 &&
       (r->state->core[r->state->newjob/CM_EXEC_CORE_WORDS][CM_EXEC_LOC]&AGC_SIGN)) {
        uint8_t slot=(uint8_t)(r->state->newjob/CM_EXEC_CORE_WORDS);
        exec_address incoming,outgoing;
        enum cm_exec_context_kind kind;
        status=saved_target(r,slot,&incoming,&kind);
        if(status==EXEC_OK) {
            outgoing.location=r->state->core[0][CM_EXEC_LOC];
            outgoing.bank=r->state->core[0][CM_EXEC_BANK];
            r->state->core[0][CM_EXEC_LOC]=2; r->state->core[0][CM_EXEC_BANK]=0;
            exchange_saved(r,slot,outgoing,incoming,kind);
        }
    } else {
        status=cm_exec_start_basic(r->state,&r->current);
        if(status==EXEC_OK) r->current_kind=CM_EXEC_CONTEXT_BASIC;
    }
    if (status == EXEC_OK) r->running=1;
    return settle(r,status);
}
enum exec_status cm_exec_native_start(cm_exec_native *r) {
    enum exec_status status;
    if (r->fault != EXEC_OK) return r->fault;
    if (r->running != 0) return EXEC_OUT_OF_SCOPE;
    status=(r->state->alarm!=0 ? EXEC_BAILOUT_PENDING : EXEC_OK);
    if (status == EXEC_OK) status=start_owned(r);
    else status=fail(r,status);
    return status;
}
enum exec_status cm_exec_native_poll(cm_exec_native *r) {
    enum exec_status status;
    if (r->fault != EXEC_OK) return r->fault;
    status=fail(r,(r->state->alarm!=0 ? EXEC_BAILOUT_PENDING : EXEC_OK));
    return status;
}
enum exec_status cm_exec_native_admit(cm_exec_native *r, agc_word priority,
                                  exec_address address, int needs_vac) {
    enum exec_status status;
    if (r->fault != EXEC_OK) return r->fault;
    status=(r->state->alarm!=0 ? EXEC_BAILOUT_PENDING : EXEC_OK);
    if(status==EXEC_OK && r->context_mapping!=0 &&
       lookup_kind(r,address,CM_EXEC_CONTEXT_BASIC)==0) status=EXEC_UNMAPPED;
    if (status == EXEC_OK)
        status=cm_exec_admit(r->state,priority,address.location,address.bank,needs_vac);
    status=fail(r,status);
    return status;
}
enum exec_status cm_exec_native_step(cm_exec_native *r) {
    exec_address resume={0,0};
    cm_exec_continuation call=0;
    enum cm_exec_action action;
    enum exec_status status;
    if (r->fault != EXEC_OK) return r->fault;
    if (r->in_callback != 0) return EXEC_OUT_OF_SCOPE;
    status=(r->state->alarm!=0 ? EXEC_BAILOUT_PENDING : EXEC_OK);
    if (status == EXEC_OK && r->running == 0) status=start_owned(r);
    if (status == EXEC_OK) {
        call=lookup(r,r->current);
        if (call == 0) status=EXEC_UNMAPPED;
    }
    if (status != EXEC_OK && status != EXEC_IDLE) status=fail(r,status);
    if (status==EXEC_OK) { r->requested_priority=0; r->in_callback=1; }
    if (status != EXEC_OK) return status;
    action=call(r->state,r->user,&resume); /* Caller retains platform IRQ state. */
    r->in_callback=0;
    /* A callback helper may already have latched a fault. Preserve its exact
     * boundary/status; never complete or exchange that failed running job. */
    status=r->fault != EXEC_OK ? r->fault : (r->state->alarm!=0 ? EXEC_BAILOUT_PENDING : EXEC_OK);
    if (status==EXEC_OK && r->requested_priority!=0 && action!=CM_EXEC_REPRIORITIZE)
        status=EXEC_INVALID_ACTION;
    if (status == EXEC_OK) {
        if (action == CM_EXEC_FINISH) {
            if(r->current_kind!=CM_EXEC_CONTEXT_BASIC) status=EXEC_INVALID_ACTION;
            else status=r->context_mapping!=0 ? complete_mixed(r) :
                cm_exec_complete_basic(r->state,(agc_word)(r->current.bank & 0160),&r->current);
        }
        else if (action == CM_EXEC_YIELD) {
            if (resume.location == 0 || resume.location > 07777 || resume.bank > AGC_MASK)
                status=EXEC_OUT_OF_SCOPE;
            else if(r->current_kind==CM_EXEC_CONTEXT_INTERPRETIVE) {
                if(resume.location<2 || r->interpreter_control.edop!=0 ||
                   resume.bank!=(agc_word)(r->state->core[0][CM_EXEC_BANK] |
                                           (r->current.bank&0160))) status=EXEC_OUT_OF_SCOPE;
                else if(lookup_kind(r,resume,CM_EXEC_CONTEXT_INTERPRETIVE)==0) status=EXEC_UNMAPPED;
                else {
                    if(r->state->alarm!=0) status=EXEC_BAILOUT_PENDING;
                    else if(agc_classify(r->state->core[0][CM_EXEC_PRIORITY])!=AGC_POSITIVE)
                        status=EXEC_OUT_OF_SCOPE;
                    else if(r->state->newjob!=0) {
                        exec_address target;
                        enum cm_exec_context_kind target_kind;
                        if(r->state->newjob>72u || r->state->newjob%CM_EXEC_CORE_WORDS!=0)
                            status=EXEC_OUT_OF_SCOPE;
                        else status=saved_target(r,
                            (uint8_t)(r->state->newjob/CM_EXEC_CORE_WORDS),&target,&target_kind);
                    }
                    if(status==EXEC_OK) r->state->core[0][CM_EXEC_LOC]=(agc_word)(resume.location-1u);
                    if(status==EXEC_OK && r->state->newjob==0) {
                        r->state->core[0][CM_EXEC_LOC]=resume.location;
                        r->current=resume;
                    } else if(status==EXEC_OK) {
                        exec_transfer transfer;
                        status=cm_exec_change_interpretive(r->state,
                            (agc_word)(r->current.bank&0160),&r->interpreter_control,&transfer);
                        if(status==EXEC_OK) {
                            r->current=transfer.address;
                            r->current_kind=transfer.interpretive ?
                                CM_EXEC_CONTEXT_INTERPRETIVE : CM_EXEC_CONTEXT_BASIC;
                        }
                    }
                }
            } else if(r->context_mapping!=0 && lookup_kind(r,resume,CM_EXEC_CONTEXT_BASIC)==0)
                status=EXEC_UNMAPPED;
            else if (r->state->newjob == 0) { r->current=resume; status=EXEC_OK; }
            else status=r->context_mapping!=0 ? change_mixed_basic(r,resume) :
                cm_exec_change_basic(r->state,resume,&r->current);
        } else if (action==CM_EXEC_SLEEP) {
            if(r->current_kind!=CM_EXEC_CONTEXT_BASIC) status=EXEC_INVALID_ACTION;
            else if (resume.location<02000 || resume.location>03777 ||
                (resume.bank & (agc_word)~UINT16_C(076167))!=0 ||
                (resume.bank & 0177)!=(r->current.bank & 0177)) status=EXEC_OUT_OF_SCOPE;
            else if (lookup(r,resume)==0) status=EXEC_UNMAPPED;
            else {
                agc_word cadr=(agc_word)((resume.location-02000) | (resume.bank & 076000));
                status=r->context_mapping!=0 ? sleep_mixed(r,cadr) :
                    cm_exec_sleep_basic(r->state,cadr,r->current.bank,&r->current);
            }
        } else if (action==CM_EXEC_REPRIORITIZE) {
            if (r->current_kind!=CM_EXEC_CONTEXT_BASIC || r->requested_priority==0) status=EXEC_INVALID_ACTION;
            else if (lookup(r,resume)==0) status=EXEC_UNMAPPED;
            else status=r->context_mapping!=0 ? priority_mixed(r,resume) :
                cm_exec_priority_basic(r->state,r->requested_priority,resume,&r->current);
        } else if(action==CM_EXEC_ENTER_INTERPRETER || action==CM_EXEC_EXIT_INTERPRETER) {
            enum cm_exec_context_kind next_kind=action==CM_EXEC_ENTER_INTERPRETER ?
                CM_EXEC_CONTEXT_INTERPRETIVE : CM_EXEC_CONTEXT_BASIC;
            if(r->context_mapping==0 || r->current_kind==next_kind ||
               resume.location==0 || resume.location>07777 ||
               resume.bank!=r->current.bank ||
               (action==CM_EXEC_EXIT_INTERPRETER && (resume.location<2 ||
                                                r->interpreter_control.edop!=0)))
                status=EXEC_OUT_OF_SCOPE;
            else if(lookup_kind(r,resume,next_kind)==0) status=EXEC_UNMAPPED;
            else {
                r->state->core[0][CM_EXEC_LOC]=action==CM_EXEC_ENTER_INTERPRETER ?
                    resume.location : (agc_word)(resume.location-1u);
                r->state->core[0][CM_EXEC_BANK]=(agc_word)(resume.bank&076007);
                if(action==CM_EXEC_ENTER_INTERPRETER) {
                    r->interpreter_control.intbit15=(agc_word)(resume.bank&040000);
                    r->interpreter_control.edop=0;
                }
                r->current=resume; r->current_kind=next_kind;
            }
        } else status=EXEC_INVALID_ACTION;
    }
    status=settle(r,status);
    return status;
}
enum exec_status cm_exec_native_wake(cm_exec_native *r, agc_word cadr) {
    enum exec_status status;
    if (r->fault!=EXEC_OK) return r->fault;
    status=(r->state->alarm!=0 ? EXEC_BAILOUT_PENDING : EXEC_OK);
    if (status==EXEC_OK) {
        status=cm_exec_wake_basic(r->state,cadr);
        if (status==EXEC_OK) {
            uint8_t slot=(uint8_t)(r->state->locctr/CM_EXEC_CORE_WORDS);
            exec_address a={r->state->core[slot][CM_EXEC_LOC],r->state->core[slot][CM_EXEC_BANK]};
            if (lookup_kind(r,a,CM_EXEC_CONTEXT_BASIC)==0) status=EXEC_UNMAPPED;
        }
    }
    if (status!=EXEC_NOT_FOUND) status=fail(r,status);
    return status;
}
enum exec_status cm_exec_native_request_priority(cm_exec_native *r, agc_word priority) {
    enum exec_status status=EXEC_OK;
    if (r->fault!=EXEC_OK) return r->fault;
    if (r->in_callback==0 || priority==0 || (priority & UINT16_C(0140777))!=0)
        status=EXEC_OUT_OF_SCOPE;
    else r->requested_priority=priority;
    status=fail(r,status);
    return status;
}
