/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only whole-program differential (docs/FLIGHT-STORE.md), linked to
 * Virtual AGC. NEVER part of the native port.
 *
 * Every lane's routine tables, in the resolution order
 * tools/flight_store_build.py writes (build/flight-store/<m>-host.inc), run
 * from GOJAM (Z = 04000) on a zeroed erasable image, beside yaAGC run from
 * the same cold start.
 *
 * Native side: each point resolves to the first table that holds it; the
 * table's routine runs with its own io record (include/flight_io.h), and
 * rk_rupt_take vectors a pending interrupt at a routine boundary, as the
 * flight resident does. Each routine reads inline operands from its source's
 * own site arrays (build/flight-store/<m>-sites.inc), as its module does on
 * the C64; only the self-check module reads the rope image. Module loads
 * per slot are counted. Every routine is checked in lockstep: yaAGC runs
 * from the state the routine started in until it reaches the routine's
 * continuation with the same erasable memory; the routine took that long
 * (the clock counters, the scaler and the interrupt requests come from
 * yaAGC), and a routine yaAGC never matches is reported.
 * A point no table holds, or one whose routine refuses (the refusal is
 * undone), is bridged by yaAGC instruction by instruction, with its clock
 * and its interrupts, until a point some table holds (or idle); each such
 * point is reported.
 *
 * Reference side: yaAGC from the same cold start with its own clock, without
 * DOWNRUPT (the native side has no downlink yet).
 * FLIGHT_DOWNRUPT=1: both sides take a DOWNRUPT every 20 ms of AGC time and
 * the channel 34/35 word streams are compared (tools/flight_downlink.h).
 *
 * Both are sampled at the first idle pass through ADVAN at or after each centisecond
 * 1..CS and their erasable memory compared, without the central registers
 * (0-017), the clock counters (024-031) and RUPTREG1-4 (070-073). DSKY keys (KEYS, e.g. V37E12E)
 * are pressed on both sides from 2 s, one every half second.
 *
 * FLIGHT_SCALED=K,D,LAG[,SEED]: scaled time (docs/RESIDENT-FLIGHT.md), the
 * replay proof. yaAGC's own clock is frozen (AgcFrozenClock: no scaler, no
 * counter cycles), and the AGC clock runs only as the flight resident runs
 * it: a modeled wall clock advances by each span's AGC machine cycles times
 * K (the C64's slowdown; SEED, when given, multiplies each span by a random
 * factor from 1/4 to 4) and by 20 ms for each module swap, CIA1 interrupts
 * land every D * 10 ms of it, and at
 * each native boundary at most LAG of them become rk_timer_tick periods
 * (TIME1-TIME6, the scalers and the radar cycle), the rest dropped
 * (rk_rupts); yaAGC's own output drives (the IMU CDU and gyro drives,
 * timed by its cycle counter and scaler, and its optics dump) stop with
 * it, and the C64 resident's (drives()) run on both sides. Bridged code
 * gets the same periods between its instructions. Every routine must then
 * match yaAGC run from the same state with the clock frozen, the clock
 * counters included, so the run is one yaAGC run under the same tick
 * schedule. No free-running reference is run.
 * FLIGHT_SCALED=W: the work clock instead (resident_flight.h): each native
 * routine spends its table's weight (host.inc, from docs/flight-routine-mct.json), bridged code its
 * own machine cycles, and every 853 1/3 MCT of that is one tick, none
 * dropped, in three steps (rk_timer_phase) with a request that can be
 * taken taken before each, as rk_rupts does; every routine must still
 * match yaAGC under that schedule.
 * FLIGHT_WORD_OUT=PATH: the reference run (yaAGC alone) writes one line per
 * interpretive word it runs (word_out, below), for the interpreter's work
 * clock weight (tools/interp_word_mct.py), and the run ends there.
 * FLIGHT_CHAN=CS:CH=VALUE,...: an input channel set at a time (below);
 * keys M and J are the CM's MARK and MARK REJECT buttons.
 *
 * FLIGHT_GOJAM=CS[,CS...]: a hardware restart at the first sample point at
 * or after each CS, on both sides (tools/flight_gojam.h).
 * FLIGHT_SNAPSHOT=CS,PATH: at the first native sample point at or after
 * centisecond CS (after that point's pokes), write the native state (the
 * erasable image, the superbank, the interrupt enable, agc_hardware and the
 * flight_io channels) as a C header for the C64 DAP pass check
 * (platform/c64/dap_pass_check.c), and stop. It waits for a point with no
 * interrupt pending, unless FLIGHT_SNAPSHOT_PENDING is set: then it takes the
 * first point, the one FLIGHT_GOJAM would restart at (the requests pending
 * there are not in the header; the GOJAM clears them).
 * FLIGHT_REPLAY=CS: with FLIGHT_SNAPSHOT, go on for a C64 window of CS
 * centiseconds and write its inputs and end state (below, rp_row).
 *   flight-host-run ROPE.bin [CS [KEYS]]   (build with -DRK_LM=1 or -DRK_CM=1,
 *   FLIGHT_HOST_TABLES and FLIGHT_HOST_SITES naming the generated files) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "flight_io.h"
#include "resident_rupt.h"
#include "resident_cdu.h"
#include "rope_image.h"
#include "self_check.h"
#include "standard_lead_routines.h"
#include "interp_word_mct.h"
#include "interp_runtime.h"
#include "flight_gojam.h"
#include "../platform/x86/night_watchman.h"
#include "rupt_hold.h"
/* FLIGHT_DOWNRUPT (tools/flight_downlink.h): wraps agc_engine and rk_timer_tick. */
#include "flight_downlink.h"
/* FLIGHT_RR (tools/flight_radar.h): the rendezvous radar and attitude model. */
#include "flight_radar.h"
#if RK_LM
#define MISSION 0u
#define FETCH LM_INTERPRETER_FETCH
#define DANZIG AGC_LM_DANZIG
#else
#define MISSION 1u
#define FETCH CM_INTERPRETER_FETCH
#define DANZIG AGC_CM_DANZIG
#endif
#define NEWJOB 067u
/* Idle: no interrupt in progress, NEWJOB -0, and Z in DUMMYJOB..NUDIRECT or
 * in bank 43 (SELFCHK's backup idle loop). */
/* ADVAN: the sample point (and where keys are pressed), a native routine
 * entry every idle pass goes through on both sides. */
#if RK_LM
#define ADVAN 03214u
#define IDLE_LO 03206u
#define IDLE_HI 03225u
#else
#define ADVAN 03231u
#define IDLE_LO 03223u
#define IDLE_HI 03242u
#endif
/* ADVAN is in bank 01. */
static int at_advan(const agc_word *w,agc_word sb){agc_word z=(agc_word)(w[5]&07777u);
 return z==ADVAN&&agc_fixed_physical(z,w[4],sb)==1024u+(ADVAN-02000u);}
static int idle(const agc_word *w,agc_word sb,int isr){agc_word z=(agc_word)(w[5]&07777u);uint16_t f;
 if(isr||(w[NEWJOB]&077777u)!=077777u)return 0;
 if(z>=IDLE_LO&&z<IDLE_HI)return 1;
 f=z>=02000u?agc_fixed_physical(z,w[4],sb):AGC_TRANSFER_NONE;
 return f!=AGC_TRANSFER_NONE&&f/1024u==043u;}
#include FLIGHT_HOST_SITES
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed,mct) extern const agc_routine_table t;
#include FLIGHT_HOST_TABLES
#undef FLIGHT_TABLE
typedef struct { const agc_routine_table *t; const char *name; uint8_t interp; enum flight_io_kind io; const char *module; uint8_t slot;
                 const agc_site_operand *sites; uint16_t nsites; const char *also; uint8_t fixed; uint16_t mct; } entry;
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed,mct) {&t,#t,i,k,mod,slot,sites,n,also,fixed,mct},
static const entry tables[]={
#include FLIGHT_HOST_TABLES
};
#undef FLIGHT_TABLE
#define TABLES (sizeof tables/sizeof tables[0])
/* A table left to the interpreter module (weight FLIGHT_PER_WORD,
 * tools/resident_flight_build.py INTERP_PER_WORD) spends its words' MCT
 * instead, as the compact interpreter charges them at NEWOPS
 * (include/interp_word_mct.h): lockstep counts them (word_mct). */
#define FLIGHT_PER_WORD 0xFFFFu
/* A FLIGHT_INTERP_STOP file's basic table (INTERP_PLUS_WORDS): its C64
 * module stops at the interpretive code its host routine runs on through,
 * so a routine spends its weight (the low bits) and the words it ran. */
#define FLIGHT_PLUS_WORDS 0x8000u
/* idle_weight's answer for an idle-loop pass: the work clock's idle. */
#define FLIGHT_IDLE_TICK 0xFFFEu
static unsigned weight(const entry *e){return e->mct;}

/* Virtual AGC adapter: no peripherals; channel 7 as SocketAPI.c stores it. */
void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){if(c==7)s->InputChannel[7]=s->OutputChannel7=v&0160;}
void ChannelRoutine(agc_t *s){(void)s;}
void RequestRadarData(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int data){(void)s;(void)data;}
void UnblockSocket(int n){(void)n;}
static agc_t loaded,m;
static int16_t *cell(unsigned a){return &m.Erasable[a/0400u][a%0400u];}
static int requests(void){int i;for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(m.InterruptRequests[i])return 1;return 0;}
static int quiet(void){return !m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode&&!m.SubstituteInstruction;}
/* FLIGHT_SCALED (above). The wall clock is in AGC machine cycles (85,333 a
 * second, so 853.33 per 10 ms). */
extern int AgcFrozenClock;
static int scaled;
static double slow=1.0,wall,swap_mct=1707.0;
static unsigned divisor=1u,lag=2u;
static unsigned long seed,consumed,taken,dropped,span_mct;
/* FLIGHT_SCALED=W: the work clock (platform/c64/resident_flight.h). Each
 * native routine spends its table's weight (host.inc), bridged code its
 * own machine cycles, and every period of it is a tick; none dropped. */
static int work;
static double jit_lo=0.25,jit_span=16.0;
static double jitter(void){if(!seed)return 1.0;
 seed^=seed<<13;seed^=seed>>7;seed^=seed<<17;return jit_lo*pow(jit_span,(double)(seed%10000ul)/9999.0);}
static void spend(unsigned long mct){wall+=(double)mct*slow*jitter();}
/* FLIGHT_STEAL=T0-T1:N[,T0-T1:N...] (centiseconds; N MCT): counter cycles
 * the program loses, as Apollo 11's RR CDU counts took about 13% of the
 * LGC in P63/P64 (the 1201/1202 alarms). From the first clock period at or
 * after T0 to the last before T1, each period (each work-clock T5 step)
 * also spends N MCT that no routine runs in, so the Executive's jobs get
 * less time; the counted cells themselves are left alone. */
static unsigned nsteal;static unsigned long steal_t[8][3],steal_cs;
static int steal_parse(const char *t){
 while(t&&*t){int n=0;
  if(nsteal>=8u||sscanf(t,"%lu-%lu:%lu%n",&steal_t[nsteal][0],&steal_t[nsteal][1],&steal_t[nsteal][2],&n)<3||!n
     ||steal_t[nsteal][1]<=steal_t[nsteal][0])return 0;
  ++nsteal;t+=n;if(*t==',')++t;else if(*t)return 0;}
 return 1;}
static void steal(void){unsigned i;
 for(i=0;i<nsteal;++i)if(steal_cs>=steal_t[i][0]&&steal_cs<steal_t[i][1]){wall+=(double)steal_t[i][2]*slow;return;}}
/* The work clock's steps (resident_flight.h rk_phase_mct): step j (from 0)
 * ends at step_at(j) MCT and steps TIME4, TIME1/TIME3 or TIME5 (j % 3, the
 * rk_timer_phase step); consumed counts the steps taken. */
static const unsigned step_end[9]={214u,427u,854u,1067u,1280u,1707u,1920u,2134u,2560u};
static double step_at(unsigned long j){return (double)(2560ul*(j/9ul)+step_end[j%9ul]);}
/* FLIGHT_HOLD: the interrupt hold meter (include/rupt_hold.h) over the
 * replay window, on the work clock, as -DRK_HOLD_METER runs it on the C64. */
static rupt_hold hold;static int hold_on;
static void hold_report(void){if(getenv("FLIGHT_HOLD"))printf("interrupt hold: max %u MCT at TIME1 %05o\n",(unsigned)hold.max,(unsigned)hold.max_at);}
static uint8_t take_held(const agc_native_program *p,apollo_overlay_state *s,flight_io *io,agc_hardware *hw,uint16_t *pending,double t){
 if(hold_on)rupt_hold_boundary(&hold,(uint16_t)(unsigned long)t,*pending,(uint8_t)(hw->in_interrupt||!io->interrupt_enabled||agc_overflow(s->words[0])));
 return rk_rupt_take(p,s,io->superbank,io->interrupt_enabled,pending);}
/* The work clock's idle loop (weight 0, RK_MCT_IDLE): to the end of the
 * period, which the boundary drops if it takes a request first. */
static int idling;
static void idle_tick(void){wall=step_at(consumed+2ul-consumed%3ul);idling=1;}
/* The night watchman (platform/x86/night_watchman.h) under scaled time: wm
 * by the native rule, wm_ya by the NEWJOB accesses yaAGC makes over the
 * same spans (lockstep and bridge arm yaAGC's own flag and read it back).
 * wm_fire: the alarm sounded at the last period's end. wm_windows counts
 * the alarm's checks, wm_disagree those where the two rules differ. */
static night_watchman wm,wm_ya;static int wm_fire;static unsigned long wm_alarms,wm_windows,wm_disagree,wm_stuck,wm_ya_tripped;
/* yaAGC's own alarm inside a span (its scaler is frozen there, so never
 * expected): counted, and cleared so the span's GOJAM bit does not stay. */
static void ya_tripped(void){if(m.NightWatchmanTripped){++wm_ya_tripped;m.NightWatchmanTripped=0;}}
static void period_end(void){++taken;
 if(!scaled)return;
 if(taken%NW_PERIODS==0u&&(wm.armed||wm_ya.armed)){++wm_windows;
  if(wm.armed!=wm_ya.armed&&++wm_disagree<=8ul)printf("night watchman: at period %lu the native rule has it %s, yaAGC's NEWJOB accesses %s\n",
   taken,wm.armed?"armed":"disarmed",wm_ya.armed?"armed":"disarmed");}
 (void)nw_period(&wm_ya,taken);
 if(nw_period(&wm,taken))wm_fire=1;}
/* rk_rupts: the CIA1 interrupts since the last boundary, at most lag of
 * them as clock periods, the rest dropped. */
static unsigned due(void){unsigned long now=(unsigned long)(wall/(85333.0/100.0*divisor));unsigned n=0;
 for(;consumed<now;++consumed){if(n<lag){++n;period_end();}else ++dropped;}
 return n;}
/* The work clock's steps due, for bridged code (yaAGC takes their
 * requests between its instructions). */
static unsigned due_steps(void){unsigned n=0;
 for(;step_at(consumed)<=wall;++consumed,++n)if(consumed%3ul==2ul)period_end();
 return n;}
/* The work clock's interpreter module (platform/c64/interp_module.c): a
 * table left to the interpreter (FLIGHT_PER_WORD) runs as the C64 runs it,
 * with the compact interpreter (src/lm_interp_core.c, built with
 * INTERP_WORD_CHARGE), slot after slot until the words charged reach the
 * next timer step (the C64's rk_budget), at most CORE_SLOTS slots, the
 * first always; off the work clock, one slot a routine (the C64's
 * resident has no other clock, but the host runs to the snapshot on yaAGC
 * time, and lockstep checks a slot like any routine). A slot the core refuses after one that ran is not run:
 * the next routine starts at that point, and the core refusing it there
 * leaves it to the table, as another holder takes it on the C64. So the
 * host takes interrupts between the same two words as the C64, not after
 * a whole chain of them. core_mct: the slots' charge; core_ran: whether
 * the core ran the routine (core_misses: its charge was not the words
 * yaAGC fetched over it, word_mct). */
#if RK_LM
#define CORE lm_interp_core
#define CORE_MCT lm_interp_core_mct
#define CORE_IO FLIGHT_IO_P66
#else
#define CORE cm_interp_core
#define CORE_MCT cm_interp_core_mct
#define CORE_IO FLIGHT_IO_CM_NATIVE
#endif
#define CORE_SLOTS 64u
static unsigned long core_mct,core_slots,core_routines,core_misses;
/* The core's charge is a fitted per-order table (tools/interp_word_mct.py),
 * not yaAGC's count. Over the routines the core ran and yaAGC matched:
 * the table's sum (core_fit), yaAGC's own MCT for the same spans
 * (core_yaagc, from span_mct) and the sum of the per-routine differences
 * (core_err). A replay window counts from its start (issue #765). */
static unsigned long core_fit,core_yaagc,core_err,core_spans;
static void core_charge_report(void){
 if(core_spans)printf("interpreter charge: fitted %lu MCT, yaAGC %lu MCT (%+.1f%%) over %lu routines; mean |error| %.1f MCT per routine\n",
  core_fit,core_yaagc,core_yaagc?100.0*((double)core_fit-(double)core_yaagc)/(double)core_yaagc:0.0,core_spans,(double)core_err/(double)core_spans);}
static int core_ran;
static enum exec_status core_run(const agc_native_program *p,apollo_overlay_state *s,void *x){
 /* As platform/c64/interp_module.c: the clock starts at 0x8000 less the
  * step's budget (whole MCT, at least 1), and the core stops before the
  * next word once bit 15 sets; off the work clock, one slot. */
 static apollo_overlay_state last;double budget=step_at(consumed)-wall;unsigned n=0,b=0;enum exec_status st;
 if(work)b=budget<1?1u:budget>=0x7FFF?0x7FFFu:(unsigned)budget+(budget>(unsigned)budget);
 CORE_MCT=(uint16_t)(0x8000u-b);if(!b)CORE_MCT=0;
 for(;;){last=*s;st=CORE(p,s,x);
  /* A call refused after a slot that ran is undone in the image only.
   * The I/O record x is not: the one field the core writes there is
   * interrupt_enabled (INHINT/RELINT in lmi_switch, CHANG2's RELINT), so
   * a call that changed it and then refused would keep the change. */
  if(st!=EXEC_OK){if(n){*s=last;st=EXEC_OK;}break;}
  if(++n>=CORE_SLOTS||!work||(CORE_MCT&0x8000u))break;}
 core_mct=b?(uint16_t)((CORE_MCT^0x8000u)+b):CORE_MCT;
 core_slots+=n;
 return st;}
/* A point no table holds goes to the interpreter module too, as the C64
 * resolver sends the interpreter's own entry points (NEWOPS, DANZIG, ...;
 * tools/flight_store_build.py INTERP_POINTS) to it; the core refuses any
 * other point (EXEC_UNMAPPED), which is then bridged. */
static const agc_routine_table core_none={0,0};
static const entry core_entry={&core_none,"interp_module",1,CORE_IO,"",0,0,0,"",0,FLIGHT_PER_WORD};
/* The routine before's table for the work clock's charge (charged), 0 at a fresh step. */
static const entry *charge_last;
/* Input channels 30 and 31 at start (FLIGHT_CH30, FLIGHT_CH31 below). */
static unsigned long chan30=037777u,chan31=077777u,chan32=077777u,chan33=077777u;
/* yaAGC's GOJAM state (agc_engine_init) without its first DOWNRUPT. */
static void cold(void){unsigned i;m=loaded;memset(m.Erasable,0,sizeof m.Erasable);
 for(i=0;i<NUM_CHANNELS;++i)m.InputChannel[i]=0;
 m.InputChannel[030]=(int16_t)chan30;m.InputChannel[031]=(int16_t)chan31;m.InputChannel[032]=(int16_t)chan32;m.InputChannel[033]=(int16_t)chan33;
 m.Erasable[0][5]=04000;m.AllowInterrupt=1;}

/* FLIGHT_CORE=FILE (scaled time only): start from a yaAGC core dump
 * (agc_engine_init's format: channels, erasable, CPU state; e.g. a NASSP
 * save) in place of GOJAM. yaAGC loads it and runs to the end of any
 * EXTEND, INDEX or pending interrupt sequence the dump stopped inside; the
 * native side takes that state (store), with the FLIGHT_CH30-33 given
 * overriding the core's, and AGC time counts from the core's TIME1/TIME2
 * (t0), so keys, pokes and the centisecond limit are relative to it. */
