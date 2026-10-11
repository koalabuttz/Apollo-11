/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_internal.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register neg(agc_register a){return (agc_register)(a^0177777u);}
static int reg_kind(agc_register a){int v=agc_overflow(a);return v>0?(int)AGC_POSITIVE:v<0?(int)AGC_NEGATIVE:(int)agc_classify(agc_correct(a));}
static agc_register dabs(agc_register a){a=(a&0100000u)?neg(a):a;return a==0?0:(agc_register)(a-1u);}
static enum cm_wl_status schedule_source(cm_waitlist *s,agc_word delay,exec_address task) {
    agc_register a;
    agc_word insert;
    uint8_t slot=0,i;
    int kind;
    if(s->alarm!=0) return CM_WL_ALARM_PENDING;
    if(delay>AGC_MASK || s->time3>037777 || task.location==0 || task.location>07777 ||
       (task.bank & (agc_word)~UINT16_C(076167))!=0) return CM_WL_OUT_OF_SCOPE;
    if(delay==0 || (delay & AGC_SIGN)!=0) { s->alarm=CM_WL_BAD_DELAY; return CM_WL_BAD_DELAY; }
    /* WAIT2 forms TD-T1+1, handling a pending TIME3 overflow (<0200).
     * Register arithmetic keeps the CCS zeros and uncorrected sign bits. */
    a=add(agc_extend(agc_negate(s->time3)),0200);
    kind=reg_kind(a); a=dabs(a);
    if(kind==AGC_POSITIVE) { a=add(a,040001); a=neg(a); }
    else if(kind==AGC_PLUS_ZERO) a=neg(a);
    if(kind!=AGC_MINUS_ZERO) a=add(a,040201);
    a=add(a,delay);
    kind=reg_kind(a); a=dabs(a);
    if(kind==AGC_POSITIVE) {
        a=add(a,s->interval[0]);
        /* WTLST5: equal deadlines insert after existing tasks. */
        for(slot=1;slot<CM_WL_TASK_COUNT;++slot) {
            kind=reg_kind(a); a=dabs(a);
            if(kind==AGC_NEGATIVE || kind==AGC_MINUS_ZERO) break;
            if(slot==CM_WL_INTERVAL_COUNT) break;
            /* CCS +0 skips AD LST1 and goes directly to the next CCS. */
            if(kind==AGC_POSITIVE) a=add(a,s->interval[slot]);
        }
        if(slot==CM_WL_TASK_COUNT || (slot==CM_WL_INTERVAL_COUNT &&
           kind==AGC_POSITIVE)) {
            s->alarm=CM_WL_FULL; return CM_WL_FULL;
        }
        /* Last CCS +0 skips FILLED via NOOP/AD ONE; -0 skips AD ONE.
         * WTLST2 splits the interval, preserving raw zero encoding. */
        if(kind==AGC_NEGATIVE || kind==AGC_PLUS_ZERO) a=add(a,1);
        if(agc_overflow(a)>0)slot=1;
        insert=agc_correct(neg(a));
        s->interval[slot-1]=agc_correct(add(add(agc_extend(agc_correct(a)),1),s->interval[slot-1]));
    } else {
        /* Earlier than task 1: replace TIME3, insert old remaining difference. */
        a=add(add(agc_extend(agc_negate(delay)),020000),020000);
        insert=agc_correct(add(add(agc_extend(s->time3),040000),delay));
        s->time3=agc_correct(a); slot=0;
    }
    for(i=CM_WL_INTERVAL_COUNT-1;i>slot;--i) s->interval[i]=s->interval[i-1];
    if(slot<CM_WL_INTERVAL_COUNT) s->interval[slot]=insert;
    {
        exec_address discarded=s->task[CM_WL_TASK_COUNT-1];
        for(i=CM_WL_TASK_COUNT-1;i>slot;--i) s->task[i]=s->task[i-1];
        s->task[slot]=task;
        if(discarded.location!=CM_WL_END_LOCATION) { s->alarm=CM_WL_FULL; return CM_WL_FULL; }
    }
    return CM_WL_OK;
}

