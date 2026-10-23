/* SPDX-License-Identifier: MIT */
/* The native x86_64 flight program's step (platform/x86/flight_dispatch.h,
 * docs/X86-FLIGHT.md). Compiled with RK_LM=1 or RK_CM=1 and
 * FLIGHT_HOST_TABLES, FLIGHT_HOST_SITES and FLIGHT_MCT naming the generated
 * table index, operand sets and routine costs. */
#include <string.h>
#include "flight_dispatch.h"
#include "resident_cdu.h"
#include "rope_image.h"
#include "self_check.h"
#include "standard_lead_routines.h"
#if RK_LM
#define MISSION 0u
#define FETCH LM_INTERPRETER_FETCH
#define DANZIG AGC_LM_DANZIG
#define CORE lm_interp_core
#define CORE_MCT        lm_interp_core_mct
#define CORE_IO FLIGHT_IO_P66
#define SELF_CHECK_ROPE lm_self_check_rope_data
#else
#include "cm_native_image.h"
#define MISSION 1u
#define FETCH CM_INTERPRETER_FETCH
#define DANZIG AGC_CM_DANZIG
#define CORE cm_interp_core
#define CORE_MCT        cm_interp_core_mct
#define CORE_IO FLIGHT_IO_CM_NATIVE
#define SELF_CHECK_ROPE cm_self_check_rope_data
#endif
extern const agc_routine CORE;
/* The core's work clock (src/lm_interp_core.c, built with
 * INTERP_WORD_CHARGE): each word NEWOPS fetches adds its MCT. */
extern uint16_t CORE_MCT;