static const char *core_path;
/* FLIGHT_DOWNRUPT under FLIGHT_CORE: waiting for the first sample point. */
static int dl_arm;
static const agc_word *view(void);
static int core_start(const char *rope){static agc_t c;int rc;unsigned i,n;
 rc=agc_engine_init(&c,rope,core_path,1);if(rc!=0&&rc!=5){fprintf(stderr,"FLIGHT_CORE: %s: agc_engine_init %d\n",core_path,rc);return 0;}
 m=loaded;memcpy(m.Erasable,c.Erasable,sizeof m.Erasable);
 for(i=0;i<NUM_CHANNELS;++i)m.InputChannel[i]=c.InputChannel[i];
 m.OutputChannel7=c.OutputChannel7;m.InputChannel[7]=c.OutputChannel7;
 m.AllowInterrupt=c.AllowInterrupt;m.InIsr=c.InIsr;memcpy(m.InterruptRequests,c.InterruptRequests,sizeof m.InterruptRequests);
 m.ExtraCode=c.ExtraCode;m.IndexValue=c.IndexValue;m.SubstituteInstruction=c.SubstituteInstruction;m.PendFlag=c.PendFlag;m.PendDelay=c.PendDelay;
 m.Trap31A=c.Trap31A;m.Trap31B=c.Trap31B;m.Trap32=c.Trap32;
 if(getenv("FLIGHT_CH30"))m.InputChannel[030]=(int16_t)chan30;
 if(getenv("FLIGHT_CH31"))m.InputChannel[031]=(int16_t)chan31;
 if(getenv("FLIGHT_CH32"))m.InputChannel[032]=(int16_t)chan32;
 if(getenv("FLIGHT_CH33"))m.InputChannel[033]=(int16_t)chan33;
 for(n=0;n<100u&&(m.ExtraCode||m.IndexValue||m.SubstituteInstruction||m.PendFlag);++n)if(agc_engine(&m))return 0;
 /* A dump taken inside a routine (an interrupt, or Q set for a return the
  * routine's entry would not see) is no state to start a native routine
  * in: yaAGC runs on to the idle loop. */
 for(n=0;n<5000000u&&!(quiet()&&!m.InIsr&&idle(view(),(agc_word)m.OutputChannel7,0));++n)if(agc_engine(&m))return 0;
 if(n==5000000u){fprintf(stderr,"FLIGHT_CORE: %s: never idle\n",core_path);return 0;}
 return 1;}

/* The word at fixed address z under FB and the superbank. */
/* Reference side only: the rope word yaAGC holds (KEYRUPT1's BRUPT). */
static agc_word rope_word(agc_word z,agc_word fb,agc_word sb){uint16_t p=agc_fixed_physical(z,fb,sb);return p==AGC_TRANSFER_NONE?0u:(agc_word)loaded.Fixed[p/1024u][p%1024u];}
/* The TC trap (docs/FLIGHT-RESTART.md, The TC trap). VERB69 (43,2037 in both
 * ropes, V69's TC VERB69; bank 043 is FB 033 with the superbank's bit 0100):
 * the test the C64's rk_resident_step makes, Z there outside an interrupt. */
static int verb69(const agc_word *w,agc_word sb){return (w[5]&07777u)==02037u&&w[4]==066000u&&(sb&0100u);}
/* Unscaled, yaAGC's own trap fires at scaler phase 000 after nothing but
 * TCs since phase 020 (agc_engine.c TCTrap; it sets channel 77 bit 3, the
 * GOJAM itself being off here with InhibitAlarms), on the reference and in
 * bridged code: yaagc_trap takes the bit, 1 at VERB69, and stops the run
 * (exit 3) anywhere else or for the no-TC alarm the bit also reports, which
 * the C64 does not model. */
static int yaagc_trap(const char *side){const agc_word *v;
 if(!(m.InputChannel[077]&CH77_TC_TRAP))return 0;
 m.InputChannel[077]&=~CH77_TC_TRAP;v=view();
 if(m.TCTrap&&!m.InIsr&&verb69(v,(agc_word)m.OutputChannel7))return 1;
 fprintf(stderr,"FLIGHT: %s: yaAGC's %s alarm at Z %05o FB %05o, not V69's VERB69 (the C64 does not model it)\n",
  side,m.TCTrap?"TC trap":"no-TC",(unsigned)v[5],(unsigned)v[4]);
 exit(3);}
/* A TC or TCF to itself, with no INDEX before it (bridged code under scaled
 * time, where yaAGC's clock is frozen and its trap cannot fire). */
static int self_loop(const agc_word *w,agc_word sb){agc_word z=(agc_word)(w[5]&07777u),k=z>=02000u?rope_word(z,w[4],sb):0u;return z>=02000u&&(k==z||k==(agc_word)(010000u|z));}
static agc_routine find(const agc_routine_table *t,uint16_t key){uint16_t lo=0,hi=t->count,mid;
 while(lo<hi){mid=(uint16_t)(lo+(hi-lo)/2u);if(t->entries[mid].physical==key)return t->entries[mid].call;
  if(t->entries[mid].physical<key)lo=(uint16_t)(mid+1u);else hi=mid;}
 return 0;}
static uint16_t basic_point(const agc_word *w,agc_word sb){agc_word z=(agc_word)(w[5]&07777u);
 return z<02000u?agc_erasable_point(z,w[3]):agc_fixed_physical(z,w[4],sb);}
static int interpreting(const agc_word *w,agc_word sb){uint16_t z=agc_fixed_physical((agc_word)(w[5]&07777u),w[4],sb);
 agc_word edop=(agc_word)(w[AGC_EDOP]&077777u);
 return z==FETCH||(z==DANZIG&&edop!=0u&&edop<040000u);}
static uint16_t point(const agc_word *w,agc_word sb,int *in){*in=interpreting(w,sb);
 return *in?agc_fixed_physical(w[0164],w[0165],sb):basic_point(w,sb);}
/* The module loaded in each slot (job, task; module_load). */
static const char *in_slot[2];
/* A table split over several modules (chunks): the module id holding
 * each key (<m>-host-chunks.inc), and the id loaded in each slot, so a
 * chunk's key runs in the loaded module only when that chunk is loaded. */
#ifdef FLIGHT_HOST_CHUNKS
#define FLIGHT_CHUNK(t,k,i) {&t,k,i},
static const struct{const agc_routine_table *t;uint16_t key,id;} chunks[]={
#include FLIGHT_HOST_CHUNKS
};
#undef FLIGHT_CHUNK
#define NCHUNKS (sizeof chunks/sizeof chunks[0])
static uint16_t chunk_of(const agc_routine_table *t,uint16_t key){unsigned lo=0,hi=NCHUNKS;
 while(lo<hi){unsigned mid=(lo+hi)/2u;if(chunks[mid].key<key)lo=mid+1u;else hi=mid;}
 for(;lo<NCHUNKS&&chunks[lo].key==key;++lo)if(chunks[lo].t==t)return chunks[lo].id;
 return 0;}
#else
static uint16_t chunk_of(const agc_routine_table *t,uint16_t key){(void)t;(void)key;return 0;}
#endif
static uint16_t in_chunk[2];
/* Whether slot s holds t's module, and for a split table the chunk with key. */
static int slot_has(unsigned s,const char *module,const agc_routine_table *t,uint16_t key){uint16_t c;
 if(!in_slot[s]||strcmp(module,in_slot[s]))return 0;
 c=chunk_of(t,key);return !c||c==in_chunk[s];}
/* A ride table (slot 5, flight_store_build.py RIDE) holds its keys only
 * while its pack is loaded in the job slot. */
/* after: the holder that refused (a pinned table: the next one may take
 * the point, as rk_direct moves on when a slot runs no step), or 0. */
static int kernel_table(const entry *e);
static const entry *holder_after(const agc_word *w,agc_word sb,agc_routine *call,const entry *after){int in;unsigned i,r;uint16_t key=point(w,sb,&in);
 /* A fresh search goes as rk_direct does: the kernel's tables, the pinned
  * sets, then the modules loaded in the job and task slots (the loaded
  * one runs a key two modules hold), then host.inc's order. */
 if(!after)for(r=0;r<4u;++r)for(i=0;i<TABLES;++i){const entry *t=&tables[i];
  if((r==0u?kernel_table(t)&&!in:r==1u?t->slot==3u||t->slot==4u
      :in_slot[r-2u]&&!kernel_table(t)&&t->slot!=3u&&t->slot!=4u&&slot_has(r-2u,t->module,t->t,key))
     &&t->interp==(uint8_t)in&&(t->slot!=5u||in_slot[0]==t->module)&&(*call=find(t->t,key)))return t;}
 for(i=after?(unsigned)(after-tables)+1u:0u;i<TABLES;++i)if(tables[i].interp==(uint8_t)in&&(tables[i].slot!=5u||in_slot[0]==tables[i].module)
                         &&(*call=find(tables[i].t,key)))return &tables[i];
 return 0;}
static const entry *holder(const agc_word *w,agc_word sb,agc_routine *call){return holder_after(w,sb,call,0);}
/* The work clock's charge (FLIGHT_REPLAY): the table the C64 would run the
 * routine from, which can be another holder of the same key than the one
 * that runs it here (a hot subset and its full table). A C64 step goes on
 * in the module that ran the last routine while that module holds the
 * point and no interrupt was taken between (rk_step_run's module
 * chain). Otherwise the step starts afresh: the kernel's tables first
 * (rk_trampoline, RK_RESIDENT_MCT), then the pinned sets (rk_direct), then
 * the resolver's order, which is host.inc's. last: the routine before's
 * table, 0 at a fresh step. */
static int kernel_name(const char *n){
 return strstr(n,"_kernel_routines")||strstr(n,"_kernel_job_routines")||strstr(n,"_kernel_task_routines")
  ||strstr(n,"_kernel_idle_routines")||(strstr(n,"_sp_routines")&&!strstr(n,"_hot_"));}
/* Asked for every table at every point (holder, charged), so each table's
 * answer is worked out once. */
static int kernel_table(const entry *e){static uint8_t known[TABLES];unsigned i=(unsigned)(e-tables);
 if(i>=TABLES)return kernel_name(e->name)!=0;
 if(!known[i])known[i]=(uint8_t)(1u+(kernel_name(e->name)!=0));
 return known[i]-1u;}
/* The resident's interrupt body tables, which the C64 runs from its wrapper
 * modules (tools/resident_flight_build.py RESIDENT_MODULES and WRAPPED,
 * 0x01E1-0x01E4): one wrapper run goes on through its table, as a module's
 * does. The other tables with no module are the kernel's (rk_trampoline,
 * one routine a step) and the DANZIG marker. */
static int wrapper_run(const entry *e){static const char *const body[]={"_lead_in_routines","_t4rupt_core_routines","_keyrupt_routines","_t6rupt_routines"};
 size_t l=strlen(e->name),k;unsigned i;
 if(*e->module)return 0;
 for(i=0;i<sizeof body/sizeof body[0];++i)if(l>=(k=strlen(body[i]))&&!strcmp(e->name+l-k,body[i]))return 1;
 return 0;}
static const entry *charged(const agc_word *w,agc_word sb,const entry *e,const entry *last){int in;unsigned i,r;uint16_t key=point(w,sb,&in);
 /* An interrupt body table (wrapper_run) runs on the C64 as one run too:
  * DOT6RUPT's TCF RESUME goes on in that run, not in the pinned RESUME set. */
 if(last&&wrapper_run(last)&&last->interp==(uint8_t)in&&find(last->t,key))return last;
 if(last&&*last->module)for(i=0;i<TABLES;++i)if(tables[i].interp==(uint8_t)in&&!strcmp(tables[i].module,last->module)
                                           &&(tables[i].slot>1u||slot_has(tables[i].slot,tables[i].module,tables[i].t,key))
                                           &&(tables[i].slot!=5u||in_slot[0]==tables[i].module)&&find(tables[i].t,key))return &tables[i];
 /* r 2 and 3: the job and task slots' loaded modules, which rk_direct
  * tries before the resolver (a pack's copy of a key runs ahead of the
  * resolver's table: T6RUPT's 05757 from the excursion pack). */
 for(r=0;r<5u;++r)for(i=0;i<TABLES;++i){const entry *t=&tables[i];
  if((r==0u?kernel_table(t)&&!in:r==1u?t->slot==3u||t->slot==4u
      :r<4u?in_slot[r-2u]&&!kernel_table(t)&&t->slot!=3u&&t->slot!=4u&&slot_has(r-2u,t->module,t->t,key)
      :!kernel_table(t)&&t->slot!=3u&&t->slot!=4u)
     &&t->interp==(uint8_t)in&&(t->slot!=5u||in_slot[0]==t->module)&&find(t->t,key))return t;}
 return e;}
/* LM: a table of weight 0 (POSTJUMP's and SELFCHK's, RK_MCT_IDLE) idles
 * only where the C64 idles: in the idle loop (FB on SELFCHK's bank 43),
 * outside an interrupt, with no job waiting (resident_flight.c, the idle
 * module's flag), and there once a pass: SELFCHK costs nothing and
 * POSTJUMP 04635 takes the tick (below). Anywhere else (POSTJUMP in a job, or at the end of a
 * RADARUPT's task) the C64 runs the routine from a module that weighs it as
 * work: the first holder with a weight. Back in the idle loop with a job
 * waiting (NEWJOB +0 or a core set, ADVAN's CCS: anything but -0), the
 * C64's pass already ended at ADVAN, so the rest of the host's pass costs
 * nothing. */
static unsigned idle_weight(const agc_word *w,agc_word sb,int isr){
#if RK_LM
 int in;unsigned i;uint16_t key;
 /* The C64 runs the idle pass (SELFCHK to POSTJUMP 04635) as one step with
  * one idle charge, and its timer steps come due at ADVAN. Here POSTJUMP is
  * a step of its own, so the pass's charge goes there: SELFCHK costs
  * nothing, and the period's steps (and an interrupt they request) also
  * come due at ADVAN, after its sample point. */
 if(!isr&&agc_fixed_physical(02000u,w[4],sb)/1024u==043u)
  return (w[NEWJOB]&077777u)==077777u&&(w[5]&07777u)==04635u?FLIGHT_IDLE_TICK:0u;
 key=point(w,sb,&in);
 for(i=0;i<TABLES;++i)if(tables[i].interp==(uint8_t)in&&tables[i].mct&&!(tables[i].mct&FLIGHT_PLUS_WORDS)&&find(tables[i].t,key))return tables[i].mct;
#else
 /* The CM's idle loop is pinned and its POSTJUMP idles as on the host
  * (resident_flight.c RK_CM_POSTJUMP). */
 (void)w;(void)sb;(void)isr;
#endif
 return FLIGHT_IDLE_TICK;}
static void where(const agc_word *w,agc_word sb,const char *what){int in;uint16_t k=point(w,sb,&in);
 printf("%s: %s Z %05o FB %05o EB %05o superbank %03o (key %u = %02o,%04o) Q %05o LOC %05o BANKSET %05o\n",what,
  in?"interpretive":"basic",(unsigned)w[5],(unsigned)w[4],(unsigned)w[3],(unsigned)sb,(unsigned)k,k/1024u,k%1024u+02000u,
  (unsigned)w[2],(unsigned)w[0164],(unsigned)w[0165]);}
/* AGC time in centiseconds since the start (FLIGHT_CORE: since the core's
 * time, t0). */
static unsigned long t0;
static unsigned long now(const agc_word *w){return (((unsigned long)(w[024]&037777u)<<14)|(w[025]&037777u))-t0;}
/* Compared: all erasable but the central registers (0-017), the clock
 * counters (024-031) and the interrupt temporaries RUPTREG1-4 (070-073),
 * which only hold what the last interrupt left. */
static int compared(unsigned a){return a>=020u&&!(a>=070u&&a<=073u)&&(getenv("FLIGHT_TIMERS")||!(a>=024u&&a<=031u));}

/* DSKY keys (argv[3], e.g. V37E12E): key k is pressed at the first sample
 * point at or after KEY_AT + k * KEY_GAP centiseconds, as channel 15 and a
 * KEYRUPT1 request, on both sides. */
#define KEY_AT 200ul
#define KEY_GAP 50ul
static const char *keys="";
/* A key may be preceded by @CS (decimal): it and the keys after it are
 * pressed from that centisecond on (e.g. V37E63E@9000R presses RSET at
 * 90 s; P is RSET too, key code 022: PRO is no key code but channel 32
 * bit 14, pressed with FLIGHT_CHAN). Each key's time is set once, by keys_parse. */
/* A key may also be preceded by ! (opt in; no scenario without one changes):
 * it waits past its time for the DSKY's verb and noun to flash (channel 11
 * bit 6, the crew's cue to answer), and the keys after it move later by as
 * long as it waited. Each side waits on its own channel 11 (struct key_side). */
enum { KEYS=1024 };
static char keyc[KEYS+1];static unsigned long keyt[KEYS];static unsigned char keyw[KEYS];
static void keys_parse(const char *t){unsigned long at=KEY_AT;unsigned n=0;unsigned char wait=0;
 while(*t&&n<KEYS){if(*t=='@'){char *e;unsigned long v=strtoul(t+1,&e,10);if(v>at)at=v;t=e;continue;}
  if(*t=='!'){wait=1;++t;continue;}
  keyw[n]=wait;wait=0;keyc[n]=*t++;keyt[n++]=at;at+=KEY_GAP;}
 keyc[n]=0;keys=keyc;}
struct key_side{unsigned pressed;unsigned long late;};
static agc_word keycode(char c){return c=='V'?021u:c=='N'?037u:c=='E'?034u:c=='0'?020u:c=='+'?032u:c=='-'?033u:
 c=='C'?036u:c=='P'?022u:c=='K'?031u:c=='R'?022u:(agc_word)(c-'0');}
/* M and J are the CM's MARK and MARK REJECT buttons (CM P52): channel 16
 * bit 6 or bit 7 and a KEYRUPT2 request, which MARKRUPT takes; x, y and
 * j are the LM's X MARK, Y MARK and MARK REJECT (channel 16 bits 3, 4
 * and 5, LM P57). key_due returns them as MARK_KEY | the channel 16 bits. */
#define MARK_KEY 0x100
static int key_due(unsigned long t,struct key_side *k,unsigned channel11){char c;unsigned i=k->pressed;
 if(!keys[i]||t<keyt[i]+k->late)return -1;
 if(keyw[i]){if(!(channel11&040u))return -1;k->late=t-keyt[i];}
 c=keys[k->pressed++];
 return c=='M'?MARK_KEY|040:c=='J'?MARK_KEY|0100:c=='x'?MARK_KEY|04:c=='y'?MARK_KEY|010:c=='j'?MARK_KEY|020:(int)keycode(c);}
/* FLIGHT_UPLINK=CS:KEYS[;CS:KEYS...]: the ground's uplink. From the first
 * sample point at or after CS, each key (the DSKY key letters, R for ERROR
 * RESET) reaches INLINK (045) as UPRUPT's triple-redundant word (the code,
 * its complement, the code: KEYRUPT_UPRUPT.agc UPRPT1) with an UPRUPT
 * request, UPLINK_GAP centiseconds apart, on both sides. #OCTAL sends that
 * raw word instead: one that fails UPTEST locks the uplink out (UPLOCKFL,
 * FLAGWRD7 bit 4) until an ERROR RESET. */
#define UPLINK_GAP 20ul
static agc_word upw[KEYS];static unsigned long upt[KEYS];static unsigned nup;
static int uplink_parse(const char *t){
 while(t&&*t){unsigned long at;char *e;
  at=strtoul(t,&e,10);if(e==t||*e!=':')return 0;t=e+1;
  while(*t&&*t!=';'){agc_word k;
   if(nup>=KEYS)return 0;
   if(*t=='#'){unsigned long v=strtoul(t+1,&e,8);if(e==t+1)return 0;upw[nup]=(agc_word)(v&077777ul);t=e;}
   else{k=keycode(*t++);upw[nup]=(agc_word)((k<<10)|((~k&037u)<<5)|k);}
   upt[nup++]=at;at+=UPLINK_GAP;}
  if(*t==';')++t;}
 return 1;}
static int uplink_due(unsigned long t,unsigned *sent){if(*sent>=nup||t<upt[*sent])return -1;return (int)upw[(*sent)++];}
/* FLIGHT_CHAN=CS:CH=VALUE,... (octal channel and value): an input channel
 * (16, 30-33) set at the first sample point at or after CS, on both sides:
 * a switch the crew moves (CM P52: the optics mode switch, channel 33 bits
 * 4 and 5, from CMC to MANUAL and back). */
enum { CHANS=64 };
static struct { unsigned long cs; unsigned ch,value; } chans[CHANS];
static unsigned nchans;
static int chans_parse(const char *t){while(t&&*t){unsigned long cs;unsigned c,v;int n=0;
  if(sscanf(t,"%lu:%o=%o%n",&cs,&c,&v,&n)<3||!n||nchans>=CHANS||(c!=016u&&(c<030u||c>033u)))return 0;
  if(nchans&&cs<chans[nchans-1].cs)return 0;
  chans[nchans].cs=cs;chans[nchans].ch=c;chans[nchans++].value=v&077777u;t+=n;if(*t==',')++t;}
 return 1;}

/* FLIGHT_CH30, FLIGHT_CH31 (octal): the input channels at start, on both
 * sides (e.g. FLIGHT_CH30=036377 FLIGHT_CH31=057777: IMU operate, PGNCS
 * control, AUTO, so DAPIDLER starts the DAP). FLIGHT_POKE=CS:ADDR=VALUE,...
 * (octal address and value): erasable words set at the first sample point
 * at or after CS centiseconds, on both sides (e.g. 5000:035=02000 moves
 * CDUY to drive the DAP's jets). FLIGHT_LOAD=CS:FILE,...: the same from a
 * file of octal "ADDR VALUE" lines ('#' starts a comment), a scenario's
 * erasable load (tools/flight_scenarios/). FLIGHT_CH32, FLIGHT_CH33 set
 * those input channels as FLIGHT_CH30 does (e.g. FLIGHT_CH33=077737: the
 * landing radar antenna in position 1, for P63). */
enum { POKES=4096 };
static struct { unsigned long cs; unsigned addr,value; } pokes[POKES];
static unsigned npokes;

/* FLIGHT_DSKY=1: print the native DSKY (DSPTAB, the relay words) at each
 * sample point where it changed: major mode, verb, noun and R1-R3. For
 * finding a scenario's displays and key times. */
static char dsky_dig(unsigned c){
 switch(c){case 0:return ' ';case 21:return '0';case 3:return '1';case 25:return '2';case 27:return '3';case 15:return '4';case 30:return '5';case 28:return '6';case 19:return '7';case 29:return '8';case 31:return '9';}return '?';}