enum cm_wl_status cm_waitlist_internal(apollo_overlay_state *s,cm_native_io *io,enum cm_waitlist_internal_entry entry,agc_word literal){
 cm_waitlist w;agc_register a;agc_word delay,caller,bank,temp=0,discarded_loc,discarded_bank;exec_address task;uint8_t slot=0;int cls;enum cm_wl_status status;
 static const agc_word pcs[3]={05255,05272,05267};
 if(!s||!io||(unsigned)entry>=3u||s->words[5]!=pcs[entry]||s->words[3]>03400u||(s->words[3]&0377u)||(s->words[4]&~076000u)||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||s->words[7]||io->interrupt_enabled||(io->superbank&~0160u))return CM_WL_OUT_OF_SCOPE;
 bank=s->words[6];
 if(entry==CM_WL_DLY2_MINUS1){if(agc_overflow(s->words[0])||agc_overflow(s->words[1])||agc_overflow(s->words[2]))return CM_WL_OUT_OF_SCOPE;delay=agc_correct(s->words[2]);caller=s->words[061];task.location=agc_correct(s->words[0]);task.bank=agc_correct(s->words[1]);}
 else{if(s->words[2]<0100u||s->words[2]>07776u||agc_overflow(s->words[0]))return CM_WL_OUT_OF_SCOPE;delay=entry==CM_WL_FIXDELAY?literal:agc_correct(s->words[0]);caller=015322;task.location=(agc_word)(s->words[2]+(entry==CM_WL_FIXDELAY));task.bank=(agc_word)(bank|io->superbank);}
 if(!delay||delay>037777u||task.location<0100u||task.location>07777u||(task.bank&~076167u)||caller>AGC_MASK)return CM_WL_OUT_OF_SCOPE;
 cm_native_gather_waitlist(s,io,&w);
 if(w.alarm||w.time3>037777u)return CM_WL_OUT_OF_SCOPE;
 a=add(agc_extend(agc_negate(w.time3)),0200);cls=reg_kind(a);a=dabs(a);
 if(cls==AGC_POSITIVE)a=neg(add(a,040001));else if(cls==AGC_PLUS_ZERO)a=neg(a);
 if(cls!=AGC_MINUS_ZERO)a=add(a,040201);
 a=add(a,delay);cls=reg_kind(a);a=dabs(a);
 if(cls==AGC_POSITIVE){a=add(a,w.interval[0]);
  for(slot=1;slot<CM_WL_TASK_COUNT;++slot){cls=reg_kind(a);a=dabs(a);if(cls==AGC_NEGATIVE||cls==AGC_MINUS_ZERO||slot==CM_WL_INTERVAL_COUNT)break;if(cls==AGC_POSITIVE)a=add(a,w.interval[slot]);}
  if(slot==CM_WL_INTERVAL_COUNT&&cls==AGC_PLUS_ZERO){s->words[061]=caller;s->words[062]=bank;s->words[063]=task.location;s->words[0]=0;s->words[1]=agc_extend(task.bank);s->words[2]=03377;s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=01203;io->interrupt_enabled=0;return CM_WL_SOURCE_OVERFLOW;}
  if(slot==CM_WL_TASK_COUNT||(slot==CM_WL_INTERVAL_COUNT&&cls==AGC_POSITIVE)){s->words[061]=caller;s->words[062]=bank;s->words[063]=task.location;s->words[0]=a;s->words[1]=agc_extend(task.bank);s->words[2]=03376;s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=07755;io->interrupt_enabled=0;return CM_WL_FULL;}
  if(cls==AGC_NEGATIVE||cls==AGC_PLUS_ZERO)a=add(a,1);
  temp=agc_correct(a);if(agc_overflow(a)>0)slot=1;
 }
 discarded_loc=w.task[8].location;discarded_bank=w.task[8].bank;status=schedule_source(&w,delay,task);if(status!=CM_WL_OK&&status!=CM_WL_FULL)return status;w.alarm=io->waitlist_alarm;
 cm_native_commit_waitlist(s,io,&w);
 if(slot)s->words[064]=temp;
 s->words[061]=077777;s->words[062]=discarded_bank;s->words[063]=task.location;
 if(status==CM_WL_FULL){s->words[061]=caller;s->words[062]=bank;s->words[0]=add(agc_extend(discarded_loc),agc_negate(CM_WL_END_LOCATION));s->words[1]=agc_extend(discarded_bank);s->words[2]=03376;s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=07755;io->interrupt_enabled=0;return status;}
 s->words[0]=05266;s->words[1]=02003;s->words[2]=slot;s->words[5]=(agc_word)(agc_correct(add(caller,2))&07777u);
 s->words[6]=bank;s->words[3]=(agc_word)((bank&7u)<<8);s->words[4]=(agc_word)(bank&076000u);io->interrupt_enabled=0;
 return CM_WL_OK;
}