#include FLIGHT_HOST_SITES
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed,mct) extern const agc_routine_table t;
#include FLIGHT_HOST_TABLES
#undef FLIGHT_TABLE
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed,mct) {&t,#t,i,k,sites,n,fixed},
const fd_table fd_tables[]={
#include FLIGHT_HOST_TABLES
};
#undef FLIGHT_TABLE
const unsigned fd_table_count=sizeof fd_tables/sizeof fd_tables[0];
static const struct { const char *name; char status; } status[]={
#include FLIGHT_STATUS
};
char fd_table_status(unsigned table){unsigned i;
 for(i=0;i<sizeof status/sizeof status[0];++i)if(!strcmp(status[i].name,fd_tables[table].name))return status[i].status;
 return 'x';}

/* Routine costs in machine cycles, by point (tools/x86_flight_mct.py: the
 * median yaAGC took for the routine at that point over the flight
 * scenarios). FD_MCT_BASIC and FD_MCT_INTERP are the medians over all
 * points, for points the scenarios never reached. */
typedef struct { uint16_t key; uint8_t interp; uint16_t mct; } fd_cost_entry;
#include FLIGHT_MCT
static double cost(uint16_t key,uint8_t interp){unsigned lo=0,hi=sizeof fd_mct/sizeof fd_mct[0],mid;
 while(lo<hi){mid=lo+(hi-lo)/2u;
  if(fd_mct[mid].interp==interp&&fd_mct[mid].key==key)return fd_mct[mid].mct;
  if(fd_mct[mid].interp<interp||(fd_mct[mid].interp==interp&&fd_mct[mid].key<key))lo=mid+1u;else hi=mid;}
 return interp?FD_MCT_INTERP:FD_MCT_BASIC;}

/* Point indexes: table index + 1 and the routine, by physical key (the
 * first table in resolution order that holds the key). */
static uint16_t at[2][65536];
#ifndef X86_CERTIFIED_ONLY
#define X86_CERTIFIED_ONLY 0
#endif
static agc_routine call_at[2][65536];

static bool rope_data(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;return apollo_rope_read(MISSION,physical,word);}

void fd_init(fd_state *st,uint8_t mode,agc_word ch30,agc_word ch31){static agc_routine_table basic[256],interp[256];unsigned i,j;uint8_t nb=0,ni=1;
 memset(st,0,sizeof *st);st->mode=mode;
 memset(at,0,sizeof at);
 for(i=0;i<fd_table_count;++i){const fd_table *e=&fd_tables[i];
  if(e->interp&&mode==FD_INTERP_CORE&&e->t!=&agc_lm_danzig_marker&&e->t!=&agc_cm_danzig_marker)continue;
#if X86_CERTIFIED_ONLY
  if(fd_table_status(i)=='u')continue;
#endif
  for(j=0;j<e->t->count;++j){uint16_t k=e->t->entries[j].physical;
   if(!at[e->interp][k]){at[e->interp][k]=(uint16_t)(i+1u);call_at[e->interp][k]=e->t->entries[j].call;}}
  /* The program a routine sees holds every table (DANZIG's marker first
   * among the interpretive ones), for routines that resolve a continuation
   * themselves. */
  if(!e->interp)basic[nb++]=*e->t;
  else if(e->t==&agc_lm_danzig_marker||e->t==&agc_cm_danzig_marker)interp[0]=*e->t;
  else interp[ni++]=*e->t;}
 st->p.basic=basic;st->p.basic_count=nb;st->p.interpretive=interp;st->p.interpretive_count=ni;
 st->p.hardware=&st->hw;st->p.fixed_data=rope_data;
 st->io.undo=&st->undo;st->io.interrupt_enabled=1;st->s.words[5]=04000u;
 st->hw.channel30=ch30;st->hw.channel31=ch31;st->hw.channel32=st->hw.channel33=077777u;
 st->next_period = 85333.0 / 400.0;
 memset(&rk_cdu, 0, sizeof rk_cdu);
}

static int interpreting(const fd_state *st){uint16_t z=agc_fixed_physical((agc_word)(st->s.words[5]&07777u),st->s.words[4],st->io.superbank);
 agc_word edop=(agc_word)(st->s.words[AGC_EDOP]&077777u);
 return z==FETCH||(z==DANZIG&&edop!=0u&&edop<040000u);}
uint16_t fd_point(const fd_state *st,int *interp){const agc_word *w=st->s.words;agc_word z=(agc_word)(w[5]&07777u);
 if((*interp=interpreting(st))!=0)return agc_fixed_physical(w[0164],w[0165],st->io.superbank);
 return z<02000u?agc_erasable_point(z,w[3]):agc_fixed_physical(z,w[4],st->io.superbank);}
unsigned long fd_centiseconds(const fd_state *st)
{
    return ((((unsigned long)(st->s.words[024] & 037777u) << 14) | (st->s.words[025] & 037777u)) -
            st->t0);
}

#if RK_CM
/* A ones-complement counter as a signed number. */
static long ones(agc_word v)
{
    return (v & 040000u) ? -(long)((~v) & 037777u) : (long)(v & 037777u);
}
#endif

/* The output drives, once each clock period at the TIME5 step, as the C64
 * resident and the host differential run them (resident_cdu.h): the IMU
 * CDU drive (CDUXCMD-CDUZCMD) and, in the CM, the optics drive, whose
 * pulses the optics error counters take (the SPS gimbal commands in TVC
 * mode; zeroed while channel 12 bit 2 is clear). */
static void drives(fd_state *st)
{
#if RK_CM
    long before[2];
    unsigned k;
    rk_cdu_count = 5u;
    for (k = 0; k < 2u; ++k) before[k] = rk_cdu.owed[3u + k] + ones(st->s.words[053u + k]);
#endif
    rk_cdu_drive(&st->s, &st->hw);
#if RK_CM
    for (k = 0; k < 2u; ++k) {
        long after = rk_cdu.owed[3u + k] + ones(st->s.words[053u + k]);
        st->optics[k] = (st->hw.channel12 & 2u) ? st->optics[k] + before[k] - after : 0;
    }
#endif
}

uint8_t fd_boundary(fd_state *st)
{
    uint8_t got;
    st->p.fixed_data = rope_data;
    st->p.operands = 0;
    st->p.operand_count = 0;
    while (!(got = rk_rupt_take(&st->p, &st->s, st->io.superbank, st->io.interrupt_enabled,
                                &st->pending)) &&
           st->mct >= st->next_period) {
        agc_word c = st->hw.channel4;
        if (st->phase == RK_PHASE_T5) drives(st);
        rk_timer_phase(&st->s, &st->hw, &st->pending, st->phase);
        if (st->phase == RK_PHASE_T5) ++st->periods;
        st->phase = st->phase == RK_PHASE_T5 ? RK_PHASE_T4 : (uint8_t)(st->phase + 1u);
        st->next_period += st->phase == RK_PHASE_T5 ? 85333.0 / 200.0 : 85333.0 / 400.0;
        if (st->downrupt && ((c & 037777u) >> 6) != ((st->hw.channel4 & 037777u) >> 6))
            st->pending |= RK_RUPT_BIT(RK_RUPT_DOWN);
    }
    return got;
}

static void gap(fd_state *st,uint16_t key,uint8_t interp,uint8_t refused,const char *by,int arrival){unsigned i;
 for(i=0;i<st->ngaps;++i)if(st->gaps[i].key==key&&st->gaps[i].interp==interp&&st->gaps[i].by==by){st->gaps[i].arrivals+=(unsigned)arrival;++st->gaps[i].slots;return;}
 if(st->ngaps<FD_GAPS){fd_gap *g=&st->gaps[st->ngaps++];g->key=key;g->interp=interp;g->refused=refused;g->by=by;g->arrivals=1;g->slots=1;}}

/* The next table after index i (resolution order) that holds key, as fd_init
 * admits tables, or 0: a compact copy pinned on the C64 (PIN_HOT in
 * tools/flight_store_build.py) refuses other callers' points there, and the
 * resident moves on to the table that takes them. */
static unsigned next_holder(const fd_state *st, unsigned i, uint8_t interp, uint16_t key,
                            agc_routine *call)
{
    unsigned j;
    for (++i; i < fd_table_count; ++i) {
        const fd_table *e = &fd_tables[i];
        if (e->interp != interp || (e->interp && st->mode == FD_INTERP_CORE)) continue;
#if X86_CERTIFIED_ONLY
        if (fd_table_status(i) == 'u') continue;
#endif
        for (j = 0; j < e->t->count; ++j)
            if (e->t->entries[j].physical == key) {
                *call = e->t->entries[j].call;
                return i + 1u;
            }
    }
    return 0;
}

static enum exec_status run(fd_state *st,agc_routine call,enum flight_io_kind kind){flight_io_buffer b;void *x;enum exec_status r;
 x=flight_io_enter(kind,&st->io,&st->hw,&b);r=call(&st->p,&st->s,x);flight_io_leave(kind,&st->io,&st->hw,&b);
 return r;}

enum exec_status fd_run(fd_state *st,fd_ran *ran){int in;uint16_t key=fd_point(st,&in);uint16_t t=at[in][key];
 static uint16_t core_key;static uint8_t core_in;
 ran->key=key;ran->interp=(uint8_t)in;ran->table=0;ran->mct=0.0;++st->steps;
 if(t){const fd_table *e=&fd_tables[t-1u];apollo_overlay_state before=st->s;flight_io io0=st->io;agc_hardware hw0=st->hw;
  st->p.operands=e->sites;st->p.operand_count=e->nsites;
  st->p.fixed_data=e->fixed==1u?SELF_CHECK_ROPE:rope_data;
  ran->table=e;ran->status=run(st,call_at[in][key],e->io);
  {
      unsigned n = t;
      agc_routine c;
      while (ran->status != EXEC_OK && !in && (n = next_holder(st, n - 1u, 0, key, &c)) != 0) {
          st->s = before;
          st->io = io0;
          st->hw = hw0;
          e = &fd_tables[n - 1u];
          t = (uint16_t)n;
          st->p.operands = e->sites;
          st->p.operand_count = e->nsites;
          st->p.fixed_data = e->fixed == 1u ? SELF_CHECK_ROPE : rope_data;
          ran->table = e;
          ran->status = run(st, c, e->io);
      }
  }
  if(ran->status==EXEC_OK){ran->mct=cost(key,(uint8_t)in);st->mct+=ran->mct;core_in=2;if(t-1u<256u)++st->uses[t-1u];return EXEC_OK;}
  /* A refusal: undo it; an interpretive point goes to the core. */
  st->s=before;st->io=io0;st->hw=hw0;
  if(!in){++st->unmapped;gap(st,key,0,1,e->name,1);return ran->status=EXEC_UNMAPPED;}
  gap(st,key,1,1,e->name,!(core_in==1&&core_key==key));}
 else if(!in){++st->unmapped;gap(st,key,0,0,0,1);return ran->status=EXEC_UNMAPPED;}
 else gap(st,key,1,0,0,!(core_in==1&&core_key==key));
 /* The interpreter core: one slot from this interpretive point. */
 st->p.operands=0;st->p.operand_count=0;st->p.fixed_data=rope_data;
 /* As tools/flight_host_run.c's core_run off the work clock: the clock
  * from 0, one slot, charged the MCT of the words it fetched. Left
  * running, the clock's bit 15 makes the core skip NEWOPS (the C64
  * module's budget), and it would never fetch a word again. */
 CORE_MCT = 0u;
 ran->status = run(st, CORE, CORE_IO);
 ++st->slots;
 if(ran->status==EXEC_UNMAPPED)ran->status=EXEC_OK;   /* left the interpreter: the next step resolves Z */
 ran->mct = (double)CORE_MCT;
 st->mct += ran->mct;
 core_key=fd_point(st,&in);core_in=(uint8_t)in;
 return ran->status;}