static void dsky_trace(unsigned long t,const agc_word *w){static char last[64];char b[64],d[24];unsigned k,sg[8]={0};
 for(k=0;k<11u;++k){unsigned v=w[01023u+k];if(v&040000u)v=(~v)&077777u;d[2*k+1]=dsky_dig(v&037u);d[2*k]=dsky_dig((v>>5)&037u);if(k<8u)sg[k]=(v>>10)&1u;}
 snprintf(b,sizeof b,"P%c%c V%c%c N%c%c R1 %c%c%c%c%c%c R2 %c%c%c%c%c%c R3 %c%c%c%c%c%c",d[20],d[21],d[18],d[19],d[16],d[17],
  sg[6]?'+':sg[5]?'-':' ',d[15],d[12],d[13],d[10],d[11],
  sg[4]?'+':sg[3]?'-':' ',d[8],d[9],d[6],d[7],d[4],
  sg[1]?'+':sg[0]?'-':' ',d[5],d[2],d[3],d[0],d[1]);
 if(strcmp(b,last)){strcpy(last,b);printf("DSKY %lu cs: %s\n",t,b);}}
/* FLIGHT_SNAPSHOT (above). Fields by name, so the layouts need not match. */
static void snapshot(const char *path,const apollo_overlay_state *s,const flight_io *io,const agc_hardware *hw){
 FILE *f=fopen(path,"w");unsigned i;
 if(!f){perror(path);exit(2);}
 fprintf(f,"/* Generated by tools/flight_host_run.c FLIGHT_SNAPSHOT at %lu cs. */\n#define SNAP_CS %luu\n",now(s->words),now(s->words));
 fprintf(f,"static const uint16_t snap_words[2048] SNAP_DATA={");
 for(i=0;i<2048u;++i)fprintf(f,"%s%uu",i?",":"",(unsigned)s->words[i]);
 fprintf(f,"};\n#define SNAP_IO(io) (io).superbank=%uu;(io).interrupt_enabled=%uu;\n",(unsigned)io->superbank,(unsigned)io->interrupt_enabled);
#define F(x) fprintf(f,"(h)." #x "=%uu;",(unsigned)hw->x);
 fprintf(f,"#define SNAP_HW(h) ");
 F(channel11)F(in_interrupt)F(channel5)F(channel6)F(channel13)F(channel15)F(channel10)F(channel32)F(channel12)F(channel14)
 F(channel30)F(channel33)F(channel31)F(channel3)F(channel4)F(radar_gate)
#undef F
#define F(x) fprintf(f,"(h)." #x "=%uu;",(unsigned)io->x);
 fprintf(f,"\n#define SNAP_FLIGHT(h) ");
 F(superbank)F(interrupt_enabled)F(channel16)F(channel34)F(channel35)F(trap31a)F(trap31b)F(trap32)F(executive_alarm)
 F(waitlist_alarm)F(warning)
#undef F
 /* The output drives' state (resident_cdu.h): a window that starts while a
  * drive owes pulses (an IMU align or torque, a TVC burn) goes on with them. */
 fprintf(f,"\n#define SNAP_CDU(c) ");
 for(i=0;i<5u;++i)fprintf(f,"(c).owed[%u]=%d;",i,(int)rk_cdu.owed[i]);
 fprintf(f,"(c).elapsed=%uu;(c).active=%uu;",(unsigned)rk_cdu.elapsed,(unsigned)rk_cdu.active);
 fprintf(f,"\n");fclose(f);}
/* FLIGHT_REPLAY=CS (with FLIGHT_SNAPSHOT): the C64 window's inputs and
 * its end state (tools/flight_scenario_vice.py --replay). After the
 * snapshot the run goes on, on the work clock from its first step (the
 * C64 window's own clock: rk_work and rk_tick_phase 0), for CS
 * centiseconds. Every input the models, keys, pokes and channel changes
 * give it in that time is one 10-byte row of PATH.replay (little-endian:
 * centiseconds since the snapshot, kind, where, address, value, work):
 * where bit 0 is 0 at a sample point, 1 at any routine boundary (the
 * engine lag's PIPA counts and CDUs, the radar model's reads); bits 1-2
 * a boundary row's work-clock step in the centisecond (1 the TIME1/TIME3
 * step's, then 2 and 3; 0 on a sample-point row, which goes in at its
 * sample point as before) and work the MCT since that step, so the C64 applies a
 * boundary row at the same point of the centisecond (a DAP pass reads
 * the CDUs on the same side of the row). Kinds: 0 an erasable word set, 1 a
 * counter (PIPA) word moved by a signed count, 2 an input channel set
 * (address: the channel; 0377 the radar gate), 3 interrupt requests raised
 * (value: RK_RUPT_BIT bits). At the first sample point CS or more
 * centiseconds after the snapshot it writes the state to PATH.end (the
 * centiseconds since the snapshot, the 2048 erasable words, then the
 * output channels rp_end lists, 16-bit little-endian) and stops; with
 * EVERY (FLIGHT_REPLAY=CS,SEED,EVERY; SEED 0 for none) also at the first
 * sample point at or after each EVERY centiseconds before that, so a
 * difference can be found by time. A hardware restart in the window fails the run (exit 3).
 * SEED: each routine's work weighs 0.8 to 1.25 times its
 * weight at random instead (a different tick schedule, for the words a
 * schedule moves: tools/flight_scenario_vice.py --replay).
 * CHAIN (FLIGHT_REPLAY=CS,SEED,EVERY,CHAIN): CHAIN windows back to back
 * (tools/flight_scenario_vice.py --chain). A window ends at the first
 * sample point CS or more centiseconds in with no interrupt pending; there
 * the run writes the next window's snapshot (PATH-1, PATH-2, ...), after
 * that point's inputs, which go to its .seam rows (the end state plus the
 * seam rows is the snapshot), and starts the next window's work clock as
 * a C64 window starts its own. A key or an interrupt request at that point
 * holds the snapshot off and fails the run (exit 4). */
/* The TC trap's GOJAM on the native side. FLIGHT_TRAP_SNAPSHOT=PRE,POST:
 * the native state just before it and just after it (as FLIGHT_SNAPSHOT
 * writes one), and the run stops. */
static int spin;
static void trap_gojam(apollo_overlay_state *s,flight_io *io,agc_hardware *hw,uint16_t *pending){const char *t=getenv("FLIGHT_TRAP_SNAPSHOT");char pre[600];size_t k;
 printf("TC trap at %lu cs (Q %05o)\n",now(s->words),(unsigned)s->words[2]);
 if(t){k=strcspn(t,",");if(!t[k]||k>=sizeof pre){fprintf(stderr,"FLIGHT_TRAP_SNAPSHOT=PRE,POST\n");exit(2);}memcpy(pre,t,k);pre[k]=0;snapshot(pre,s,io,hw);}
 gojam_native(s->words,io,hw,pending);wm.hung=0;
 if(t){snapshot(t+k+1,s,io,hw);printf("trap snapshots at %lu cs\n",now(s->words));exit(0);}}
static long ones(agc_word v);
static FILE *rp_out;static unsigned long rp_seed;
static unsigned long rp_t0,rp_cs,rp_rows;static unsigned rp_chain=1u,rp_win;static FILE *rp_seam;
static char rp_base[512],rp_path[600];
static agc_word rp_w[2048];static agc_hardware rp_hw;static agc_word rp_c16;static uint16_t rp_pending;
static void rp_row(unsigned long t,unsigned kind,unsigned where,unsigned a,unsigned v){unsigned char b[10];unsigned long d=t-rp_t0;
 /* The work clock where the row comes: the step taken last (the TIME1
  * step's is 2 mod 3, consumed counting from the window's start) and the
  * MCT since it, as the C64's rk_tick_phase and rk_work. */
 double since=wall-(consumed?step_at(consumed-1ul):0.0);unsigned work=since<0.0?0u:since>65535.0?65535u:(unsigned)since;
 if(where&1u)where|=(unsigned)((consumed+1ul)%3ul+1ul)<<1;
 if(d>0xffffu){fprintf(stderr,"FLIGHT_REPLAY: window longer than 65535 cs\n");exit(2);}
 b[0]=(unsigned char)d;b[1]=(unsigned char)(d>>8);b[2]=(unsigned char)kind;b[3]=(unsigned char)where;
 b[4]=(unsigned char)a;b[5]=(unsigned char)(a>>8);b[6]=(unsigned char)v;b[7]=(unsigned char)(v>>8);
 b[8]=(unsigned char)work;b[9]=(unsigned char)(work>>8);
 fwrite(b,1,10,rp_out);++rp_rows;}
static void rp_mark(const agc_word *w,const agc_hardware *hw,agc_word c16,uint16_t pending){memcpy(rp_w,w,sizeof rp_w);rp_hw=*hw;rp_c16=c16;rp_pending=pending;}
/* The rows for what changed since rp_mark. */
static void rp_diff(unsigned long t,unsigned where,const agc_word *w,const agc_hardware *hw,agc_word c16,uint16_t pending){unsigned i;
 if(!rp_out)return;
 for(i=0;i<2048u;++i)if(w[i]!=rp_w[i]){
  long d=ones(w[i])-ones(rp_w[i]);
  /* A PIPA is a count, not a set: setting it would wipe the C64's counts
   * READACCS has not read yet. A PIPA turning -0 into +0 (the models store
   * 0 as +0) is a count of 0, which turns the C64's -0 into +0 the same
   * way and leaves any other value as it is; without it READACCS reads
   * +0 on the host and -0 on the C64 (TEMY -0 against +0). */
  if(i>=037u&&i<=041u)rp_row(t,1u,where,i,(unsigned)(d&0xffff));
  else rp_row(t,0u,where,i,w[i]);}
#define C(x,n) if(hw->x!=rp_hw.x)rp_row(t,2u,where,n,hw->x);
 C(channel30,030u)C(channel31,031u)C(channel32,032u)C(channel33,033u)C(channel15,015u)C(channel12,012u)C(channel13,013u)C(channel14,014u)
 C(channel3,03u)C(channel4,04u)C(channel11,011u)C(channel5,05u)C(channel6,06u)C(channel10,010u)C(radar_gate,0377u)
#undef C
 if(c16!=rp_c16)rp_row(t,2u,where,016u,c16);
 if(pending&~rp_pending)rp_row(t,3u,where,0u,(unsigned)(pending&~rp_pending));}
/* The output channels PATH.end carries after the erasable words (and
 * tools/flight_scenario_vice.py compares, dap_pass_check.c DP_REPLAY). */
static FILE *rp_ends;static unsigned long rp_every,rp_next;
/* After the last window's end the run goes on RP_AFTER_CS centiseconds
 * more and writes the state at each sample point to PATH.after (the .end
 * format) and the inputs it took there to PATH.after-replay (the .replay
 * format): a C64 that reaches its first sample point a centisecond or
 * more after the host's end is compared with the host's state at that
 * same centisecond (tools/flight_scenario_vice.py pair_states), when no
 * input came in between. */
#define RP_AFTER_CS 8ul
static unsigned long rp_after=~0ul,rp_after_last;
static void rp_end(unsigned long d,const agc_word *w,const flight_io *io,const agc_hardware *hw){FILE *f=rp_ends;unsigned i;
 uint16_t c[]={hw->channel5,hw->channel6,hw->channel11,hw->channel12,hw->channel13,hw->channel14,io->channel34,io->channel35,io->superbank,
  hw->channel10,(uint16_t)(io->interrupt_enabled!=0u),hw->radar_gate};
 {unsigned char b[2]={(unsigned char)d,(unsigned char)(d>>8)};fwrite(b,1,2,f);}
 for(i=0;i<2048u;++i){unsigned char b[2]={(unsigned char)w[i],(unsigned char)(w[i]>>8)};fwrite(b,1,2,f);}
 for(i=0;i<sizeof c/sizeof c[0];++i){unsigned char b[2]={(unsigned char)c[i],(unsigned char)(c[i]>>8)};fwrite(b,1,2,f);}}
/* PATH.outputs: each change of output channels 5, 6, 11, 12, 13 and 14 in
 * the window, seen at the routine boundary where it shows, as a 6-byte row
 * (little-endian: centiseconds since the snapshot, channel, value), the
 * list dap_pass_check.c DP_OUTPUTS keeps on the C64
 * (tools/flight_scenario_vice.py --replay-outputs). log 0 takes the values
 * the window starts from. */
static FILE *rp_outs;static agc_word rp_ov[6];
static void rp_outputs(unsigned long t,const agc_hardware *hw,int log){static const unsigned char ch[6]={05u,06u,011u,012u,013u,014u};
 agc_word v[6];unsigned i;
 v[0]=hw->channel5;v[1]=hw->channel6;v[2]=hw->channel11;v[3]=hw->channel12;v[4]=hw->channel13;v[5]=hw->channel14;
 for(i=0;i<6u;++i){
  if(log&&rp_outs&&v[i]!=rp_ov[i]){unsigned long d=t-rp_t0;
   unsigned char b[6]={(unsigned char)d,(unsigned char)(d>>8),ch[i],0u,(unsigned char)v[i],(unsigned char)(v[i]>>8)};fwrite(b,1,6,rp_outs);}
  rp_ov[i]=v[i];}}
static void pokes_parse(const char *t){while(t&&*t&&npokes<POKES){unsigned long cs;unsigned a,v;int n=0;
  if(sscanf(t,"%lu:%o=%o%n",&cs,&a,&v,&n)<3||!n||a>=2048u)break;
  pokes[npokes].cs=cs;pokes[npokes].addr=a;pokes[npokes++].value=v&077777u;t+=n;if(*t==',')++t;}}
static int load_parse(const char *t){while(t&&*t){unsigned long cs;int n=0;char path[512];FILE *f;char line[256];
  if(sscanf(t,"%lu:%511[^,]%n",&cs,path,&n)<2||!n)return 0;
  t+=n;if(*t==',')++t;
  if(!(f=fopen(path,"r"))){fprintf(stderr,"FLIGHT_LOAD: cannot open %s\n",path);return 0;}
  while(fgets(line,sizeof line,f)){unsigned a,v;char *c=strchr(line,'#');if(c)*c=0;
   if(sscanf(line,"%o %o",&a,&v)!=2)continue;
   if(a>=2048u||npokes>=POKES){fclose(f);fprintf(stderr,"FLIGHT_LOAD: bad address or too many words in %s\n",path);return 0;}
   pokes[npokes].cs=cs;pokes[npokes].addr=a;pokes[npokes++].value=v&077777u;}
  fclose(f);}
 return 1;}
/* FLIGHT_LR, FLIGHT_LR_POS2 (tools/flight_lr.h): the landing radar. */
#include "flight_lr.h"
/* The channel 33 bits a running model owns (FLIGHT_RR, FLIGHT_LR,
 * FLIGHT_LR_POS2): a FLIGHT_CHAN write to channel 33 leaves them to the
 * model. A new model adds its bits here. */
#if RK_LM
static agc_word ch33_model_bits(void){return (agc_word)((rr_on?010u:0u)|lr_ch33_bits());}
#else
static agc_word ch33_model_bits(void){return rr_on?02u:0u;}
#endif
/* The channel changes due at time t, once each side (done counts them):
 * into yaAGC (hw 0) or the native channels. */
static void chan_due(unsigned long t,unsigned *done,agc_hardware *hw,flight_io *io){
 while(*done<nchans&&t>=chans[*done].cs){unsigned c=chans[*done].ch;agc_word v=(agc_word)chans[*done].value;++*done;
  if(!hw)m.InputChannel[c]=(int16_t)v;
  else if(c==016u)io->channel16=v;
  else if(c==033u)hw->channel33=(agc_word)((v&~ch33_model_bits())|(hw->channel33&ch33_model_bits()));
  else *(c==030u?&hw->channel30:c==031u?&hw->channel31:&hw->channel32)=v;}}
/* poke_due takes them in time order. */
static void pokes_sort(void){unsigned i,j;for(i=1;i<npokes;++i)for(j=i;j>0&&pokes[j-1].cs>pokes[j].cs;--j){unsigned long c=pokes[j].cs;unsigned a=pokes[j].addr,v=pokes[j].value;
  pokes[j]=pokes[j-1];pokes[j-1].cs=c;pokes[j-1].addr=a;pokes[j-1].value=v;}}
/* The pokes due at time t, once each side (done counts them). */
static void poke_due(unsigned long t,unsigned *done,agc_word *words){
 while(*done<npokes&&t>=pokes[*done].cs){unsigned a=pokes[*done].addr,v=pokes[*done].value;
  ++*done;if(words)words[a]=(agc_word)v;else *cell(a)=(int16_t)v;}}

/* FLIGHT_ENGINE=KG (LM): a minimal descent engine, the same on both
 * sides, so DVMON sees thrust and the landing guidance flies. From the
 * first sample point with ENGONFLG set (FLAGWRD5 bit 7) it thrusts at 10%
 * (373 throttle bits of 12.53 N; BITPERF), moves by each new throttle
 * command (PSEUDO55, taken when TTHROT changes) within 373 and FMAXPOS
 * (3467), and burns KG kilograms down at DPSVEX (2955.9 m/s). The thrust
 * acts along body X, put into stable-member axes from CDUY (inner gimbal)
 * and CDUZ (middle): (cos IG cos MG, sin MG, -sin IG cos MG). Each sample
 * point adds the velocity since the last one to PIPAX-Z (1 cm/s a count),
 * carrying the fraction. No attitude dynamics: while the engine is on, each
 * sample point sets CDUX-Z to the DAP's desired angles CDUXD-ZD (perfect
 * attitude control). */
static double engine_kg,engine_lag;
static struct engine { int on,lifted,stopped,landed; unsigned long t; agc_word tthrot; double bits,cmd,ramp,kg,carry[3]; } eng_ref,eng_nat;
enum { FLAGWRD5=0101, ENGONBIT=0100, PSEUDO55=03614, TTHROT=03617, PIPAX=037, CDUX=032, CDUY=033, CDUZ=034, CDUXD=03235 };
static agc_word eword(const agc_word *w,unsigned a){return w?w[a]:(agc_word)(*cell(a)&077777);}
static long ones(agc_word v){return (v&040000u)?-(long)((~v)&037777u):(long)v;}
/* FLIGHT_ENGINE_LAG=S: the throttle as the hardware drives it. Each
 * command (PSEUDO55, sent to the THRUST counter) reaches the engine as
 * pulses at 3,200 a second (THROTTLE's FRATE, 32 bits a centisecond), and
 * the thrust follows them with a first-order lag of S seconds (THROTLAG
 * assumes 0.2 s); under scaled time the PIPAs then also count at every
 * native routine boundary, not only at sample points. THROTTLE's FWEIGHT
 * expects all of that: P66's 1 s rate-of-descent loop reads the PIPAs a
 * few centiseconds after SERVICER, often with no idle pass between, and
 * oscillates without it.
 * FLIGHT_TOUCHDOWN=CS: touchdown at CS. The engine gives no thrust from
 * then on; at contact the PIPAs take the velocity change that stops the LM
 * (minus its velocity over the surface), and after it the surface's
 * reaction, lunar gravity (GM/r^2) up along RN, put into stable-member
 * axes through REFSMMAT, so the navigation holds the LM still.
 * FLIGHT_ASCENT=KG: the ascent stage's mass at liftoff. The first time the
 * engine is on after touchdown and an engine-off there, the LM lifts off
 * on the ascent engine: 15,569 N (3,500 lbf) along the X axis at 3,050 m/s
 * exhaust velocity, no throttle; with the engine off after that the PIPAs
 * stay still.
 * FLIGHT_STAGE=CS: an abort stage at CS (P71: the crew's ABORT STAGE, set
 * on channel 30 bit 4 with FLIGHT_CHAN). The descent stage separates in
 * flight: from CS the engine is the ascent engine of FLIGHT_ASCENT, as at
 * liftoff, on or off as ENGONFLG says.
 * In P70 (MODREG 70) the descent engine goes to full thrust: THROTUP
 * writes BIT13 to THRUST directly, more than FMAXPOS, with no PSEUDO55 or
 * TTHROT change. */
static unsigned long touchdown_cs,stage_cs;
static double ascent_kg;
static double eng_dp(const agc_word *w,unsigned a,int scale){
 return ((double)ones(eword(w,a))*16384.0+(double)ones(eword(w,a+1u)))*ldexp(1.0,scale-28);}
/* The velocity over the surface in stable-member axes, m/s: the LLGE's V
 * plus DELVS (the surface's own velocity, negated), as VHORCOMP forms it. */
#define MOON_MU 4.9028e12 /* m^3/s^2 */
static void surface_v(const agc_word *w,double v[3]){unsigned k;
 for(k=0;k<3u;++k)v[k]=eng_dp(w,03526u+2u*k,7)*100.0+eng_dp(w,03733u+2u*k,5)*100.0;}
static void pipa_add(agc_word *w,unsigned j,long n){long v=ones(eword(w,PIPAX+j))+n;
 v=v>037777l?037777l:v<-037777l?-037777l:v;
 v=v<0?(long)((~(unsigned long)(-v))&077777ul):v;
 if(w)w[PIPAX+j]=(agc_word)v;else *cell(PIPAX+j)=(int16_t)v;}
static void surface_due(unsigned long t,struct engine *e,agc_word *w){double r[3],rm=0.0,dt;unsigned j,k;
 /* Contact: the surface stops the LM, a velocity change the PIPAs count. */
 if(!e->landed){double v[3];e->landed=1;surface_v(w,v);for(j=0;j<3u;++j)pipa_add(w,j,(long)floor(-v[j]*100.0+0.5));}
 dt=e->t&&t>e->t?(double)(t-e->t)*0.01:0.0;e->t=t;
 for(k=0;k<3u;++k){r[k]=eng_dp(w,01220u+2u*k,29);rm+=r[k]*r[k];}
 rm=sqrt(rm);if(rm<=0.0)return;
 for(j=0;j<3u;++j){long n;double u=0.0;for(k=0;k<3u;++k)u+=eng_dp(w,01733u+6u*j+2u*k,1)*r[k]/rm;
  e->carry[j]+=MOON_MU/(rm*rm)*dt*u*100.0;n=(long)e->carry[j];e->carry[j]-=(double)n;
  pipa_add(w,j,n);}}
/* FLIGHT_PILOT=TAU: in P66 (MODREG 66) before touchdown, the crew's
 * attitude: the thrust axis tilts from the local vertical against the
 * velocity over the surface (the LLGE's V plus DELVS, stable-member axes),
 * nulling it with time constant TAU seconds, at most 15 degrees; the
 * stable-member CDUs and the DAP's desired CDUs (rate-command attitude hold
 * following the hand controller) take that attitude, outer gimbal kept. */
static double pilot_tau;
static void eset(agc_word *w,unsigned a,agc_word v){if(w)w[a]=v;else *cell(a)=(int16_t)v;}
/* FLIGHT_RHC (tools/flight_rhc.h): the LM hand controller. */
#include "flight_rhc.h"
static void pilot(agc_word *w){double r[3],v[3],u[3],rm=0.0,vu=0.0,h[3],hm=0.0,t[3],tm=0.0,ig,mg,lim=tan(15.0*3.141592653589793/180.0);unsigned k;
 surface_v(w,v);for(k=0;k<3u;++k){r[k]=eng_dp(w,03520u+2u*k,24);rm+=r[k]*r[k];}
 rm=sqrt(rm);if(rm<=0.0)return;
 for(k=0;k<3u;++k){u[k]=r[k]/rm;vu+=v[k]*u[k];}
 for(k=0;k<3u;++k){h[k]=-(v[k]-vu*u[k])/(pilot_tau*1.62);hm+=h[k]*h[k];}
 hm=sqrt(hm);if(hm>lim)for(k=0;k<3u;++k)h[k]*=lim/hm;
 for(k=0;k<3u;++k){t[k]=u[k]+h[k];tm+=t[k]*t[k];}
 tm=sqrt(tm);for(k=0;k<3u;++k)t[k]/=tm;
 mg=asin(t[1]);ig=atan2(-t[2],t[0]);
 {long cy=(long)floor(ig/(2.0*3.141592653589793)*32768.0+0.5),cz=(long)floor(mg/(2.0*3.141592653589793)*32768.0+0.5);
  agc_word y=(agc_word)((unsigned long)cy&077777ul),z=(agc_word)((unsigned long)cz&077777ul);
  eset(w,CDUY,y);eset(w,CDUZ,z);eset(w,CDUXD+1u,y);eset(w,CDUXD+2u,z);}}
static void engine_due(unsigned long t,struct engine *e,agc_word *w){double f,a,dt,ig,mg,dir[3];unsigned k;
 if(!engine_kg)return;
 if(touchdown_cs&&t>=touchdown_cs&&!e->lifted){int on=(eword(w,FLAGWRD5)&ENGONBIT)!=0;if(!on)e->stopped=1;
  if(!(ascent_kg&&e->stopped&&on)){surface_due(t,e,w);return;}
  e->lifted=1;e->on=0;e->kg=ascent_kg;}
 if(stage_cs&&t>=stage_cs&&!e->lifted){e->lifted=1;e->kg=ascent_kg;}
 if(!(eword(w,FLAGWRD5)&ENGONBIT)){e->on=0;return;}
 for(k=0;k<3u;++k){if(w)w[CDUX+k]=eword(w,CDUXD+k);else *cell(CDUX+k)=(int16_t)eword(w,CDUXD+k);}
 if(pilot_tau&&!e->lifted&&eword(w,01011)==0102u&&(!touchdown_cs||t<touchdown_cs))pilot(w);
 if(!e->on){e->on=1;e->t=t;e->tthrot=eword(w,TTHROT);if(!e->kg){e->kg=engine_kg;e->bits=e->cmd=e->ramp=373.0;}return;}
 if(!e->lifted&&eword(w,01011)==0106u){e->cmd=3467.0;if(!engine_lag)e->bits=e->cmd;}
 if(!e->lifted&&eword(w,TTHROT)!=e->tthrot){e->tthrot=eword(w,TTHROT);e->cmd+=(double)ones(eword(w,PSEUDO55));
  e->cmd=e->cmd<373.0?373.0:e->cmd>3467.0?3467.0:e->cmd;if(!engine_lag)e->bits=e->cmd;}
 dt=(double)(t-e->t)*0.01;e->t=t;f=e->lifted?15569.0:e->bits*12.53;a=f/e->kg;e->kg-=f/(e->lifted?3050.0:2955.9)*dt;
 if(!e->lifted&&engine_lag){double r=32.0*100.0*dt;
  e->ramp+=e->cmd-e->ramp>r?r:e->cmd-e->ramp<-r?-r:e->cmd-e->ramp;
  e->bits+=(e->ramp-e->bits)*(1.0-exp(-dt/engine_lag));}
 ig=(double)eword(w,CDUY)*(6.283185307179586/32768.0);mg=(double)eword(w,CDUZ)*(6.283185307179586/32768.0);
 dir[0]=cos(ig)*cos(mg);dir[1]=sin(mg);dir[2]=-sin(ig)*cos(mg);
 if(w){double dv[3];for(k=0;k<3u;++k)dv[k]=a*dt*dir[k];lrm_dv(dv,t,dt);}
 for(k=0;k<3u;++k){long n,v;e->carry[k]+=a*dt*dir[k]*100.0;n=(long)e->carry[k];e->carry[k]-=(double)n;
  v=ones(eword(w,PIPAX+k))+n;if(v>037777l)v=037777l;if(v<-037777l)v=-037777l;
  v=v<0?(long)((~(unsigned long)(-v))&077777ul):v;
  if(w)w[PIPAX+k]=(agc_word)v;else *cell(PIPAX+k)=(int16_t)v;}}

/* FLIGHT_RCS=KG (LM): the RCS jets' translation, for a vehicle of KG
 * kilograms. Each jet gives 444.8 N (100 lbf) along the axis the DAP's
 * jet tables name: channel 5's even bits +X, odd bits -X; channel 6's
 * bits 5 and 8 +Y, 6 and 7 -Y, 1 and 4 +Z, 2 and 3 -Z (rotation couples
 * cancel). Each native routine boundary adds the jets' velocity since the
 * last, to the centisecond, put into stable-member axes from CDUX-Z, to
 * PIPAX-Z (1 cm/s a count), carrying the fraction. No attitude dynamics
 * and no propellant use. The crew's translation hand controller is
 * channel 31 bits 7-12 (+X, -X, +Y, -Y, +Z, -Z, active 0) with
 * FLIGHT_CHAN. */
#if RK_LM
static double rcs_kg;
static struct { unsigned long t; agc_word c5,c6; double carry[3]; } rcs;
static int rcs_pop(unsigned v){int n=0;for(;v;v&=v-1u)++n;return n;}
static void rcs_due(unsigned long t,agc_word *w,const agc_hardware *hw){double b[3],d[3],dt;unsigned k;
 if(!rcs_kg)return;
 if(rcs.t&&t>rcs.t){dt=(double)(t-rcs.t)*0.01;
  b[0]=(double)(rcs_pop(rcs.c5&0252u)-rcs_pop(rcs.c5&0125u));
  b[1]=(double)(rcs_pop(rcs.c6&0220u)-rcs_pop(rcs.c6&0140u));
  b[2]=(double)(rcs_pop(rcs.c6&0011u)-rcs_pop(rcs.c6&0006u));
  if(b[0]!=0.0||b[1]!=0.0||b[2]!=0.0){
   for(k=0;k<3u;++k)b[k]*=444.8/rcs_kg*dt;
   lr_nbsm(lr_angle(w[CDUY]),lr_angle(w[CDUZ]),lr_angle(w[CDUX]),b,d);
   lrm_dv(d,t,dt);
   for(k=0;k<3u;++k){long n;rcs.carry[k]+=d[k]*100.0;n=(long)rcs.carry[k];rcs.carry[k]-=(double)n;pipa_add(w,k,n);}}}
 if(t>rcs.t)rcs.t=t;
 rcs.c5=hw->channel5;rcs.c6=hw->channel6;}
#else
static void rcs_due(unsigned long t,agc_word *w,const agc_hardware *hw){(void)t;(void)w;(void)hw;}
#endif

/* FLIGHT_SPS=KG (CM, scaled time only): the service propulsion engine,
 * so P40 sees its burn and the TVC DAP its own steering. From the first
 * sample point with ENGONFLG set (FLAGWRD5 bit 7) it thrusts 91,188 N,
 * burning KG kilograms down at 3,080 m/s, and each sample point adds the
 * velocity since the last one to PIPAX-Z (5.85 cm/s a count, carrying the
 * fraction).
 *
 * The gimbal: the optics error counters (drives(), each 10 ms of the
 * clock: rk_cdu_drive's 053/054 pulses) hold the TVC DAP's pitch and yaw
 * commands, 85.41 arcseconds a pulse. While channel 12 bit 8 (TVC enable) is set the
 * bell follows them with a first-order lag of SPS_LAG seconds, else it
 * goes back to the actuator zero. The thrust leaves the bell turned by the
 * mechanical biases P40 assumes (PBIAS -2.15, YBIAS +0.95 degrees) plus
 * the gimbal angles, pitch about body Y and yaw about body Z.
 *
 * The attitude: before the engine first lights, each sample point sets
 * CDUX-Z to the RCS DAP's desired angles CDUXD-ZD (perfect attitude
 * control, so P40's R60 maneuver ends). While it burns the thrust line
 * misses the centre of mass by the gimbal angle less the trim the DAP was
 * loaded with (PACTOFF, YACTOFF at ignition: a perfect trim), and the
 * vehicle turns in pitch and yaw with angular acceleration (that angle) /
 * (IAVG/TLX), the AGC's own inertia over thrust times moment arm
 * (TVCMASSPROP.agc, scaled at B+2 s^2), in 10 ms steps; CDUX-Z follow the
 * body (CDUX outer, CDUZ middle, CDUY inner gimbal). Roll does not move:
 * no RCS jet model. When the engine stops, the rates go to zero (the RCS
 * DAP holds attitude) and the attitude holds. */
/* The CM optics error counters (resident_cdu.h): [0] yaw (053, TVCYAW),
 * [1] pitch (054, TVCPITCH), in pulses. */
static long optics_counter[2];
static double sps_kg,sps_cg[2];
enum { CM_CDUXD=03246, PACTOFF=03023, IAVGTLX=03072 };
#define SPS_LAG 0.05
#define SPS_PULSE (85.41/206264.806)
static struct sps { int on,lit; unsigned long t; double kg,carry[3],del[2],cg[2],rate[2],m[3][3]; } sps_ref,sps_nat;
/* m: stable member to body, from CDUX-Z, and back. */
static void sps_matrix(const agc_word *w,double m[3][3]){double o,g,i,co,so,cg,sg,ci,si;
 o=(double)eword(w,CDUX)*(6.283185307179586/32768.0);g=(double)eword(w,CDUZ)*(6.283185307179586/32768.0);i=(double)eword(w,CDUY)*(6.283185307179586/32768.0);
 co=cos(o);so=sin(o);cg=cos(g);sg=sin(g);ci=cos(i);si=sin(i);
 /* Rx(outer) Rz(middle) Ry(inner): row 0 is body X in stable member axes. */
 m[0][0]=cg*ci;m[0][1]=sg;m[0][2]=-cg*si;
 m[1][0]=-co*sg*ci+so*si;m[1][1]=co*cg;m[1][2]=co*sg*si+so*ci;
 m[2][0]=so*sg*ci+co*si;m[2][1]=-so*cg;m[2][2]=-so*sg*si+co*ci;}
static agc_word sps_cdu(double a){long n=lround(a*(32768.0/6.283185307179586));return (agc_word)(n&077777l);}
static void sps_turn(struct sps *e,double h){double p=e->rate[0]*h,y=e->rate[1]*h,r[3][3],n[3][3],l;unsigned i,j,k;
 /* The body turns by (0, p, y): the stable member, seen from it, by minus that. */
 r[0][0]=1.0;r[0][1]=y;r[0][2]=-p;r[1][0]=-y;r[1][1]=1.0;r[1][2]=0.0;r[2][0]=p;r[2][1]=0.0;r[2][2]=1.0;
 for(i=0;i<3u;++i)for(j=0;j<3u;++j){n[i][j]=0.0;for(k=0;k<3u;++k)n[i][j]+=r[i][k]*e->m[k][j];}
 /* Keep the rows orthonormal. */
 for(i=0;i<3u;++i){for(j=0;j<i;++j){double d=0.0;for(k=0;k<3u;++k)d+=n[i][k]*n[j][k];for(k=0;k<3u;++k)n[i][k]-=d*n[j][k];}
  l=sqrt(n[i][0]*n[i][0]+n[i][1]*n[i][1]+n[i][2]*n[i][2]);for(k=0;k<3u;++k)e->m[i][k]=n[i][k]/l;}}
/* dt seconds of gimbal (and, with acc, the attitude it turns) in 10 ms steps. */
static void sps_gimbal(struct sps *e,double dt,const agc_hardware *hw,double acc){double h;unsigned k;
 for(h=dt;h>1e-9;h-=0.01){double s=h<0.01?h:0.01;
  for(k=0;k<2u;++k){double c=hw&&(hw->channel12&0200u)?(double)optics_counter[1u-k]*SPS_PULSE:0.0;
   e->del[k]+=(c-e->del[k])*(1.0-exp(-s/SPS_LAG));
   if(acc>0.0)e->rate[k]-=(e->del[k]-e->cg[k])/acc*s;}
  if(acc>0.0)sps_turn(e,s);}}
static void sps_due(unsigned long t,struct sps *e,agc_word *w,const agc_hardware *hw){double a,dt,dir[3],b[3],pb,yb,acc;unsigned k,j;
 if(!sps_kg)return;
 if(!e->lit)for(k=0;k<3u;++k)eset(w,CDUX+k,eword(w,CM_CDUXD+2u*k));
 dt=e->t&&t>e->t?(double)(t-e->t)*0.01:0.0;e->t=t;
 if(!(eword(w,FLAGWRD5)&ENGONBIT)){sps_gimbal(e,dt,hw,0.0);if(e->on){e->rate[0]=e->rate[1]=0.0;}e->on=0;return;}
 if(!e->on){sps_gimbal(e,dt,hw,0.0);e->on=e->lit=1;if(!e->kg)e->kg=sps_kg;sps_matrix(w,e->m);
  for(k=0;k<2u;++k){e->cg[k]=(double)ones(eword(w,PACTOFF+k))*SPS_PULSE+sps_cg[k]*(3.141592653589793/180.0);}
  return;}
 a=91188.0/e->kg;e->kg-=91188.0/3080.0*dt;
 acc=(double)ones(eword(w,IAVGTLX))*(4.0/16384.0);
 sps_gimbal(e,dt,hw,acc);
 eset(w,CDUY,sps_cdu(atan2(-e->m[0][2],e->m[0][0])));eset(w,CDUZ,sps_cdu(asin(e->m[0][1])));eset(w,CDUX,sps_cdu(atan2(-e->m[2][1],e->m[1][1])));
 /* The thrust in body axes, then in stable member axes (the transpose). */
 pb=-0.0375245789+e->del[0];yb=0.0165806279+e->del[1];
 b[0]=cos(pb)*cos(yb);b[1]=sin(yb);b[2]=-sin(pb)*cos(yb);
 for(j=0;j<3u;++j){dir[j]=0.0;for(k=0;k<3u;++k)dir[j]+=e->m[k][j]*b[k];}
 for(k=0;k<3u;++k){long n,v;e->carry[k]+=a*dt*dir[k]*100.0/5.85;n=(long)e->carry[k];e->carry[k]-=(double)n;
  v=ones(eword(w,PIPAX+k))+n;if(v>037777l)v=037777l;if(v<-037777l)v=-037777l;
  v=v<0?(long)((~(unsigned long)(-v))&077777ul):v;
  eset(w,PIPAX+k,(agc_word)v);}}

/* FLIGHT_AERO=B (CM): drag and lift on a capsule of ballistic coefficient
 * B kg/m^2 and L/D 0.3, the same on both sides, so the entry programs see
 * the atmosphere. While SERVICER runs (AVEGFLAG) and the CM is below
 * 140 km, each sample point takes position and velocity from the AGC's own
 * RN and VN (extrapolated from PIPTIME to TIME1/TIME2: with exact PIPAs the
 * AGC's navigation is the trajectory), drag D = rho v^2 / 2B against the
 * velocity relative to the turning earth (rho = 1.225 exp(-h / 6.7 km),
 * h above ERAD), lift 0.3 D perpendicular to it, turned from straight up
 * by the commanded roll ROLLC (perfect roll control), and adds the
 * velocity since the last point to PIPAX-Z (5.85 cm/s a count, stable
 * member axes through REFSMMAT), carrying the fraction. While P62 waits
 * for the maneuver to entry attitude (P63FLAG +1) each sample point sets
 * CDUX-Z to the desired angles THETAD (perfect attitude control), so the
 * entry DAP sees ALFA within 45 degrees and wakes P62. */
static double aero_b;
static struct aero { unsigned long t; double carry[3]; } aero_ref,aero_nat;
enum { AVEGWORD=075, AVEGBIT=1, RN=01170, VN=01176, PIPTIME=01204, REFSMMAT=01735, ROLLC=03315, P63FLAG=03327, THETAD=01155 };
static double edp(const agc_word *w,unsigned a,int scale){
 return ((double)ones(eword(w,a))*16384.0+(double)ones(eword(w,a+1u)))*ldexp(1.0,scale-28);}
static void aero_due(unsigned long t,struct aero *e,agc_word *w){double r[3],v[3],vr[3],lt[3],sd[3],acc[3],rt,h,vm,d,ph,un,dt,tn;unsigned k,j;
 if(!aero_b)return;
 if(eword(w,P63FLAG)==1u)for(k=0;k<3u;++k){if(w)w[CDUX+k]=eword(w,THETAD+k);else *cell(CDUX+k)=(int16_t)eword(w,THETAD+k);}
 dt=(double)(t-e->t)*0.01;e->t=t;
 if(!(eword(w,AVEGWORD)&AVEGBIT)||dt<=0.0)return;
 tn=(double)(eword(w,024)&037777u)*16384.0+(double)(eword(w,025)&037777u)-edp(w,PIPTIME,28);
 for(k=0;k<3u;++k){v[k]=edp(w,VN+2u*k,7)*100.0;r[k]=edp(w,RN+2u*k,29)+v[k]*tn*0.01;}
 rt=sqrt(r[0]*r[0]+r[1]*r[1]+r[2]*r[2]);h=rt-6373338.0;
 if(rt<6.0e6||h>140000.0)return;
 vr[0]=v[0]+7.29211494e-5*r[1];vr[1]=v[1]-7.29211494e-5*r[0];vr[2]=v[2];
 vm=sqrt(vr[0]*vr[0]+vr[1]*vr[1]+vr[2]*vr[2]);
 d=1.225*exp(-h/6700.0)*vm*vm/(2.0*aero_b);
 for(k=0;k<3u;++k)vr[k]/=vm;
 un=(r[0]*vr[0]+r[1]*vr[1]+r[2]*vr[2])/rt;
 for(k=0;k<3u;++k)lt[k]=r[k]/rt-un*vr[k];
 un=sqrt(lt[0]*lt[0]+lt[1]*lt[1]+lt[2]*lt[2]);for(k=0;k<3u;++k)lt[k]/=un;
 sd[0]=lt[1]*vr[2]-lt[2]*vr[1];sd[1]=lt[2]*vr[0]-lt[0]*vr[2];sd[2]=lt[0]*vr[1]-lt[1]*vr[0];
 ph=edp(w,ROLLC,0)*6.283185307179586;
 for(k=0;k<3u;++k)acc[k]=d*(-vr[k]+0.3*(cos(ph)*lt[k]+sin(ph)*sd[k]));
 for(j=0;j<3u;++j){long n,c;double a=0.0;for(k=0;k<3u;++k)a+=edp(w,REFSMMAT+6u*j+2u*k,1)*acc[k];
  e->carry[j]+=a*dt*100.0/5.85;n=(long)e->carry[j];e->carry[j]-=(double)n;
  c=ones(eword(w,PIPAX+j))+n;if(c>037777l)c=037777l;if(c<-037777l)c=-037777l;
  c=c<0?(long)((~(unsigned long)(-c))&077777ul):c;
  if(w)w[PIPAX+j]=(agc_word)c;else *cell(PIPAX+j)=(int16_t)c;}}

/* FLIGHT_IMU=1 (CM): the IMU gimbals, for the prelaunch alignment (P01):
 * while channel 12 bit 5 (zero the ICDUs) is set each sample point holds
 * CDUX-Z at 0; while bits 4 and 6 (coarse align, error counters enabled)
 * are set it sets them to the desired angles THETAD (a perfect coarse
 * align, so COARS's 2 degree check passes). Fine align and gyro torquing
 * move nothing: the platform stays where coarse align put it, and the
 * gyrocompass sees level PIPAs. */
static int imu_model;
static void imu_due(agc_word *w,agc_hardware *hw){agc_word c12=hw?hw->channel12:(agc_word)m.InputChannel[012];unsigned k;
 if(!imu_model)return;
 if(c12&020u){for(k=0;k<3u;++k)eset(w,CDUX+k,0u);return;}
 if((c12&050u)==050u)for(k=0;k<3u;++k)eset(w,CDUX+k,eword(w,THETAD+k));}
/* FLIGHT_SATURN=CS (CM): liftoff at CS and the Saturn V's boost to earth
 * orbit, the same on both sides. At CS channel 30 bit 5 (liftoff,
 * active 0) goes to 0, so P02's compass loop starts P11. From then on each
 * sample point adds the boost's specific force since the last one to
 * PIPAX-Z (5.85 cm/s a count, carrying the fraction), in stable-member
 * axes as the prelaunch alignment leaves them (X downrange at the launch
 * azimuth, Z down): thrust acceleration a along the vehicle axis, pitched
 * from the launch vertical toward downrange by THETA, f = a (sin THETA, 0,
 * -cos THETA). a: S-IC 35.1 MN on 2,960 t less 13.0 t/s, centre engine out
 * at 135.2 s (80%, 10.4 t/s), cutoff 161.6 s; S-II from 164.0 s, 5.1 MN on
 * 665 t less 1.22 t/s, centre engine out at 460 s (80%, 0.98 t/s),
 * cutoff 548 s; S-IVB from 552 s, 0.9 MN on 167 t less 0.21 t/s, cutoff
 * at 823 s. THETA: 0 to 12 s, then linear to 52.2 degrees at 161.6 s,
 * 81.2 at 400 s and 93.3 at 700 s, held after. With the earth's rotation
 * and gravity that puts the vehicle in a 142 x 218 km orbit at 823 s
 * (tools/flight_scenarios/cm_p11.py prints the trajectory). No attitude
 * model: the CDUs stay at the liftoff angles. */
/* Velocity f (m/s, stable-member axes) into PIPAX-Z at 5.85 cm/s a
 * count, carrying the fraction. */
static void pipa_carry(double *carry,const double f[3],agc_word *w){unsigned k;
 for(k=0;k<3u;++k){long n;carry[k]+=f[k]*100.0/5.85;n=(long)carry[k];carry[k]-=(double)n;pipa_add(w,k,n);}}
static unsigned long saturn_lo;
static struct saturn { int lo; unsigned long t,lo_cs; double carry[3]; } sat_ref,sat_nat;
static double saturn_acc(double t){
 if(t<0.0)return 0.0;
 if(t<135.2)return 35.1e6/(2960e3-13.0e3*t);
 if(t<161.6)return 0.8*35.1e6/(2960e3-13.0e3*135.2-10.4e3*(t-135.2));
 if(t<164.0)return 0.0;
 if(t<460.0)return 5.1e6/(665e3-1.22e3*(t-164.0));
 if(t<548.0)return 4.08e6/(665e3-1.22e3*296.0-0.98e3*(t-460.0));
 if(t<552.0||t>=823.0)return 0.0;
 return 0.9e6/(167e3-0.21e3*(t-552.0));}
static double saturn_pitch(double t){double d;
 if(t<12.0)d=0.0;else if(t<161.6)d=52.2*(t-12.0)/149.6;else if(t<400.0)d=52.2+29.0*(t-161.6)/238.4;
 else if(t<700.0)d=81.2+12.1*(t-400.0)/300.0;else d=93.3;
 return d*(3.141592653589793/180.0);}
static void saturn_due(unsigned long t,struct saturn *e,agc_word *w,agc_hardware *hw){double f[3]={0.0,0.0,0.0};unsigned long c;
 if(!saturn_lo)return;
 if(!e->lo){if(t<saturn_lo)return;e->lo=1;e->t=e->lo_cs=t;
  if(hw)hw->channel30=(agc_word)(hw->channel30&~020u);else m.InputChannel[030]=(int16_t)(m.InputChannel[030]&~020);return;}
 /* P11 zeroes the clock at liftoff: from the first point after that, time
  * counts from liftoff on the AGC clock. */
 if(t<e->t){e->lo_cs=0;e->t=0;}
 /* The velocity since the last point, in 10 ms steps. */
 for(c=e->t;c<t;++c){double tl=((double)(c-e->lo_cs)+0.5)*0.01,a=saturn_acc(tl)*0.01,th=saturn_pitch(tl);
  f[0]+=a*sin(th);f[2]-=a*cos(th);}
 e->t=t;
 pipa_carry(e->carry,f,w);}
/* FLIGHT_SIVB=CS,UX,UY,UZ (CM): the S-IVB's second burn, translunar
 * injection, from CS on the AGC clock: 0.9 MN on 140 t less 215 kg/s for
 * 347 s (about 3,190 m/s), along the unit vector (UX,UY,UZ) in
 * stable-member axes (an inertially fixed attitude; the CSM rides on the
 * S-IVB, so the CDUs stay put). Specific force into PIPAX-Z as for
 * FLIGHT_SATURN. tools/flight_scenarios/cm_tli.py gives the vector. */
static unsigned long sivb_tig;
static double sivb_u[3];
static struct sivb { unsigned long t; double carry[3]; } sivb_ref,sivb_nat;
static void sivb_due(unsigned long t,struct sivb *e,agc_word *w){double f[3]={0.0,0.0,0.0};unsigned long c;unsigned k;
 if(!sivb_tig)return;
 for(c=e->t>sivb_tig?e->t:sivb_tig;c<t;++c){double tl=((double)(c-sivb_tig)+0.5)*0.01,a;
  if(tl>=347.0)break;
  a=0.9e6/(140e3-215.0*tl)*0.01;
  for(k=0;k<3u;++k)f[k]+=a*sivb_u[k];}
 e->t=t;
 pipa_carry(e->carry,f,w);}

/* FLIGHT_PATH=FROM,TO: print every registered point each side passes
 * between those centiseconds (a diagnostic for aligning the two runs). */
static unsigned long path_from=1,path_to=0;
static void path(const char *side,const agc_word *w,agc_word sb,agc_word ch4,int isr,const char *name){int in;
 if(now(w)>=path_from&&now(w)<=path_to)printf("%s %lu.%02o key %u%s %s\n",side,now(w),(unsigned)ch4&037u,point(w,sb,&in),isr?" isr":"",name);}

static unsigned differs(const agc_word *a,const agc_word *b){unsigned i,n=0;for(i=0;i<2048u;++i)n+=compared(i)&&a[i]!=b[i];return n;}

/* Unregistered points, in order of first arrival. */
enum { HOLES=512 };
static struct { uint16_t key,qat; uint8_t in; agc_word z,fb,q; unsigned long count,instructions; const char *by; } holes[HOLES];
static unsigned nholes;
static void hole(const agc_word *w,agc_word sb,unsigned long instructions,const char *by){int in;uint16_t key=point(w,sb,&in);unsigned i;
 for(i=0;i<nholes;++i)if(holes[i].key==key&&holes[i].in==(uint8_t)in&&holes[i].by==by){++holes[i].count;holes[i].instructions+=instructions;return;}
 if(nholes<HOLES){holes[nholes].key=key;holes[nholes].in=(uint8_t)in;holes[nholes].z=w[5];holes[nholes].fb=w[4];holes[nholes].q=w[2];
  holes[nholes].qat=agc_fixed_physical(w[2],w[4],sb);
  holes[nholes].count=1;holes[nholes].instructions=instructions;holes[nholes].by=by;++nholes;}}

/* Set when the last routine was a RESUME: yaAGC then runs BRUPT (017),
 * which can differ from the word at Z (an interrupted INDEX leaves the
 * indexed instruction there), in place of the next instruction. */
static int substitute;
static FILE *mct_out;
/* FLIGHT_WORD_OUT: a word starts each time yaAGC is at the interpreter's
 * fetch point (FETCH, NEWOPS) outside an interrupt, and ends at the next
 * one. Its line: the centisecond it started, LOC and BANKSET then (octal),
 * the machine cycles spent in the interpreter's banks (00 and 03) and
 * those spent elsewhere (an RTB's routine, the basic code after EXIT, the
 * Executive), both outside interrupts, and the word NEWOPS fetched (octal).
 * A word whose job changed meanwhile (FIXLOC) is not written. */
static FILE *word_out;
static struct { int open; unsigned long cs; uint16_t loc, bankset, fixloc, code; uint64_t in, out; } word;
static void word_step(unsigned long cs);
/* Native state into yaAGC and back. */
static const unsigned short ch[]={3,4,5,6,010,011,012,013,014,015,030,031,032,033};
static agc_word *hwc(agc_hardware *hw,unsigned i){agc_word *c[]={&hw->channel3,&hw->channel4,&hw->channel5,&hw->channel6,&hw->channel10,
 &hw->channel11,&hw->channel12,&hw->channel13,&hw->channel14,&hw->channel15,&hw->channel30,&hw->channel31,&hw->channel32,&hw->channel33};return c[i];}
#define CHANNELS (sizeof ch/sizeof ch[0])
static void load(const apollo_overlay_state *s,const flight_io *io,agc_hardware *hw){unsigned i;
 for(i=0;i<2048u;++i)*cell(i)=(int16_t)s->words[i];
 for(i=0;i<CHANNELS;++i)m.InputChannel[ch[i]]=(int16_t)*hwc(hw,i);
 m.InputChannel[013]&=043777;   /* bits 12-14 arm the traps (io->trap*), and yaAGC reads its channel 13 back */
 m.InputChannel[016]=(int16_t)io->channel16;m.InputChannel[034]=(int16_t)io->channel34;m.InputChannel[035]=(int16_t)io->channel35;
 m.InputChannel[7]=m.OutputChannel7=(int16_t)io->superbank;m.AllowInterrupt=io->interrupt_enabled!=0u;m.InIsr=hw->in_interrupt!=0u;
 m.Trap31A=io->trap31a;m.Trap31B=io->trap31b;m.Trap32=io->trap32;m.RadarGateCounter=hw->radar_gate;
 memset(m.InterruptRequests,0,sizeof m.InterruptRequests);m.DownruptTimeValid=0;
 m.SubstituteInstruction=(unsigned)substitute;substitute=0;m.PendFlag=0;m.PendDelay=0;m.ExtraDelay=0;m.IndexValue=0;m.ExtraCode=0;}
/* A, L and Q are 16 bits (Q keeps the overflow pair a TS Q or XCH Q gives it). */
static agc_word mword(unsigned i){return (agc_word)(*cell(i)&(i<3u?0177777u:077777u));}
static void store(apollo_overlay_state *s,flight_io *io,agc_hardware *hw){unsigned i;
 for(i=0;i<2048u;++i)s->words[i]=mword(i);
 for(i=0;i<CHANNELS;++i)*hwc(hw,i)=(agc_word)(m.InputChannel[ch[i]]&077777);
 io->channel16=(agc_word)m.InputChannel[016];io->channel34=(agc_word)m.InputChannel[034];io->channel35=(agc_word)m.InputChannel[035];
 io->superbank=(agc_word)(m.OutputChannel7&0160);io->interrupt_enabled=m.AllowInterrupt;hw->in_interrupt=m.InIsr;
 io->trap31a=m.Trap31A;io->trap31b=m.Trap31B;io->trap32=m.Trap32;hw->radar_gate=(uint8_t)m.RadarGateCounter;}
/* The words point() and idle() read, from yaAGC. */
static const agc_word *view(void){static agc_word v[2048];static const unsigned short at[]={3,4,5,AGC_EDOP,NEWJOB,0164,0165};unsigned i;
 for(i=0;i<sizeof at/sizeof at[0];++i)v[at[i]]=mword(at[i]);
 return v;}
/* FLIGHT_GYRO=1: the platform (both programs). The gyro pulses the drive
 * sends (drives(), below) turn the stable member, and CDUX-Z follow. A pulse is 2^-21 of a
 * revolution (0.618 arcseconds, 64 to a CDU count; GYROFRAC in
 * IMU_MODE_SWITCHING_ROUTINES.agc rounds commands to it). Channel 14 bits
 * 7-8 select the gyro (1 X, 2 Y, 3 Z: the stable member axis it turns
 * about), bit 9 the sign (set: minus); pulses with no gyro selected turn
 * nothing. Each sample point turns the stable member by the pulses sent
 * since the last one, the body held still, and sets CDUX-Z from the new
 * matrix (sps_matrix's gimbal order). The matrix carries the fractions of
 * a CDU count between points: it is taken again from CDUX-Z only when they
 * are no longer what the last point set (another model, or the program's
 * coarse align, moved them). */
static int gyro_model;
static long gyro_sent[4];
static struct { int have; agc_word cdu[3]; double m[3][3]; } gyro;
static void gyro_due(agc_word *w){unsigned a,i,j,l;
 if(!gyro_model){memset(gyro_sent,0,sizeof gyro_sent);return;}
 if(!gyro_sent[1]&&!gyro_sent[2]&&!gyro_sent[3])return;
 if(!gyro.have||eword(w,CDUX)!=gyro.cdu[0]||eword(w,CDUY)!=gyro.cdu[1]||eword(w,CDUZ)!=gyro.cdu[2]){sps_matrix(w,gyro.m);gyro.have=1;}
 for(a=1;a<4u;++a)if(gyro_sent[a]){double t=(double)gyro_sent[a]*(6.283185307179586/2097152.0),c=cos(t),sn=sin(t),r[3][3]={{1,0,0},{0,1,0},{0,0,1}},n[3][3];
  /* r turns by t about stable member axis a-1; a body axis b (a row of
   * m, in stable member axes) is then r^T b in the new axes. */
  i=(a%3u);j=((a+1u)%3u);r[i][i]=c;r[i][j]=-sn;r[j][i]=sn;r[j][j]=c;
  for(i=0;i<3u;++i)for(j=0;j<3u;++j){n[i][j]=0.0;for(l=0;l<3u;++l)n[i][j]+=gyro.m[i][l]*r[l][j];}
  memcpy(gyro.m,n,sizeof n);gyro_sent[a]=0;}
 eset(w,CDUY,sps_cdu(atan2(-gyro.m[0][2],gyro.m[0][0])));eset(w,CDUZ,sps_cdu(asin(gyro.m[0][1])));eset(w,CDUX,sps_cdu(atan2(-gyro.m[2][1],gyro.m[1][1])));
 for(a=0;a<3u;++a)gyro.cdu[a]=eword(w,CDUX+a);}
#ifdef RK_CM
/* FLIGHT_OPTICS=1: the CM optics (sextant). While channel 12 bit 2 (the
 * optics error counters) is set and bit 8 (TVC enable) clear, the optics follow the pulses the
 * drive sends from CDUTCMD and CDUSCMD (053, 054), and each sample point
 * adds them to CDUT and CDUS (035, 036), 4 counts a pulse. The scale is
 * OPTDRIVE's (T4RUPT_PROGRAM.agc): it commands (DESOPT - CDU) / 4 pulses
 * a pass, so 4 counts a pulse puts the optics on the desired angles when
 * the pulses are out (a perfect servo). In TVC mode the same pulses
 * steer the SPS gimbal (sps_due) and the optics stay. */
static int optics_model;
static long optics_sent[2];
static void optics_due(agc_word *w){unsigned k;
 if(!optics_model){optics_sent[0]=optics_sent[1]=0;return;}
 for(k=0;k<2u;++k)if(optics_sent[k]){eset(w,035u+k,(agc_word)((unsigned long)((long)eword(w,035u+k)+4l*optics_sent[k])&077777ul));optics_sent[k]=0;}}
#endif
/* The output drives, once each clock period, as the C64 resident runs
 * them: the gyro drive (GYROCMD), the IMU CDU drive (CDUXCMD-CDUZCMD) and,
 * in the CM, the optics error counters (the SPS gimbal commands in TVC
 * mode, else the optics). The work clock drives at the T5 step before
 * rk_timer_phase (resident_flight.c rk_rupts_due); the scaled clock after
 * rk_timer_tick (rk_rupts). */
/* FLIGHT_ISS=1: the ISS gimbals in coarse align and ICDU zero (both
 * programs; the CM's FLIGHT_IMU is a perfect coarse align instead). While
 * channel 12 bit 5 (zero the ICDUs) is set each sample point holds CDUX-Z
 * at 0. While bits 4 and 6 (coarse align, ISS error counters) are set the
 * gimbals follow the pulses the drive sends from CDUXCMD-CDUZCMD (050-052),
 * 4 CDU counts a pulse: COARS (IMU_MODE_SWITCHING_ROUTINES.agc) commands
 * (THETAD - CDU) / 4, so the gimbals reach THETAD when the pulses are out
 * (a perfect servo, no gimbal rate limit). Pulses sent at other times move
 * nothing. */
static int iss_model;
static long iss_sent[3];
static void iss_due(agc_word *w,agc_hardware *hw){unsigned k;
 if(!iss_model){memset(iss_sent,0,sizeof iss_sent);return;}
 if(hw->channel12&020u){for(k=0;k<3u;++k){eset(w,CDUX+k,0u);iss_sent[k]=0;}return;}
 for(k=0;k<3u;++k)if(iss_sent[k]){eset(w,CDUX+k,(agc_word)((unsigned long)((long)eword(w,CDUX+k)+4l*iss_sent[k])&077777ul));iss_sent[k]=0;}}
static void drives(apollo_overlay_state *s,agc_hardware *hw){
 agc_word g=s->words[047];
 unsigned i;long issb[3];
 for(i=0;i<3u;++i)issb[i]=rk_cdu.owed[i]+ones(s->words[050u+i]);
#ifdef RK_CM
 unsigned k;long before[2];
 rk_cdu_count=5u;
 for(k=0;k<2u;++k)before[k]=rk_cdu.owed[3u+k]+ones(s->words[053u+k]);
#endif
 rk_cdu_drive(s,hw);
 /* The gyro pulses sent, signed, for the platform (gyro_due). */
 if(g!=s->words[047]){long n=(long)g-(long)s->words[047];gyro_sent[(hw->channel14>>6)&3u]+=hw->channel14&0400u?-n:n;}
#ifdef RK_CM
 /* The pulses sent leave the counter and what the drive owes; the optics
  * error counters take them (zeroed while channel 12 bit 2 is clear). */
 for(k=0;k<2u;++k){long sent=before[k]-(rk_cdu.owed[3u+k]+ones(s->words[053u+k]));
  optics_counter[k]=(hw->channel12&2u)?optics_counter[k]+sent:0;
  if((hw->channel12&0202u)==2u)optics_sent[k]+=sent;}
#endif
 /* The IMU CDU pulses sent, for the gimbals in coarse align (iss_due). */
 if((hw->channel12&050u)==050u)for(i=0;i<3u;++i)iss_sent[i]+=issb[i]-(rk_cdu.owed[i]+ones(s->words[050u+i]));
}

/* One clock period (rk_timer_tick), or with step >= 0 one work-clock step
 * (rk_timer_phase), on yaAGC's state, for bridged code. */
static void ref_tick(int step){static apollo_overlay_state t;agc_hardware h;uint16_t p=0;unsigned i;
 for(i=024u;i<=031u;++i)t.words[i]=mword(i);
 memset(&h,0,sizeof h);h.channel13=(agc_word)(m.InputChannel[013]&077777);
 h.channel3=(agc_word)(m.InputChannel[3]&077777);h.channel4=(agc_word)(m.InputChannel[4]&077777);
 h.radar_gate=(uint8_t)m.RadarGateCounter;
 /* The drives (drives(), above) on yaAGC's counters, at the TIME5 step. */
 if(step<0||step==2){for(i=047u;i<=054u;++i)t.words[i]=mword(i);
  h.channel12=(agc_word)(m.InputChannel[012]&077777);h.channel14=(agc_word)(m.InputChannel[014]&077777);drives(&t,&h);
  for(i=047u;i<=054u;++i)*cell(i)=(int16_t)t.words[i];}
 if(step<0)rk_timer_tick(&t,&h,&p);else rk_timer_phase(&t,&h,&p,(uint8_t)step);
 m.RadarGateCounter=h.radar_gate;
 for(i=024u;i<=031u;++i)*cell(i)=(int16_t)t.words[i];
 m.InputChannel[013]=(int16_t)h.channel13;m.InputChannel[3]=(int16_t)h.channel3;m.InputChannel[4]=(int16_t)h.channel4;
 for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(p&RK_RUPT_BIT(i))m.InterruptRequests[i]=1;}
/* FLIGHT_REACH=BB,AAAA[,BB,AAAA...] (octal, a point as the reports print
 * it): the first centisecond yaAGC runs the instruction there, in a native
 * routine's lockstep or bridged code, printed at the end ("not reached"
 * when it never does). A row of the program map names the body its
 * scenario must run this way (tools/program_acceptance.py). */
enum { REACHES=64 };
static uint16_t reach_at[REACHES];static long reach_cs[REACHES];static unsigned nreach;
static void reach_parse(void){const char *t=getenv("FLIGHT_REACH");char *e;unsigned long b,a;
 while(t&&*t&&nreach<REACHES){b=strtoul(t,&e,8);if(e==t||*e!=',')break;t=e+1;a=strtoul(t,&e,8);if(e==t||a<02000ul||a>03777ul)break;
  reach_at[nreach]=(uint16_t)(b*1024ul+(a-02000ul));reach_cs[nreach++]=-1;t=*e==','?e+1:e;}}
static void reach(void){unsigned i;uint16_t at;
 if(!nreach)return;
 at=agc_fixed_physical((agc_word)(*cell(5)&07777),(agc_word)(*cell(4)&077777),(agc_word)m.OutputChannel7);
 for(i=0;i<nreach;++i)if(reach_cs[i]<0&&reach_at[i]==at){agc_word w[2048];w[024]=mword(024);w[025]=mword(025);reach_cs[i]=(long)now(w);}}
/* One yaAGC instruction with its clock running; the interrupts it requests
 * are left pending for the native side to take. */
static int step(uint16_t *pending){unsigned i;reach();if(agc_engine(&m))return -1;
 for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(m.InterruptRequests[i]){*pending|=RK_RUPT_BIT(i);m.InterruptRequests[i]=0;}
 return 0;}

/* Bridge: yaAGC from the native state until a point some table holds or
 * idle (after at least one instruction). Bridged code is yaAGC's own, so
 * yaAGC takes the native side's pending interrupts, and any it requests,
 * where it would (an interrupt vector is a point the lead-in tables hold);
 * the requests still pending at the end go back to the native side. */
/* Unscaled, bridged code runs through VERB69 (whatever table holds it) until
 * yaAGC's own TC trap (yaagc_trap) sets trapped; under scaled time the
 * bridge stops there for the native spin, and stops the run (-2) at any
 * other TC or TCF to itself. The trap's watch starts clear: the native code
 * before it ran other instructions. */
static int trapped;
static long bridge(apollo_overlay_state *s,flight_io *io,agc_hardware *hw,uint16_t *pending,long limit){long n;unsigned i;agc_routine call;
 load(s,io,hw);m.NightWatchman=1;
 for(i=1;i<=NUM_INTERRUPT_TYPES;++i)m.InterruptRequests[i]=(int8_t)((*pending&RK_RUPT_BIT(i))!=0u);
 m.TCTrap=0;m.NoTC=0;m.InputChannel[077]&=~CH77_TC_TRAP;trapped=0;
 for(n=0;n<limit;++n){
  if(n&&quiet()){const agc_word *v=view();agc_word sb=(agc_word)m.OutputChannel7;
   if((holder(v,sb,&call)&&(scaled||m.InIsr||!verb69(v,sb)))||idle(v,sb,m.InIsr)||(scaled&&!m.InIsr&&verb69(v,sb)))break;
   if(scaled&&!m.InIsr&&self_loop(v,sb)){fprintf(stderr,"FLIGHT: bridged code at a TC or TCF to itself, Z %05o FB %05o (not VERB69)\n",(unsigned)v[5],(unsigned)v[4]);return -2;}}
  reach();
  if(scaled){uint64_t c=m.CycleCounter;unsigned k;if(agc_engine(&m))return -1;spend((unsigned long)(m.CycleCounter-c));
   if(quiet()){if(work){unsigned long j=consumed;for(k=due_steps();k;--k,++j){ref_tick((int)(j%3ul));if(j%3ul==2ul)steal();}}else for(k=due();k;--k){ref_tick(-1);steal();}}
   continue;}
  if(agc_engine(&m))return -1;
  if(yaagc_trap("bridge")){trapped=1;++n;break;}}
 if(n==limit)return -1;
 *pending=0;for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(m.InterruptRequests[i])*pending|=RK_RUPT_BIT(i);
 store(s,io,hw);return n;}

/* Lockstep: yaAGC from the state a native routine started in, until it
 * reaches the routine's continuation with the same erasable memory (clock
 * counters aside). The routine took that long: the native side takes the
 * counters, the scaler and the interrupts requested meanwhile from yaAGC,
 * and the instructions are returned. When yaAGC never matches, -1; the
 * native side then takes the time from yaAGC's first arrival at the
 * continuation (first holds its memory; Z -0 when it never arrived). */
/* The IMU CDU drive empties CDUXCMD-CDUZCMD (050-052) on yaAGC's cycle
 * clock while channel 14 bit 15, 14 or 13 is set; the C64 resident's
 * rk_cdu_drive (src/resident_cdu.c) does it on AGC time, so here,
 * like the timers, a driven counter is yaAGC's. cdu_driven: the axes whose
 * drive was on before or after the routine. */
static agc_word cdu_driven;
static int cdu_counter(unsigned i){return !scaled&&i>=050u&&i<=052u&&(cdu_driven&(040000u>>(i-050u)));}
static void adopt(apollo_overlay_state *after,agc_hardware *hw){unsigned i;
 if(scaled)return;   /* the clock is frozen: the routine's own counters stand */
 for(i=024u;i<=031u;++i)after->words[i]=mword(i);
 for(i=050u;i<=052u;++i)if(cdu_counter(i))after->words[i]=mword(i);
 hw->channel3=(agc_word)(m.InputChannel[3]&077777);hw->channel4=(agc_word)(m.InputChannel[4]&077777);
 if(!(m.InputChannel[013]&040000))hw->channel13&=037777u;}
/* The work clock's charge of the interpretive words yaAGC fetched over
 * the routine (word_mct): at the interpreter's fetch point (FETCH, NEWOPS)
 * outside an interrupt, the word at LOC, as FLIGHT_WORD_OUT reads it. The
 * code an interpretive word calls outside the interpreter's banks (00 and
 * 03: an RTB's routine) is a routine of its own on the C64, charged its
 * table's weight; so is it here, each time yaAGC leaves those banks for a
 * point a table holds (in: whether the last instruction was in them).
 * A routine that starts outside interpretive code (a basic table's, in -2)
 * counts nothing until its first FETCH: native fixed-fixed code (KILLTASK
 * 06027 on to KILLTSK2) is the routine's own, as on the C64. */
static unsigned long word_mct;
static void word_fetch(unsigned long *words,int *in){agc_word fb=(agc_word)(*cell(4)&077777),sb=(agc_word)m.OutputChannel7;
 uint16_t at;int now;agc_routine call;const entry *t;unsigned w;
 /* At a RESUME point yaAGC runs BRUPT, the word at Z an interrupt
  * held: a fetch there is the word's own. */
 if(m.InIsr||!(quiet()||(m.SubstituteInstruction&&!m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode)))return;
 at=agc_fixed_physical((agc_word)(*cell(5)&07777),fb,sb);
 if(*in==-2&&at!=FETCH)return;
 if(at==FETCH)*words+=4ul*interp_word_units_of(rope_word((agc_word)(*cell(0164)&077777),fb,sb));
 now=at!=AGC_TRANSFER_NONE&&(at/1024u==0u||at/1024u==3u);
 if(*in==1&&!now&&at!=AGC_TRANSFER_NONE&&(t=holder(view(),sb,&call))&&(w=weight(t))!=FLIGHT_PER_WORD)
  *words+=w&~FLIGHT_PLUS_WORDS;
 *in=now;}
/* The output channels a routine must also leave as yaAGC does (channel 13
 * as the CPU can write it, 043777): a routine that drops a channel write
 * (an I/O record copied back over it, src/flight_io.c) misses even when
 * the erasable words agree. chan_diff says how, for the miss report. */
static const unsigned short out_ch[]={5,6,010,011,012,013,014};
static char chan_diff[40];
static int channels_differ(agc_hardware *hw){unsigned i,k;
 for(i=0;i<sizeof out_ch/sizeof out_ch[0];++i){agc_word nat,ref;unsigned c=out_ch[i];
  for(k=0;ch[k]!=c;++k);
  nat=*hwc(hw,k);ref=(agc_word)(m.InputChannel[c]&077777);if(c==013u){nat&=043777u;ref&=043777u;}
  if(nat!=ref){snprintf(chan_diff,sizeof chan_diff," ch%o %05o/%05o",c,(unsigned)nat,(unsigned)ref);return 1;}}
 return 0;}
/* A native continuation at GOTO (the interpreter's, fixed-fixed) with A
 * already POLISH is yaAGC's GOTO +1: GOTOERS loops back there (TCF GOTO +1)
 * after each erasable CADR, with A the POLISH it stored, and GOTO's CA
 * POLISH would set no other word. So an erasable CADR chain the native
 * code stops in at GOTO, and the night watchman's cycle (night_watchman.h),
 * match yaAGC at GOTO +1. */
#if RK_LM
#define GOTO_Z 06646u
#else
#define GOTO_Z 06615u
#endif
#define POLISH_W 0117u
static int goto_seam(uint16_t target,int tin,const apollo_overlay_state *after,uint16_t at,int in,agc_word sb){
 return !tin&&!in&&target==agc_fixed_physical(GOTO_Z,0u,sb)&&at==agc_fixed_physical(GOTO_Z+1u,0u,sb)&&after->words[0]==after->words[POLISH_W];}
static long lockstep(const apollo_overlay_state *before,const flight_io *io0,agc_hardware *hw0,apollo_overlay_state *after,
                     agc_word superbank,agc_hardware *hw,uint16_t *pending,long limit,agc_word *first,int interp){
 static apollo_overlay_state timed;static agc_hardware timed_hw;
 long n;int in,tin;uint16_t at,target=point(after->words,superbank,&tin),got=0,got_first=0;unsigned i;uint64_t c0;unsigned long first_mct=0,words=0,first_words=0;int in_interp=interp?-1:-2;
 first[5]=077777u;chan_diff[0]=0;cdu_driven=(agc_word)((hw0->channel14|hw->channel14)&070000u);load(before,io0,hw0);m.NightWatchman=1;c0=m.CycleCounter;
 for(n=0;n<limit;++n){
  /* After RESUME, yaAGC is at ZRUPT with BRUPT substituted for the next
   * instruction: the native continuation is that point. */
  if(n&&(quiet()||(m.SubstituteInstruction&&!m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode))
     &&(((at=point(view(),(agc_word)m.OutputChannel7,&in))==target&&in==tin)||goto_seam(target,tin,after,at,in,superbank))){
   /* An interpretive step (SWITCH's TS Q) may leave yaAGC's Q with bit 16,
    * a copy of bit 15, where the native frontier holds 15 bits (#409). */
   for(i=0;i<2048u;++i)if((scaled||!(i>=024u&&i<=031u))&&i!=5u&&!cdu_counter(i)
                          &&(i==2u&&interp?(mword(2)^after->words[2])&077777u:mword(i)!=after->words[i]))break;
   int words_agree=i==2048u;
   if(words_agree&&!channels_differ(hw)){adopt(after,hw);*pending|=got;substitute=m.SubstituteInstruction;span_mct=(unsigned long)(m.CycleCounter-c0);word_mct=words;return n;}
   if(first[5]==077777u){for(i=0;i<2048u;++i)first[i]=mword(i);first_mct=(unsigned long)(m.CycleCounter-c0);first_words=words;timed=*after;timed_hw=*hw;adopt(&timed,&timed_hw);got_first=got;}
   if(words_agree)break;}   /* the words agree, a channel does not: no later arrival will do better */
  if(work)word_fetch(&words,&in_interp);
  if(step(&got))return -1;}
 /* A miss: the routine took as long as yaAGC to its first arrival there
  * (none when it never arrived). */
 span_mct=first[5]!=077777u?first_mct:0ul;word_mct=first[5]!=077777u?first_words:0ul;
 if(first[5]!=077777u&&!scaled){for(i=024u;i<=031u;++i)after->words[i]=timed.words[i];
  for(i=050u;i<=052u;++i)if(cdu_counter(i))after->words[i]=timed.words[i];
  hw->channel3=timed_hw.channel3;hw->channel4=timed_hw.channel4;hw->channel13=timed_hw.channel13;*pending|=got_first;}
 return -1;}

/* A native routine reads the scalers (channels 3 and 4) latched for its
 * whole run, yaAGC at the moment of each READ: one LOSCALAR count is about
 * 27 MCTs, so FINETIME's L can be a count apart. When a routine misses,
 * step yaAGC from its start state to the native continuation; if it READs
 * channel 3 or 4 on the way, latch each as yaAGC had it at its last READ
 * (in hw) and say so, so the routine runs again with them (FINETIME reads
 * LOSCALAR again when it ticked between two READs). Routines that match,
 * or read no scaler, cost nothing more. */
static int scaler_latch(const apollo_overlay_state *before,const flight_io *io0,agc_hardware *hw,const apollo_overlay_state *after,
                        agc_word superbank,long limit){
 long n;int in,tin,read=0;uint16_t target=point(after->words,superbank,&tin),got=0;agc_word c3=hw->channel3,c4=hw->channel4;
 load(before,io0,hw);
 for(n=0;n<limit;++n){
  if(n&&quiet()&&point(view(),(agc_word)m.OutputChannel7,&in)==target&&in==tin)break;
  if(m.ExtraCode&&!m.IndexValue&&!m.PendFlag){
   agc_word w=rope_word((agc_word)(*cell(5)&07777),(agc_word)(*cell(4)&077777),(agc_word)m.OutputChannel7);
   if(w==3u){c3=(agc_word)(m.InputChannel[3]&077777);read=1;}
   if(w==4u){c4=(agc_word)(m.InputChannel[4]&077777);read=1;}}
  if(step(&got))return 0;}
 if(n==limit||!read||(c3==hw->channel3&&c4==hw->channel4))return 0;
 hw->channel3=c3;hw->channel4=c4;return 1;}

/* Routines whose result yaAGC never matched. */
enum { MISSES=256 };
static struct { const char *name; uint16_t key; uint8_t in; unsigned long count; agc_word word[2048],ref[2048]; int arrived; char chan[40]; } misses[MISSES];
static unsigned nmisses;
static void miss(const char *name,const agc_word *before,agc_word sb,const agc_word *after,const agc_word *first){int in;uint16_t key=point(before,sb,&in);unsigned i;
 if(getenv("FLIGHT_MISS_LOG"))printf("miss %s at %lu cs: Z %05o FB %05o Q %05o (before), Q %05o/%05o after (native/yaAGC)\n",name,now(before),(unsigned)before[5],(unsigned)before[4],(unsigned)before[2],(unsigned)after[2],(unsigned)first[2]);
 for(i=0;i<nmisses;++i)if(misses[i].name==name&&misses[i].key==key&&misses[i].in==(uint8_t)in){++misses[i].count;return;}
 if(nmisses<MISSES){misses[nmisses].name=name;misses[nmisses].key=key;misses[nmisses].in=(uint8_t)in;misses[nmisses].count=1;
  memcpy(misses[nmisses].word,after,sizeof misses[nmisses].word);memcpy(misses[nmisses].ref,first,sizeof misses[nmisses].ref);
  misses[nmisses].arrived=first[5]!=077777u;memcpy(misses[nmisses].chan,chan_diff,sizeof chan_diff);++nmisses;}}

/* The flight resident's fixed_data hook (platform/c64/resident_flight.c
 * rk_fixed_word): the rope word at every point where it stops between
 * routines, the keys of every table it can resolve
 * (tools/resident_boundary_words.py with the store's resolver records). */
/* Sized for every table key before duplicates go (CM has over 10,000): a
 * full array would drop the last tables and FETCH and DANZIG, and the
 * native side would hold every interrupt at an interpretive order. */
static agc_site_operand boundary[16384];
static uint16_t nboundary;
static int by_key(const void *a,const void *b){return (int)((const agc_site_operand *)a)->physical-(int)((const agc_site_operand *)b)->physical;}
/* Interrupts take the word at Z: at a basic routine Z is its key, at an
 * interpretive one Z is the interpreter fetch point or DANZIG (the key is
 * LOC's), so those two words are boundary words too. */
static void boundary_build(void){unsigned i,j;
 for(i=0;i<=TABLES;++i)for(j=0;j<(i<TABLES?tables[i].t->count:2u);++j){uint16_t k=i<TABLES?tables[i].t->entries[j].physical:j?DANZIG:FETCH;agc_word w=0;
  if(k<APOLLO_ROPE_WORDS&&apollo_rope_read(MISSION,k,&w)){
   if(nboundary==sizeof boundary/sizeof boundary[0]){fprintf(stderr,"boundary words: more than %u keys\n",(unsigned)nboundary);exit(2);}
   boundary[nboundary].physical=k;boundary[nboundary++].word=w;}}
 qsort(boundary,nboundary,sizeof boundary[0],by_key);
 for(i=j=0;i<nboundary;++i)if(!j||boundary[j-1].physical!=boundary[i].physical)boundary[j++]=boundary[i];
 nboundary=(uint16_t)j;}
static bool boundary_word(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;return agc_site_lookup(boundary,nboundary,physical,word);}
static bool rope_data(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;return apollo_rope_read(MISSION,physical,word);}

/* Module loads: the C64 keeps one module per slot (job, task); a routine of
 * another module in its slot loads that module (a swap). Counted at each
 * native call, refused or not, by module and by the point that entered it.
 * The pinned sets (slot 3 and 4, loaded at boot, run through rk_direct
 * before any module) never load and never take a slot; a ride table (slot
 * 5) runs only in its loaded pack. */
#define LOADS 512u
static unsigned long nloads;
static struct { const char *module; uint16_t key; uint8_t in, slot; unsigned long count; } loads[LOADS];
static unsigned nload_points;
/* The resident's interrupt-body wrappers (resident_rupt_module.c,
 * 0x01E1-0x01E4) are modules too: with the LM's class-4 home in the task
 * slot, they load in the job slot (flight_store_build.py task_pinned_out),
 * so a wrapper run swaps out the job slot's module. */
static int ends(const char *n,const char *x){size_t l=strlen(n),k=strlen(x);return l>=k&&!strcmp(n+l-k,x);}
static int wrapped_table(const entry *e){const char *n=e->name;
#if RK_LM
 return !*e->module&&(ends(n,"_t6rupt_routines")||ends(n,"_lead_in_routines")||ends(n,"_t4rupt_core_routines")
                      ||ends(n,"_keyrupt_routines"));
#else
 /* The CM packs no class-4 home: its wrappers keep the task slot. */
 (void)n;(void)ends;return 0;
#endif
}
static void module_load(const entry *e,const agc_word *w,agc_word sb){int in;uint16_t key;unsigned i;
 if(wrapped_table(e)){if(in_slot[0]!=e->name){in_slot[0]=e->name;in_chunk[0]=0;}return;}
 if(e->slot>1)return;
 key=point(w,sb,&in);
 if(slot_has(e->slot,e->module,e->t,key))return;
 /* A table packed into the loaded module too runs there (CO_LOCATE). */
 if(in_slot[e->slot]&&*e->also){const char *a=strstr(e->also,in_slot[e->slot]);size_t n=strlen(in_slot[e->slot]);
  if(a&&(a==e->also||a[-1]==' ')&&(a[n]==0||a[n]==' '))return;}
 in_slot[e->slot]=e->module;in_chunk[e->slot]=chunk_of(e->t,key);++nloads;
 /* Scaled time: a swap is about 20 ms of C64 time (docs/FLIGHT-STORE.md:
  * 7,459 cycles plus 1.1 a byte), 1,707 AGC machine cycles of wall. The
  * work clock counts no load time (resident_flight.h). */
 if(scaled&&!work)wall+=swap_mct*jitter();
 for(i=0;i<nload_points;++i)if(loads[i].module==e->module&&loads[i].key==key){++loads[i].count;return;}
 if(nload_points<LOADS){loads[nload_points].module=e->module;loads[nload_points].key=key;loads[nload_points].in=(uint8_t)in;
  loads[nload_points].slot=e->slot;loads[nload_points++].count=1;}}
static unsigned long snap_cs;
static int snap_any;
static const char *snap_path;
/* FLIGHT_WATCH=ADDR[,ADDR...] (octal): at each native sample point, print
 * the words that changed since the last one (a diagnostic, e.g. MODREG,
 * the verb and noun, FAILREG). */
static void watch(const agc_word *w){static agc_word last[64];static unsigned addr[64],n;static int parsed;unsigned i;
 if(!parsed){const char *t=getenv("FLIGHT_WATCH");char *e;parsed=1;
  while(t&&*t&&n<64u){addr[n]=(unsigned)strtoul(t,&e,8);if(e==t||addr[n]>=2048u)break;last[n]=(agc_word)~0u;++n;t=*e==','?e+1:e;}}
 for(i=0;i<n;++i)if(w[addr[i]]!=last[i]){printf("watch %lu cs: %04o %05o\n",now(w),addr[i],(unsigned)w[addr[i]]);last[i]=w[addr[i]];}}
/* FLIGHT_BUSY_SAMPLE=CS (scaled runs only, which compare no samples): also
 * take a sample point at a job's step boundary when none has come for CS
 * centiseconds, so keys, radar data and pokes still reach a job that busy
 * waits for the crew (LM R22's V06N49 loop, SLOAD N49FLAG; BZE -3). */
static unsigned long busy_last;
static int busy_due(int scaled,const agc_word *w,int isr){static unsigned long every;static int parsed;
 if(!parsed){parsed=1;if(getenv("FLIGHT_BUSY_SAMPLE"))every=strtoul(getenv("FLIGHT_BUSY_SAMPLE"),0,0);}
 return scaled&&every&&!isr&&now(w)-busy_last>=every;}
static void word_step(unsigned long cs){int isr=m.InIsr;uint64_t c=m.CycleCounter;
 uint16_t at=agc_fixed_physical((agc_word)(*cell(5)&07777),(agc_word)(*cell(4)&077777),(agc_word)m.OutputChannel7);
 if(!isr&&quiet()&&at==FETCH){uint16_t fixloc=(uint16_t)(*cell(0120)&077777);
  if(word.open&&fixloc==word.fixloc)fprintf(word_out,"%lu %o %o %llu %llu %o\n",word.cs,(unsigned)word.loc,(unsigned)word.bankset,
                                            (unsigned long long)word.in,(unsigned long long)word.out,(unsigned)word.code);
  word.open=1;word.cs=cs;word.loc=(uint16_t)(*cell(0164)&077777);word.bankset=(uint16_t)(*cell(0165)&077777);word.fixloc=fixloc;word.in=word.out=0;
  /* NEWOPS INDEX LOC, CA 0: the word at LOC in the bank BANKSET set. */
  word.code=(uint16_t)rope_word((agc_word)word.loc,(agc_word)(*cell(4)&077777),(agc_word)m.OutputChannel7);}
 if(agc_engine(&m)){fclose(word_out);exit(2);}
 if(!isr&&!m.InIsr){if(at!=AGC_TRANSFER_NONE&&(at/1024u==0u||at/1024u==3u))word.in+=m.CycleCounter-c;else word.out+=m.CycleCounter-c;}}
int main(int argc,char **argv){static apollo_overlay_state s,undo,before;static agc_word ref[32768][2048],first[2048];
 flight_io io;agc_hardware hw;agc_native_program p;uint16_t pending=0;
 unsigned long cs=argc>2?strtoul(argv[2],0,0):100ul,steps,next,refs,limit,bridged=0,locked=0;
 unsigned i,j,uref=0,unat=0,cref=0,cnat=0,pref=0,pnat=0,gref=0,gnat=0;struct key_side kref={0,0},knat={0,0};int latency=getenv("FLIGHT_NATIVE_LATENCY")!=0;unsigned long diffs=0,samples=0,lost_brupt=0;
 enum { TRACE=24 };uint16_t trace[TRACE][3];const char *names[TRACE];
 if(argc<2||!apollo_rope_load(MISSION,argv[1])||agc_load_binfile(&loaded,argv[1])){fprintf(stderr,"usage: flight-host-run ROPE.bin [CS [KEYS]]\n");return 2;}
 keys_parse(argc>3?argv[3]:"");
 if(!uplink_parse(getenv("FLIGHT_UPLINK"))){fprintf(stderr,"FLIGHT_UPLINK=CS:KEYS[;CS:KEYS...] (#OCTAL a raw word)\n");return 2;}
 if(!steal_parse(getenv("FLIGHT_STEAL"))){fprintf(stderr,"FLIGHT_STEAL=T0-T1:MCT[,T0-T1:MCT...]\n");return 2;}
 atexit(hold_report);
 if(getenv("FLIGHT_MCT_OUT")&&!(mct_out=fopen(getenv("FLIGHT_MCT_OUT"),"a"))){perror("FLIGHT_MCT_OUT");return 2;}
 if(getenv("FLIGHT_WORD_OUT")&&!(word_out=fopen(getenv("FLIGHT_WORD_OUT"),"w"))){perror("FLIGHT_WORD_OUT");return 2;}
 if(getenv("FLIGHT_CH30"))chan30=strtoul(getenv("FLIGHT_CH30"),0,8)&077777u;
 if(getenv("FLIGHT_CH31"))chan31=strtoul(getenv("FLIGHT_CH31"),0,8)&077777u;
 if(getenv("FLIGHT_CH32"))chan32=strtoul(getenv("FLIGHT_CH32"),0,8)&077777u;
 if(getenv("FLIGHT_CH33"))chan33=strtoul(getenv("FLIGHT_CH33"),0,8)&077777u;
 pokes_parse(getenv("FLIGHT_POKE"));
 if(!chans_parse(getenv("FLIGHT_CHAN"))){fprintf(stderr,"FLIGHT_CHAN=CS:CH=VALUE,... (channel 16 or 30-33, in time order)\n");return 2;}
 if(getenv("FLIGHT_SNAPSHOT")){static char path_buf[512];
  snap_any=getenv("FLIGHT_SNAPSHOT_PENDING")!=0;
  if(sscanf(getenv("FLIGHT_SNAPSHOT"),"%lu,%511s",&snap_cs,path_buf)!=2){fprintf(stderr,"FLIGHT_SNAPSHOT=CS,PATH\n");return 2;}
  snap_path=path_buf;
  if(getenv("FLIGHT_REPLAY")){static char rp_buf[600];char *c;rp_cs=strtoul(getenv("FLIGHT_REPLAY"),&c,10);
   if(*c==',')rp_seed=strtoul(c+1,&c,10);
   rp_every=*c==','?strtoul(c+1,&c,10):0ul;if(!rp_every||rp_every>rp_cs)rp_every=rp_cs;rp_next=rp_every;
   if(*c==','&&(rp_chain=(unsigned)strtoul(c+1,0,10))<1u)rp_chain=1u;
   snprintf(rp_base,sizeof rp_base,"%s",snap_path);
   snprintf(rp_buf,sizeof rp_buf,"%s.replay",snap_path);if(!rp_cs||!(rp_out=fopen(rp_buf,"wb"))){fprintf(stderr,"FLIGHT_REPLAY=CS[,SEED[,EVERY]] (with FLIGHT_SNAPSHOT)\n");return 2;}
   snprintf(rp_buf,sizeof rp_buf,"%s.end",snap_path);if(!(rp_ends=fopen(rp_buf,"wb"))){perror(rp_buf);return 2;}
   snprintf(rp_buf,sizeof rp_buf,"%s.outputs",snap_path);if(!(rp_outs=fopen(rp_buf,"wb"))){perror(rp_buf);return 2;}}}
 if(!load_parse(getenv("FLIGHT_LOAD")))return 2;
 if(!gojam_parse(getenv("FLIGHT_GOJAM"))){fprintf(stderr,"FLIGHT_GOJAM=CS[,CS...]\n");return 2;}
 dl_parse();
 pokes_sort();
 if(!rhc_parse(getenv("FLIGHT_RHC"))
#if !RK_LM
  ||nrhc
#endif
  ){fprintf(stderr,"FLIGHT_RHC=FROM-TO:P,Q,R;... (LM; counts at most 42)\n");return 2;}
#if RK_LM
 if(getenv("FLIGHT_ENGINE"))engine_kg=strtod(getenv("FLIGHT_ENGINE"),0);
 if(getenv("FLIGHT_ENGINE_LAG"))engine_lag=strtod(getenv("FLIGHT_ENGINE_LAG"),0);
 if(getenv("FLIGHT_TOUCHDOWN"))touchdown_cs=strtoul(getenv("FLIGHT_TOUCHDOWN"),0,10);
 if(getenv("FLIGHT_PILOT"))pilot_tau=strtod(getenv("FLIGHT_PILOT"),0);
 if(getenv("FLIGHT_ASCENT"))ascent_kg=strtod(getenv("FLIGHT_ASCENT"),0);
 if(getenv("FLIGHT_RCS"))rcs_kg=strtod(getenv("FLIGHT_RCS"),0);
 if(rcs_kg&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_RCS needs FLIGHT_SCALED\n");return 2;}
 if(getenv("FLIGHT_STAGE"))stage_cs=strtoul(getenv("FLIGHT_STAGE"),0,10);
 if(stage_cs&&!ascent_kg){fprintf(stderr,"FLIGHT_STAGE needs FLIGHT_ASCENT\n");return 2;}
 if(getenv("FLIGHT_LR_POS2"))lr_pos2=strtoul(getenv("FLIGHT_LR_POS2"),0,10);
 if(!lrm_parse(getenv("FLIGHT_LR"))){fprintf(stderr,"FLIGHT_LR=CS[:DH]\n");return 2;}
 if(!drops_parse(&lrm.drops,getenv("FLIGHT_LR_DROP"))){fprintf(stderr,"FLIGHT_LR_DROP=T0-T1[,T0-T1...]\n");return 2;}
 if(lrm.on&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_LR needs FLIGHT_SCALED\n");return 2;}
 if(!rr_parse(getenv("FLIGHT_RR"))){fprintf(stderr,"FLIGHT_RR=CS:rx,ry,rz,vx,vy,vz:rx,ry,rz,vx,vy,vz\n");return 2;}
 if(rr_on&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_RR needs FLIGHT_SCALED\n");return 2;}
 if(!drops_parse(&rr_drops,getenv("FLIGHT_RR_DROP"))){fprintf(stderr,"FLIGHT_RR_DROP=T0-T1[,T0-T1...]\n");return 2;}
#else
 if(getenv("FLIGHT_AERO"))aero_b=strtod(getenv("FLIGHT_AERO"),0);
 if(getenv("FLIGHT_SPS")&&sscanf(getenv("FLIGHT_SPS"),"%lf,%lf,%lf",&sps_kg,&sps_cg[0],&sps_cg[1])<1){fprintf(stderr,"FLIGHT_SPS=KG[,PITCH,YAW]\n");return 2;}
 if(getenv("FLIGHT_IMU"))imu_model=atoi(getenv("FLIGHT_IMU"));
 if(getenv("FLIGHT_OPTICS"))optics_model=atoi(getenv("FLIGHT_OPTICS"));
 if(getenv("FLIGHT_SATURN"))saturn_lo=strtoul(getenv("FLIGHT_SATURN"),0,10);
 if(!rr_parse(getenv("FLIGHT_RR"))){fprintf(stderr,"FLIGHT_RR=CS:rx,ry,rz,vx,vy,vz:rx,ry,rz,vx,vy,vz\n");return 2;}
 if(rr_on&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_RR needs FLIGHT_SCALED\n");return 2;}
 if(!drops_parse(&rr_drops,getenv("FLIGHT_RR_DROP"))){fprintf(stderr,"FLIGHT_RR_DROP=T0-T1[,T0-T1...]\n");return 2;}
 if(getenv("FLIGHT_SIVB")&&sscanf(getenv("FLIGHT_SIVB"),"%lu,%lf,%lf,%lf",&sivb_tig,&sivb_u[0],&sivb_u[1],&sivb_u[2])!=4){fprintf(stderr,"FLIGHT_SIVB=CS,UX,UY,UZ\n");return 2;}
#endif
 if(getenv("FLIGHT_GYRO"))gyro_model=atoi(getenv("FLIGHT_GYRO"));
 if(getenv("FLIGHT_ISS"))iss_model=atoi(getenv("FLIGHT_ISS"));
 core_path=getenv("FLIGHT_CORE");
 if(sps_kg&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_SPS needs FLIGHT_SCALED\n");return 2;}
 if(core_path&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_CORE needs FLIGHT_SCALED\n");return 2;}
 /* FLIGHT_CORE with FLIGHT_DOWNRUPT: core_start's yaAGC steps to the idle
  * loop take no DOWNRUPT and log no downlink word; DOWNRUPT starts on both
  * sides at the first native sample point after the core (dl_arm). */
 if(core_path&&dl_on){dl_on=0;dl_arm=1;}
 if(getenv("FLIGHT_PATH"))sscanf(getenv("FLIGHT_PATH"),"%lu,%lu",&path_from,&path_to);
 reach_parse();
 if(getenv("FLIGHT_SCALED")&&!strcmp(getenv("FLIGHT_SCALED"),"W")){scaled=1;AgcFrozenClock=1;work=1;lag=~0u;}
 else if(getenv("FLIGHT_SCALED")){scaled=1;AgcFrozenClock=1;
  if(sscanf(getenv("FLIGHT_SCALED"),"%lf,%u,%u,%lu",&slow,&divisor,&lag,&seed)<3||slow<=0.0||!divisor||!lag){fprintf(stderr,"FLIGHT_SCALED=K,D,LAG[,SEED]\n");return 2;}
  if(getenv("FLIGHT_SWAP_MCT"))swap_mct=strtod(getenv("FLIGHT_SWAP_MCT"),0);}
 /* The reference keeps 32,767 centiseconds; scaled time keeps none. */
 if(!scaled&&cs>=sizeof ref/sizeof ref[0])cs=sizeof ref/sizeof ref[0]-1u;
 InhibitAlarms=1;
 /* Reference (none under FLIGHT_SCALED: the lockstep spans are it). */
 cold();
 for(next=1,limit=0;!scaled&&next<=cs&&limit<2000000000ul;++limit){
  /* yaAGC counts TIME5, TIME4 and TIME1/TIME3 at scaler phases 000, 010
   * and 020 of each 10 ms. Sample in phase 020-037, when all have the
   * same count. */
  if(quiet()&&!m.InIsr&&!requests()&&(m.InputChannel[4]&020)&&(*cell(NEWJOB)&077777)==077777){agc_word w[2048];for(i=0;i<2048u;++i)w[i]=(agc_word)(*cell(i)&077777);
   if(at_advan(w,(agc_word)m.OutputChannel7)&&idle(w,(agc_word)m.OutputChannel7,0)){int k;while(next<=cs&&now(w)>=next){memcpy(ref[next],w,sizeof w);++next;}
    /* The native side takes KEYRUPT1 right here, at ADVAN; so does the
     * reference (yaAGC itself would take it an instruction or more later,
     * which can delay the job it starts by a self-check pass). The entry
     * is rk_rupt_take's: ZRUPT = Z + 1, BRUPT = the word at Z. */
    poke_due(now(w),&pref,0);
    engine_due(now(w),&eng_ref,0);
    sps_due(now(w),&sps_ref,0,0);
    aero_due(now(w),&aero_ref,0);
    imu_due(0,0);
    saturn_due(now(w),&sat_ref,0,0);
    sivb_due(now(w),&sivb_ref,0);
    lr_due(now(w),&lr_ref,0);
    chan_due(now(w),&cref,0,0);
    rhc_due(now(w),0,0);
    if(gojam_due(now(w),&gref))gojam_yaagc(&m);
    else if((k=key_due(now(w),&kref,(unsigned)m.InputChannel[011]))>=0){unsigned r=k&MARK_KEY?RK_RUPT_KEY2:RK_RUPT_KEY1;m.InputChannel[k&MARK_KEY?016:015]=(int16_t)(k&0377);
     if(m.AllowInterrupt){*cell(015)=(int16_t)(ADVAN+1u);*cell(017)=(int16_t)rope_word(ADVAN,(agc_word)(*cell(4)&077777),(agc_word)m.OutputChannel7);*cell(5)=(int16_t)(04000u+4u*r);m.InIsr=1;}
     else m.InterruptRequests[r]=1;}
    else if((k=uplink_due(now(w),&uref))>=0){*cell(045)=(int16_t)k;
     if(m.AllowInterrupt){*cell(015)=(int16_t)(ADVAN+1u);*cell(017)=(int16_t)rope_word(ADVAN,(agc_word)(*cell(4)&077777),(agc_word)m.OutputChannel7);*cell(5)=(int16_t)(04000u+4u*RK_RUPT_UP);m.InIsr=1;}
     else m.InterruptRequests[RK_RUPT_UP]=1;}}}
  /* FLIGHT_NATIVE_LATENCY: the reference takes interrupts only where some
   * table holds the point, as the native side does (a diagnostic). */
  if(latency&&requests()){agc_routine call;int8_t held[1+NUM_INTERRUPT_TYPES];
   if(!(quiet()&&holder(view(),(agc_word)m.OutputChannel7,&call))){memcpy(held,m.InterruptRequests,sizeof held);memset(m.InterruptRequests,0,sizeof held);
    if(agc_engine(&m))return 2;
    for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(held[i])m.InterruptRequests[i]=1;
    continue;}}
  if(path_to&&quiet()){agc_routine c;const agc_word *v=view();
   if(holder(v,(agc_word)m.OutputChannel7,&c)){static agc_word w[2048];memcpy(w,v,sizeof w);w[024]=mword(024);w[025]=mword(025);
    path("REF",w,(agc_word)m.OutputChannel7,(agc_word)m.InputChannel[4],m.InIsr,"");}}
  if(word_out){word_step(next);continue;}
  if(agc_engine(&m))return 2;
  if(yaagc_trap("reference"))gojam_yaagc(&m);}
 if(word_out){fclose(word_out);return 0;}
 refs=scaled?cs:next-1u;
 if(refs<cs)printf("reference: only %lu of %lu centiseconds reached an idle boundary\n",refs,cs);
 /* Native. */
 dl_native_phase();
 cold();
 memset(&s,0,sizeof s);memset(&io,0,sizeof io);memset(&hw,0,sizeof hw);memset(&p,0,sizeof p);
 io.undo=&undo;s.words[5]=04000u;io.interrupt_enabled=1;
 hw.channel30=(agc_word)chan30;hw.channel31=(agc_word)chan31;hw.channel32=(agc_word)chan32;hw.channel33=(agc_word)chan33;
 if(core_path){if(!core_start(argv[1]))return 2;
  store(&s,&io,&hw);for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(m.InterruptRequests[i])pending|=RK_RUPT_BIT(i);
  t0=now(s.words);printf("core %s at %lu cs\n",core_path,t0);}
 boundary_build();
 /* Operands and fixed_data are set per call, as the module holding the
  * routine sets them on the C64 (platform/c64/flight_table_module.c). */
 p.hardware=&hw;
 /* The program a routine sees holds every table (DANZIG's marker first among
  * the interpretive ones), for routines that resolve a continuation
  * themselves. */
 {static agc_routine_table basic[TABLES],interp[TABLES];uint8_t nb=0,ni=1;
  for(i=0;i<TABLES;++i){if(!tables[i].interp)basic[nb++]=*tables[i].t;
   else if(tables[i].t==&agc_lm_danzig_marker||tables[i].t==&agc_cm_danzig_marker)interp[0]=*tables[i].t;
   else interp[ni++]=*tables[i].t;}
  p.basic=basic;p.basic_count=nb;p.interpretive=interp;p.interpretive_count=ni;}
 for(steps=0,next=1;next<=refs&&steps<20000000ul;++steps){agc_routine call=0;const entry *e;flight_io_buffer b;void *x;enum exec_status st;int v69;
  steal_cs=now(s.words);
  if(!pending&&(scaled||(hw.channel4&020u))&&((at_advan(s.words,io.superbank)&&idle(s.words,io.superbank,hw.in_interrupt))||busy_due(scaled,s.words,hw.in_interrupt))){int k;busy_last=now(s.words);
   while(next<=refs&&now(s.words)>=next){if(scaled){++next;continue;}++samples;
    if((j=differs(s.words,ref[next]))!=0u&&++diffs<=(getenv("FLIGHT_SHOW")?strtoul(getenv("FLIGHT_SHOW"),0,0):20ul)){
     printf("%lu cs (native routine %lu):",next,steps);
     for(i=0,j=0;i<2048u;++i)if(compared(i)&&s.words[i]!=ref[next][i]&&j++<8u)printf(" %04o %05o/%05o",i,(unsigned)s.words[i],(unsigned)ref[next][i]);
     printf("%s (%u words, native/yaAGC)\n",j>8u?" ...":"",j);}
    ++next;}
   if(dl_arm){dl_arm=0;dl_core_start(s.words);printf("downlink: DOWNRUPT from %lu cs, the first sample point after the core\n",now(s.words));}
   if(rp_after!=~0ul&&now(s.words)-rp_t0>rp_after_last){unsigned long d=now(s.words)-rp_t0;
    if(d>rp_after+RP_AFTER_CS){fclose(rp_out);fclose(rp_ends);return 0;}
    rp_end(d,s.words,&io,&hw);rp_after_last=d;}
   else if(rp_out&&!snap_path&&now(s.words)-rp_t0>=rp_next&&now(s.words)-rp_t0<rp_cs){rp_end(now(s.words)-rp_t0,s.words,&io,&hw);
    while(rp_next<=now(s.words)-rp_t0)rp_next+=rp_every;}
   if(rp_after==~0ul&&rp_out&&!snap_path&&now(s.words)-rp_t0>=rp_cs&&(rp_win+1u>=rp_chain||!pending)){
    rp_end(now(s.words)-rp_t0,s.words,&io,&hw);fclose(rp_out);fclose(rp_ends);fclose(rp_outs);rp_outs=0;printf("replay end at %lu cs (%lu cs after the snapshot): %lu input rows\n",now(s.words),now(s.words)-rp_t0,rp_rows);
    if(core_routines){printf("interpreter module: %lu routines, %lu slots; %lu charged other than yaAGC's words\n",core_routines,core_slots,core_misses);core_charge_report();}
    if(++rp_win>=rp_chain){char b[640];rp_after=rp_after_last=now(s.words)-rp_t0;
     snprintf(b,sizeof b,"%s.after-replay",rp_win>1u?rp_path:rp_base);rp_out=fopen(b,"wb");
     snprintf(b,sizeof b,"%s.after",rp_win>1u?rp_path:rp_base);rp_ends=fopen(b,"wb");
     if(!rp_out||!rp_ends){perror(b);return 2;}}
    else{
    /* CHAIN: the next window starts here (FLIGHT_REPLAY above). */
    snprintf(rp_path,sizeof rp_path,"%s-%u",rp_base,rp_win);snap_path=rp_path;snap_cs=now(s.words);snap_any=0;
    {char b[640];snprintf(b,sizeof b,"%s.replay",rp_path);rp_out=fopen(b,"wb");snprintf(b,sizeof b,"%s.end",rp_path);rp_ends=fopen(b,"wb");
     snprintf(b,sizeof b,"%s.outputs",rp_path);rp_outs=fopen(b,"wb");
     snprintf(b,sizeof b,"%s.seam",rp_path);rp_seam=fopen(b,"wb");if(!rp_out||!rp_ends||!rp_outs||!rp_seam){perror(b);return 2;}}
    rp_mark(s.words,&hw,io.channel16,pending);}}
   if(rp_out&&!snap_path)rp_mark(s.words,&hw,io.channel16,pending);
   /* A key is pressed at the first sample point at or after its time, on
    * both sides. */
   poke_due(now(s.words),&pnat,s.words);
   engine_due(now(s.words),&eng_nat,s.words);
   sps_due(now(s.words),&sps_nat,s.words,&hw);
   if(getenv("FLIGHT_DSKY"))dsky_trace(now(s.words),s.words);
   {int r=0;rr_due(now(s.words),&rr_nat,s.words,&hw.channel12,&hw.channel13,&hw.channel14,&hw.channel33,&r);if(r)pending|=RK_RUPT_BIT(RK_RUPT_RADAR);}
   watch(s.words);
   aero_due(now(s.words),&aero_nat,s.words);
   imu_due(s.words,&hw);
   iss_due(s.words,&hw);
   gyro_due(s.words);
#ifdef RK_CM
   optics_due(s.words);
#endif
   saturn_due(now(s.words),&sat_nat,s.words,&hw);
   sivb_due(now(s.words),&sivb_nat,s.words);
   lr_due(now(s.words),&lr_nat,&hw);
   lrm_due(now(s.words),touchdown_cs,s.words,&hw.channel13,&hw.channel33);
   chan_due(now(s.words),&cnat,&hw,&io);
   rhc_due(now(s.words),s.words,&hw);
   if(rp_after!=~0ul&&gojam_due(now(s.words),&gnat)){fclose(rp_out);fclose(rp_ends);return 0;}
   if(rp_out&&!snap_path&&gojam_due(now(s.words),&gnat)){fprintf(stderr,"FLIGHT_REPLAY: a GOJAM at %lu cs is in the window\n",now(s.words));return 3;}
   if(gojam_due(now(s.words),&gnat)){printf("GOJAM at %lu cs (Q %05o)\n",now(s.words),(unsigned)s.words[5]);gojam_native(s.words,&io,&hw,&pending);wm.hung=0;substitute=0;}
   else if((k=key_due(now(s.words),&knat,hw.channel11))>=0){if(k&MARK_KEY){io.channel16=(agc_word)(k&0377);pending|=RK_RUPT_BIT(RK_RUPT_KEY2);}
    else{hw.channel15=(agc_word)k;pending|=RK_RUPT_BIT(RK_RUPT_KEY1);}}
   else if((k=uplink_due(now(s.words),&unat))>=0){s.words[045]=(agc_word)k;pending|=RK_RUPT_BIT(RK_RUPT_UP);}
   if(rp_out&&!snap_path)rp_diff(now(s.words),0u,s.words,&hw,io.channel16,pending);
   if(rp_seam&&snap_path&&pending){fprintf(stderr,"FLIGHT_REPLAY: a key or interrupt request at the chain point %lu cs\n",now(s.words));return 4;}
   if(rp_seam&&snap_path){FILE *o=rp_out;rp_out=rp_seam;rp_t0=now(s.words);rp_diff(now(s.words),0u,s.words,&hw,io.channel16,pending);fclose(rp_seam);rp_seam=0;rp_out=o;rp_rows=0;rp_next=rp_every;}
   if(snap_path&&now(s.words)>=snap_cs&&(!pending||snap_any)){snapshot(snap_path,&s,&io,&hw);printf("snapshot at %lu cs\n",now(s.words));
    if(!rp_out)return 0;
    /* FLIGHT_REPLAY: on, on the C64 window's work clock. */
    snap_path=0;rp_t0=now(s.words);rp_outputs(now(s.words),&hw,0);scaled=1;AgcFrozenClock=1;work=1;lag=~0u;slow=1.0;seed=rp_seed;jit_lo=0.8;jit_span=1.5625;consumed=0;wall=0.0;idling=0;charge_last=0;
    core_fit=core_yaagc=core_err=core_spans=0;}}
  {agc_word brupt=s.words[017];uint8_t got;
   if(scaled&&work){
    /* rk_rupts: a request that can be taken before each step due. */
    hold_on=rp_out&&!snap_path;
    /* While idling, wall is the idle period's end; the boundary is at the
     * last step the loop has run (where the take resets wall to). */
    while(!(got=take_held(&p,&s,&io,&hw,&pending,idling?(consumed?step_at(consumed-1ul):0.0):wall))&&step_at(consumed)<=wall){
     /* The drives run on the TIME5 step, as rk_rupts_due's. */
     if(consumed%3ul==2ul)drives(&s,&hw);
     rk_timer_phase(&s,&hw,&pending,(uint8_t)(consumed%3ul));
     if(hold_on)rupt_hold_raise(&hold,(uint16_t)(unsigned long)step_at(consumed),pending);
     if(consumed++%3ul==2ul){period_end();steal();}}
    if(got&&idling)wall=consumed?step_at(consumed-1ul):0.0;
    if(got&&hold_on)rupt_hold_taken(&hold,(uint16_t)(unsigned long)wall,s.words[025]);
    /* Only a taken interrupt ends the module chain: a C64 run its budget
     * cut at a timer step goes on in its module after the step (rk_cut). */
    if(got)charge_last=0;
    idling=0;}
   else{if(scaled){unsigned k;for(k=due();k;--k){rk_timer_tick(&s,&hw,&pending);drives(&s,&hw);steal();}}
    got=rk_rupt_take(&p,&s,io.superbank,io.interrupt_enabled,&pending);}
   /* The PIPAs count as the clock runs, not only at sample points: P66
    * reads them a few centiseconds after SERVICER, often with no idle pass
    * between. */
   if(rp_out&&!snap_path)rp_mark(s.words,&hw,io.channel16,pending);
   if(scaled&&engine_lag)engine_due(now(s.words),&eng_nat,s.words);
   rcs_due(now(s.words),s.words,&hw);
   if(rp_out&&!snap_path)rp_diff(now(s.words),1u,s.words,&hw,io.channel16,pending);
   if(got){
    /* At a RESUME point the instruction to run is BRUPT, not the word at
     * Z; rk_rupt_take saves the word at Z. */
    if(substitute&&brupt!=s.words[017]&&++lost_brupt==1ul)printf("interrupt at a RESUME point replaced BRUPT %05o with %05o at %lu cs\n",(unsigned)brupt,(unsigned)s.words[017],now(s.words));
    substitute=0;}}
  /* The TC trap (verb69): under the work clock, the spin's period ran
   * through its T5 step with no interrupt taken and held its T3 step, as
   * rk_resident_step finds it on the C64. */
  /* The night watchman sounded at the last period's end: GOJAM, Q the
   * point the AGC was at (night_watchman.h). */
  if(wm_fire){wm_fire=0;spin=0;++wm_alarms;printf("night watchman at %lu cs (Q %05o)\n",now(s.words),(unsigned)s.words[5]);
   gojam_native(s.words,&io,&hw,&pending);wm.hung=0;substitute=0;continue;}
  if(spin){int armed=spin==2;spin=0;
   if(armed&&!hw.in_interrupt){trap_gojam(&s,&io,&hw,&pending);substitute=0;continue;}}
  v69=!hw.in_interrupt&&!substitute&&verb69(s.words,io.superbank);
  /* At VERB69 the AGC runs nothing but TCs: under the work clock a spin
   * as the idle loop's, to the period's end (rk_resident_step's), with Q
   * TC VERB69's; unscaled, yaAGC's bridged code, to its own trap. */
  if(v69&&scaled){if(!work){fprintf(stderr,"FLIGHT: VERB69 under FLIGHT_SCALED=K,D,LAG: the TC trap needs the work clock or unscaled time\n");return 3;}
   s.words[2]=02040u;spin=consumed%3ul==2ul?1:2;idle_tick();continue;}
  if(rp_out&&!snap_path)rp_outputs(now(s.words),&hw,1);
  e=v69?0:holder(s.words,io.superbank,&call);
  if(!e&&!v69)e=&core_entry;
  /* FLIGHT_REPLAY_TRACE: each routine's Z, FB, TIME1, work clock (step,
   * MCT since it), Q and table, as dap_pass_check.c DP_STEPS records the
   * C64's steps with DP_REPLAY. */
  if(rp_out&&!snap_path&&getenv("FLIGHT_REPLAY_TRACE"))printf("RT %05o %05o %03o %lu %.0f Q %05o %s\n",(unsigned)s.words[5],(unsigned)s.words[4],(unsigned)(s.words[025]&0377u),consumed%9ul,wall-(consumed?step_at(consumed-1ul):0.0),(unsigned)s.words[2],e?e->name:"(bridge)");
  if(e&&(*e->module||wrapped_table(e)))module_load(e,s.words,io.superbank);
  if(getenv("FLIGHT_WATCH_STEP"))watch(s.words);
  /* A radar read starts at any routine boundary and the resident's cycle
   * ends it nine TIME5 counts on, often before the next sample point: the
   * model answers it at the boundary where channel 13 bit 4 comes on. A
   * read ends when bit 4 drops, often between sample points too; the next
   * read (LM R22's range after range rate) must find the model ready. */
  if(lrm.on&&!(hw.channel13&010u))lrm.active=0;
  else if(lrm.on&&!lrm.active&&(hw.channel13&04u)){if(rp_out&&!snap_path)rp_mark(s.words,&hw,io.channel16,pending);lrm_due(now(s.words),touchdown_cs,s.words,&hw.channel13,&hw.channel33);if(rp_out&&!snap_path)rp_diff(now(s.words),1u,s.words,&hw,io.channel16,pending);}
  if(rr_on&&!(hw.channel13&010u))rr_nat.active=0;
  else if(rr_on&&!rr_nat.active){int r=0;if(rp_out&&!snap_path)rp_mark(s.words,&hw,io.channel16,pending);rr_due(now(s.words),&rr_nat,s.words,&hw.channel12,&hw.channel13,&hw.channel14,&hw.channel33,&r);if(rp_out&&!snap_path)rp_diff(now(s.words),1u,s.words,&hw,io.channel16,pending);}
#if RK_LM
#endif
  if(path_to)path("NAT",s.words,io.superbank,hw.channel4,hw.in_interrupt,e?e->name:"(bridge)");
  if(e){flight_io io0=io;agc_hardware hw0=hw;const int isr0=hw.in_interrupt!=0u;long n;
   unsigned long t=steps%TRACE;trace[t][0]=(uint16_t)s.words[5];trace[t][1]=(uint16_t)s.words[4];names[t]=e->name;
   before=s;
   for(;;){const entry *next_e;
    p.operands=e->sites;p.operand_count=e->nsites;
    /* The module's fixed_data (flight_store_build.py FIXED_DATA): the
     * resident's boundary words, self-check's rope hook, or every rope word. */
#if RK_LM
    p.fixed_data=e->fixed==1u?lm_self_check_rope_data:e->fixed==2u?rope_data:boundary_word;
#else
    p.fixed_data=e->fixed==1u?cm_self_check_rope_data:e->fixed==2u?rope_data:boundary_word;
#endif
    core_ran=0;
    if(weight(e)==FLIGHT_PER_WORD){
     x=flight_io_enter(CORE_IO,&io,&hw,&b);st=core_run(&p,&s,x);flight_io_leave(CORE_IO,&io,&hw,&b);
     if(st==EXEC_OK){agc_routine c2;core_ran=1;
      if(getenv("FLIGHT_CORE_LOG")&&!holder(s.words,io.superbank,&c2))
       printf("core left an unheld point: Z %05o FB %05o LOC %05o (from Z %05o LOC %05o, %s, %lu cs)\n",(unsigned)s.words[5],(unsigned)s.words[4],
        (unsigned)s.words[0164],(unsigned)before.words[5],(unsigned)before.words[0164],e->name,now(s.words));
      break;}
     if(getenv("FLIGHT_CORE_LOG"))printf("core refused: status %d Z %05o FB %05o LOC %05o BANKSET %05o EDOP %05o, %s\n",(int)st,(unsigned)before.words[5],
      (unsigned)before.words[4],(unsigned)before.words[0164],(unsigned)before.words[0165],(unsigned)before.words[AGC_EDOP],e->name);
     s=before;io=io0;hw=hw0;
     if(e==&core_entry)break;}
    x=flight_io_enter(e->io,&io,&hw,&b);st=call(&p,&s,x);flight_io_leave(e->io,&io,&hw,&b);
    /* A pinned table that refuses the point (slot 3 or 4) leaves it to the
     * next holder, as rk_direct does on the C64 (a slot that runs no step). */
    if(st==EXEC_OK||e->slot<3u||e->slot>4u||!(next_e=holder_after(before.words,io0.superbank,&call,e)))break;
    s=before;io=io0;hw=hw0;e=next_e;names[t]=e->name;
    if(*e->module||wrapped_table(e))module_load(e,s.words,io.superbank);}
   if(st==EXEC_OK){uint16_t pending0=pending;agc_hardware hw1=hw0;int interp;(void)point(before.words,io0.superbank,&interp);
    n=lockstep(&before,&io0,&hw0,&s,io.superbank,&hw,&pending,50000l,first,interp);
    /* Again with the scalers yaAGC read (scaler_latch). */
    if(n<0&&!scaled&&scaler_latch(&before,&io0,&hw1,&s,io.superbank,50000l)){
     s=before;io=io0;hw=hw1;pending=pending0;
     x=flight_io_enter(e->io,&io,&hw,&b);st=call(&p,&s,x);flight_io_leave(e->io,&io,&hw,&b);
     dl_clear();
     if(st==EXEC_OK)n=lockstep(&before,&io0,&hw0,&s,io.superbank,&hw,&pending,50000l,first,interp);}
    if(st==EXEC_OK){
     if(core_ran){++core_routines;
      if(n>=0){core_fit+=core_mct;core_yaagc+=span_mct;core_err+=core_mct>span_mct?core_mct-span_mct:span_mct-core_mct;++core_spans;}
      if(n>=0&&core_mct!=word_mct){++core_misses;
       if(getenv("FLIGHT_CORE_LOG"))printf("core charge %lu, yaAGC's words %lu: Z %05o LOC %05o to Z %05o LOC %05o, %s\n",core_mct,word_mct,
        (unsigned)before.words[5],(unsigned)before.words[0164],(unsigned)s.words[5],(unsigned)s.words[0164],e->name);}}
     if(scaled){const int rp_on=rp_out&&!snap_path;const entry *ce=work&&rp_on&&!core_ran&&weight(e)!=FLIGHT_PER_WORD?charged(before.words,io0.superbank,e,charge_last):e;unsigned w=!weight(e)||weight(ce)==FLIGHT_PER_WORD||!weight(ce)?weight(e):weight(ce);
      charge_last=core_ran||weight(e)==FLIGHT_PER_WORD||kernel_table(ce)?0:ce;if(core_ran&&work)spend(core_mct);else if(work&&w==FLIGHT_PER_WORD)spend(word_mct);else if(work&&(w&FLIGHT_PLUS_WORDS))spend((w&~FLIGHT_PLUS_WORDS)+word_mct);else if(work&&!w&&(!rp_on||(w=idle_weight(before.words,io0.superbank,isr0))==FLIGHT_IDLE_TICK))idle_tick();else spend(work?w:span_mct);}
     /* A stuck step (an erasable CADR cycle in GOTO) spins to the
      * period's end, interrupts taken, until the night watchman. */
     {int stuck=nw_stuck(before.words,s.words,isr0,interp);if(nw_step(&wm,before.words,s.words,isr0,stuck))wm.armed=0;
      if(!m.NightWatchman)wm_ya.armed=0;
      ya_tripped();
      /* The C64's boundary words hold no stuck point (GOTO), so no
       * interrupt could come there: the AGC and the x86 program take them
       * (night_watchman.h), and so does the next boundary here. */
      /* Only for the boundary after it: the next routine sets fixed_data again. */
      if(stuck){++wm_stuck;p.fixed_data=rope_data;if(work)idle_tick();}}
     dl_routine(e->name,n,&before,&s,&io0,&io,&hw);
     if(n>=0){locked+=(unsigned long)n;
      /* FLIGHT_MCT_OUT: each matched routine's machine cycles in yaAGC, by
       * point and table (tools/x86_flight_mct.py). */
      if(mct_out){int in;uint16_t k=point(before.words,io0.superbank,&in);fprintf(mct_out,"%d %u %lu %s\n",in,k,span_mct,e->name);}}
     else{miss(e->name,before.words,io0.superbank,s.words,first);
      /* FLIGHT_MISS_LOG=DIR also writes the erasable state the routine started
       * in (octal ADDR VALUE lines) and names the routines before it. */
      if(getenv("FLIGHT_MISS_LOG")&&*getenv("FLIGHT_MISS_LOG")!='1'){static unsigned dumped;char path[600];FILE *f;unsigned b;
       for(b=1;b<=3u&&b<=steps;++b){unsigned long u=(steps-b)%TRACE;printf("  before it (%u): Z %05o FB %05o %s\n",b,(unsigned)trace[u][0],(unsigned)trace[u][1],names[u]);}
       if(dumped<8u){snprintf(path,sizeof path,"%s/miss-%lu-%05o-%05o.txt",getenv("FLIGHT_MISS_LOG"),now(before.words),(unsigned)before.words[4],(unsigned)before.words[5]);
        if((f=fopen(path,"w"))){fprintf(f,"# %s, native state at entry (%lu cs), superbank %o\n",e->name,now(before.words),(unsigned)io0.superbank);
         for(b=0;b<2048u;++b){fprintf(f,"%04o %06o\n",b,(unsigned)before.words[b]);}
         fclose(f);++dumped;}}}}
     continue;}}
   /* A refusal: undo whatever the routine did and bridge the point. */
   s=before;io=io0;hw=hw0;}
  dl_clear();
  {agc_word w[2048];long n;memcpy(w,s.words,sizeof w);
   if((n=bridge(&s,&io,&hw,&pending,5000000l))==-2)return 3;
   if(n<0){printf("bridge did not reach a registered point after 5000000 instructions\n");where(w,io.superbank,"from");where(s.words,io.superbank,"at");break;}
   dl_bridged(w);
   {int in,stuck;(void)point(w,io.superbank,&in);stuck=nw_stuck(w,s.words,hw.in_interrupt!=0u,in);if(nw_step(&wm,w,s.words,hw.in_interrupt!=0u,stuck))wm.armed=0;if(!m.NightWatchman)wm_ya.armed=0;ya_tripped();}
   hole(w,io.superbank,(unsigned long)n,e&&e!=&core_entry?e->name:0);bridged+=(unsigned long)n;
   if(trapped){trap_gojam(&s,&io,&hw,&pending);substitute=0;}}}
 printf("%s: %lu native routines (%lu yaAGC instructions of time), %lu bridged yaAGC instructions; %lu samples: %lu equal, %lu differ\n",
  MISSION?"CM":"LM",steps,locked,bridged,samples,samples-diffs,diffs);
 if(scaled)printf("scaled time: K %.2f, divisor %u, lag %u, seed %lu: %lu CIA interrupts, %lu clock periods taken, %lu dropped (rk_dilation); AGC time %lu cs\n",
  slow,divisor,lag,seed,work?consumed/3ul:consumed,taken,dropped,now(s.words));   /* work: whole periods' steps */
 if(lost_brupt)printf("%lu interrupts at a RESUME point replaced BRUPT\n",lost_brupt);
 if(scaled)printf("night watchman: %lu alarms, %lu stuck steps; %lu checks while armed, %lu where yaAGC's NEWJOB accesses differ from the native rule; yaAGC's own alarm inside %lu spans\n",wm_alarms,wm_stuck,wm_windows,wm_disagree,wm_ya_tripped);
 if(refs){const agc_word *r=ref[refs<sizeof ref/sizeof ref[0]?refs:0u];   /* scaled: no reference, refs may pass its end */
  printf("major mode (MODREG 01011) at %lu cs: native %02o, yaAGC %02o; FAILREG %05o/%05o\n",refs,(unsigned)s.words[01011],(unsigned)r[01011],(unsigned)s.words[0375],(unsigned)r[0375]);}
 if(core_routines){printf("interpreter module: %lu routines, %lu slots; %lu charged other than yaAGC's words\n",core_routines,core_slots,core_misses);core_charge_report();}
 printf("native routines yaAGC did not match (%u):\n",nmisses);
 for(i=0;i<nmisses;++i){unsigned k,d=0;printf("  %s %02o,%04o %s: %lu times;",misses[i].in?"interp":"basic ",misses[i].key/1024u,misses[i].key%1024u+02000u,misses[i].name,misses[i].count);
  if(!misses[i].arrived)printf(" yaAGC never reached the native continuation Z %05o\n",(unsigned)misses[i].word[5]);
  else{for(k=0;k<2048u;++k)if(!(k>=024u&&k<=031u)&&misses[i].word[k]!=misses[i].ref[k]&&d++<6u)printf(" %04o %05o/%05o",k,(unsigned)misses[i].word[k],(unsigned)misses[i].ref[k]);
   printf("%s%s (native/yaAGC)\n",d>6u?" ...":"",misses[i].chan);}}
 for(i=0;i<nreach;++i){if(reach_cs[i]<0)printf("reach %02o,%04o: not reached\n",reach_at[i]/1024u,reach_at[i]%1024u+02000u);
  else printf("reach %02o,%04o: at %ld cs\n",reach_at[i]/1024u,reach_at[i]%1024u+02000u,reach_cs[i]);}
 printf("unregistered or refused points (%u):\n",nholes);
 for(i=0;i<nholes;++i){printf("  %s %02o,%04o Z %05o FB %05o Q %05o",holes[i].in?"interp":"basic ",
  holes[i].key/1024u,holes[i].key%1024u+02000u,(unsigned)holes[i].z,(unsigned)holes[i].fb,(unsigned)holes[i].q);
  /* Q's own fixed point, so a bridged TC Q return names its routine
   * (the TC at Q - 1 called it; tools/flight_holes.py). */
  if(holes[i].qat!=AGC_TRANSFER_NONE)printf(" at %02o,%04o",holes[i].qat/1024u,holes[i].qat%1024u+02000u);
  printf(": %lu arrivals, %lu instructions%s%s\n",holes[i].count,holes[i].instructions,holes[i].by?", refused by ":"",holes[i].by?holes[i].by:"");}
 printf("module loads (%lu in %lu cs):\n",nloads,now(s.words));
 for(i=0;i<nload_points;++i)printf("  load %s slot %u %s %02o,%04o: %lu\n",loads[i].module,loads[i].slot,loads[i].in?"interp":"basic ",
  loads[i].key/1024u,loads[i].key%1024u+02000u,loads[i].count);
 dl_report();
 printf("last routines:\n");
 for(i=0;i<TRACE&&i<steps;++i){unsigned long t=(steps-1u-i)%TRACE;printf("  Z %05o FB %05o %s\n",trace[t][0],trace[t][1],names[t]);}
 return 0;}
