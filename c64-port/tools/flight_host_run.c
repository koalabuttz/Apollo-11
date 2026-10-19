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
 * (rk_rupts); the IMU CDU
 * drive, timed by yaAGC's cycle counter, stops with it, as do the optics
 * and gyro drives (no C64 resident has them). Bridged code
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
#include "rope_image.h"
#include "self_check.h"
#include "standard_lead_routines.h"
#include "flight_gojam.h"
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
/* The work clock's idle loop (weight 0, RK_MCT_IDLE): to the end of the
 * period, which the boundary drops if it takes a request first. */
static int idling;
static void idle_tick(void){wall=step_at(consumed+2ul-consumed%3ul);idling=1;}
/* rk_rupts: the CIA1 interrupts since the last boundary, at most lag of
 * them as clock periods, the rest dropped. */
static unsigned due(void){unsigned long now=(unsigned long)(wall/(85333.0/100.0*divisor));unsigned n=0;
 for(;consumed<now;++consumed){if(n<lag){++n;++taken;}else ++dropped;}
 return n;}
/* The work clock's steps due, for bridged code (yaAGC takes their
 * requests between its instructions). */
static unsigned due_steps(void){unsigned n=0;
 for(;step_at(consumed)<=wall;++consumed,++n)if(consumed%3ul==2ul)++taken;
 return n;}
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
/* A ride table (slot 5, flight_store_build.py RIDE) holds its keys only
 * while its pack is loaded in the job slot. */
/* after: the holder that refused (a pinned table: the next one may take
 * the point, as rk_direct moves on when a slot runs no step), or 0. */
static const entry *holder_after(const agc_word *w,agc_word sb,agc_routine *call,const entry *after){int in;unsigned i;uint16_t key=point(w,sb,&in);
 for(i=after?(unsigned)(after-tables)+1u:0u;i<TABLES;++i)if(tables[i].interp==(uint8_t)in&&(tables[i].slot!=5u||in_slot[0]==tables[i].module)
                         &&(*call=find(tables[i].t,key)))return &tables[i];
 return 0;}
static const entry *holder(const agc_word *w,agc_word sb,agc_routine *call){return holder_after(w,sb,call,0);}
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
enum { KEYS=1024 };
static char keyc[KEYS+1];static unsigned long keyt[KEYS];
static void keys_parse(const char *t){unsigned long at=KEY_AT;unsigned n=0;
 while(*t&&n<KEYS){if(*t=='@'){char *e;unsigned long v=strtoul(t+1,&e,10);if(v>at)at=v;t=e;continue;}
  keyc[n]=*t++;keyt[n++]=at;at+=KEY_GAP;}
 keyc[n]=0;keys=keyc;}
static agc_word keycode(char c){return c=='V'?021u:c=='N'?037u:c=='E'?034u:c=='0'?020u:c=='+'?032u:c=='-'?033u:
 c=='C'?036u:c=='P'?022u:c=='K'?031u:c=='R'?022u:(agc_word)(c-'0');}
/* M and J are the CM's MARK and MARK REJECT buttons (CM P52): channel 16
 * bit 6 or bit 7 and a KEYRUPT2 request, which MARKRUPT takes; x, y and
 * j are the LM's X MARK, Y MARK and MARK REJECT (channel 16 bits 3, 4
 * and 5, LM P57). key_due returns them as MARK_KEY | the channel 16 bits. */
#define MARK_KEY 0x100
static int key_due(unsigned long t,unsigned *pressed){char c;if(!keys[*pressed]||t<keyt[*pressed])return -1;c=keys[(*pressed)++];
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
 fprintf(f,"\n");fclose(f);}
/* FLIGHT_REPLAY=CS (with FLIGHT_SNAPSHOT): the C64 window's inputs and
 * its end state (tools/flight_scenario_vice.py --replay). After the
 * snapshot the run goes on, on the work clock from its first step (the
 * C64 window's own clock: rk_work and rk_tick_phase 0), for CS
 * centiseconds. Every input the models, keys, pokes and channel changes
 * give it in that time is one 8-byte row of PATH.replay (little-endian:
 * centiseconds since the snapshot, kind, where, address, value): where 0
 * at a sample point, 1 at any routine boundary (the engine lag's PIPA
 * counts, the radar model's reads). Kinds: 0 an erasable word set, 1 a
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
static long ones(agc_word v);
static FILE *rp_out;static unsigned long rp_seed;
static unsigned long rp_t0,rp_cs,rp_rows;static unsigned rp_chain=1u,rp_win;static FILE *rp_seam;
static char rp_base[512],rp_path[600];
static agc_word rp_w[2048];static agc_hardware rp_hw;static agc_word rp_c16;static uint16_t rp_pending;
static void rp_row(unsigned long t,unsigned kind,unsigned where,unsigned a,unsigned v){unsigned char b[8];unsigned long d=t-rp_t0;
 if(d>0xffffu){fprintf(stderr,"FLIGHT_REPLAY: window longer than 65535 cs\n");exit(2);}
 b[0]=(unsigned char)d;b[1]=(unsigned char)(d>>8);b[2]=(unsigned char)kind;b[3]=(unsigned char)where;
 b[4]=(unsigned char)a;b[5]=(unsigned char)(a>>8);b[6]=(unsigned char)v;b[7]=(unsigned char)(v>>8);
 fwrite(b,1,8,rp_out);++rp_rows;}
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
static void rp_end(unsigned long d,const agc_word *w,const flight_io *io,const agc_hardware *hw){FILE *f=rp_ends;unsigned i;
 uint16_t c[]={hw->channel5,hw->channel6,hw->channel11,hw->channel12,hw->channel13,hw->channel14,io->channel34,io->channel35,io->superbank};
 {unsigned char b[2]={(unsigned char)d,(unsigned char)(d>>8)};fwrite(b,1,2,f);}
 for(i=0;i<2048u;++i){unsigned char b[2]={(unsigned char)w[i],(unsigned char)(w[i]>>8)};fwrite(b,1,2,f);}
 for(i=0;i<sizeof c/sizeof c[0];++i){unsigned char b[2]={(unsigned char)c[i],(unsigned char)(c[i]>>8)};fwrite(b,1,2,f);}}
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

/* FLIGHT_SPS=KG (CM): the service propulsion engine, the same on both
 * sides, so P40 sees its burn. From the first sample point with ENGONFLG
 * set (FLAGWRD5 bit 7) it thrusts 91,188 N along the engine bell, body X
 * turned by the mechanical biases P40 assumes (PBIAS -2.15, YBIAS +0.95
 * degrees; small angles: CDUY less 2.15 and CDUZ plus 0.95 degrees in the
 * FLIGHT_ENGINE direction), burning KG kilograms down at 3,080 m/s, and each sample
 * point adds the velocity since the last one to PIPAX-Z (5.85 cm/s a
 * count), carrying the fraction. No attitude dynamics: each sample point
 * sets CDUX-Z to the RCS DAP's desired angles CDUXD-ZD (perfect attitude
 * control, so P40's R60 maneuver ends) until the engine first lights;
 * from then on the TVC DAP reuses those cells as temporaries, so the
 * attitude holds at ignition for the rest of the run. */
static double sps_kg;
enum { CM_CDUXD=03246 };
static struct sps { int on,lit; unsigned long t; double kg,carry[3]; } sps_ref,sps_nat;
static void sps_due(unsigned long t,struct sps *e,agc_word *w){double a,dt,ig,mg,dir[3];unsigned k;
 if(!sps_kg)return;
 if(!e->lit)for(k=0;k<3u;++k){if(w)w[CDUX+k]=eword(w,CM_CDUXD+2u*k);else *cell(CDUX+k)=(int16_t)eword(w,CM_CDUXD+2u*k);}
 if(!(eword(w,FLAGWRD5)&ENGONBIT)){e->on=0;return;}
 if(!e->on){e->on=e->lit=1;e->t=t;if(!e->kg)e->kg=sps_kg;return;}
 dt=(double)(t-e->t)*0.01;e->t=t;a=91188.0/e->kg;e->kg-=91188.0/3080.0*dt;
 ig=(double)eword(w,CDUY)*(6.283185307179586/32768.0)-0.0375245789;mg=(double)eword(w,CDUZ)*(6.283185307179586/32768.0)+0.0165806279;
 dir[0]=cos(ig)*cos(mg);dir[1]=sin(mg);dir[2]=-sin(ig)*cos(mg);
 for(k=0;k<3u;++k){long n,v;e->carry[k]+=a*dt*dir[k]*100.0/5.85;n=(long)e->carry[k];e->carry[k]-=(double)n;
  v=ones(eword(w,PIPAX+k))+n;if(v>037777l)v=037777l;if(v<-037777l)v=-037777l;
  v=v<0?(long)((~(unsigned long)(-v))&077777ul):v;
  if(w)w[PIPAX+k]=(agc_word)v;else *cell(PIPAX+k)=(int16_t)v;}}

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
static struct { uint16_t key; uint8_t in; agc_word z,fb,q; unsigned long count,instructions; const char *by; } holes[HOLES];
static unsigned nholes;
static void hole(const agc_word *w,agc_word sb,unsigned long instructions,const char *by){int in;uint16_t key=point(w,sb,&in);unsigned i;
 for(i=0;i<nholes;++i)if(holes[i].key==key&&holes[i].in==(uint8_t)in&&holes[i].by==by){++holes[i].count;holes[i].instructions+=instructions;return;}
 if(nholes<HOLES){holes[nholes].key=key;holes[nholes].in=(uint8_t)in;holes[nholes].z=w[5];holes[nholes].fb=w[4];holes[nholes].q=w[2];
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
 * Executive), both outside interrupts. A word whose job changed meanwhile
 * (FIXLOC) is not written. */
static FILE *word_out;
static struct { int open; unsigned long cs; uint16_t loc, bankset, fixloc; uint64_t in, out; } word;
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
/* One clock period (rk_timer_tick), or with step >= 0 one work-clock step
 * (rk_timer_phase), on yaAGC's state, for bridged code. */
static void ref_tick(int step){static apollo_overlay_state t;agc_hardware h;uint16_t p=0;unsigned i;
 for(i=024u;i<=031u;++i)t.words[i]=mword(i);
 memset(&h,0,sizeof h);h.channel13=(agc_word)(m.InputChannel[013]&077777);
 h.channel3=(agc_word)(m.InputChannel[3]&077777);h.channel4=(agc_word)(m.InputChannel[4]&077777);
 h.radar_gate=(uint8_t)m.RadarGateCounter;
 if(step<0)rk_timer_tick(&t,&h,&p);else rk_timer_phase(&t,&h,&p,(uint8_t)step);
 m.RadarGateCounter=h.radar_gate;
 for(i=024u;i<=031u;++i)*cell(i)=(int16_t)t.words[i];
 m.InputChannel[013]=(int16_t)h.channel13;m.InputChannel[3]=(int16_t)h.channel3;m.InputChannel[4]=(int16_t)h.channel4;
 for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(p&RK_RUPT_BIT(i))m.InterruptRequests[i]=1;}
/* One yaAGC instruction with its clock running; the interrupts it requests
 * are left pending for the native side to take. */
static int step(uint16_t *pending){unsigned i;if(agc_engine(&m))return -1;
 for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(m.InterruptRequests[i]){*pending|=RK_RUPT_BIT(i);m.InterruptRequests[i]=0;}
 return 0;}

/* Bridge: yaAGC from the native state until a point some table holds or
 * idle (after at least one instruction). Bridged code is yaAGC's own, so
 * yaAGC takes the native side's pending interrupts, and any it requests,
 * where it would (an interrupt vector is a point the lead-in tables hold);
 * the requests still pending at the end go back to the native side. */
static long bridge(apollo_overlay_state *s,flight_io *io,agc_hardware *hw,uint16_t *pending,long limit){long n;unsigned i;agc_routine call;
 load(s,io,hw);
 for(i=1;i<=NUM_INTERRUPT_TYPES;++i)m.InterruptRequests[i]=(int8_t)((*pending&RK_RUPT_BIT(i))!=0u);
 for(n=0;n<limit;++n){
  if(n&&quiet()){const agc_word *v=view();if(holder(v,(agc_word)m.OutputChannel7,&call)||idle(v,(agc_word)m.OutputChannel7,m.InIsr))break;}
  if(scaled){uint64_t c=m.CycleCounter;unsigned k;if(agc_engine(&m))return -1;spend((unsigned long)(m.CycleCounter-c));
   if(quiet()){if(work){unsigned long j=consumed;for(k=due_steps();k;--k,++j){ref_tick((int)(j%3ul));if(j%3ul==2ul)steal();}}else for(k=due();k;--k){ref_tick(-1);steal();}}
   continue;}
  if(agc_engine(&m))return -1;}
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
static long lockstep(const apollo_overlay_state *before,const flight_io *io0,agc_hardware *hw0,apollo_overlay_state *after,
                     agc_word superbank,agc_hardware *hw,uint16_t *pending,long limit,agc_word *first,int interp){
 static apollo_overlay_state timed;static agc_hardware timed_hw;
 long n;int in,tin;uint16_t target=point(after->words,superbank,&tin),got=0,got_first=0;unsigned i;uint64_t c0;unsigned long first_mct=0;
 first[5]=077777u;cdu_driven=(agc_word)((hw0->channel14|hw->channel14)&070000u);load(before,io0,hw0);c0=m.CycleCounter;
 for(n=0;n<limit;++n){
  /* After RESUME, yaAGC is at ZRUPT with BRUPT substituted for the next
   * instruction: the native continuation is that point. */
  if(n&&(quiet()||(m.SubstituteInstruction&&!m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode))
     &&point(view(),(agc_word)m.OutputChannel7,&in)==target&&in==tin){
   /* An interpretive step (SWITCH's TS Q) may leave yaAGC's Q with bit 16,
    * a copy of bit 15, where the native frontier holds 15 bits (#409). */
   for(i=0;i<2048u;++i)if((scaled||!(i>=024u&&i<=031u))&&i!=5u&&!cdu_counter(i)
                          &&(i==2u&&interp?(mword(2)^after->words[2])&077777u:mword(i)!=after->words[i]))break;
   if(i==2048u){adopt(after,hw);*pending|=got;substitute=m.SubstituteInstruction;span_mct=(unsigned long)(m.CycleCounter-c0);return n;}
   if(first[5]==077777u){for(i=0;i<2048u;++i)first[i]=mword(i);first_mct=(unsigned long)(m.CycleCounter-c0);timed=*after;timed_hw=*hw;adopt(&timed,&timed_hw);got_first=got;}}
  if(step(&got))return -1;}
 /* A miss: the routine took as long as yaAGC to its first arrival there
  * (none when it never arrived). */
 span_mct=first[5]!=077777u?first_mct:0ul;
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
static struct { const char *name; uint16_t key; uint8_t in; unsigned long count; agc_word word[2048],ref[2048]; int arrived; } misses[MISSES];
static unsigned nmisses;
static void miss(const char *name,const agc_word *before,agc_word sb,const agc_word *after,const agc_word *first){int in;uint16_t key=point(before,sb,&in);unsigned i;
 if(getenv("FLIGHT_MISS_LOG"))printf("miss %s at %lu cs: Z %05o FB %05o Q %05o (before), Q %05o/%05o after (native/yaAGC)\n",name,now(before),(unsigned)before[5],(unsigned)before[4],(unsigned)before[2],(unsigned)after[2],(unsigned)first[2]);
 for(i=0;i<nmisses;++i)if(misses[i].name==name&&misses[i].key==key&&misses[i].in==(uint8_t)in){++misses[i].count;return;}
 if(nmisses<MISSES){misses[nmisses].name=name;misses[nmisses].key=key;misses[nmisses].in=(uint8_t)in;misses[nmisses].count=1;
  memcpy(misses[nmisses].word,after,sizeof misses[nmisses].word);memcpy(misses[nmisses].ref,first,sizeof misses[nmisses].ref);
  misses[nmisses].arrived=first[5]!=077777u;++nmisses;}}

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
static void module_load(const entry *e,const agc_word *w,agc_word sb){int in;uint16_t key;unsigned i;
 if(e->slot>1||in_slot[e->slot]==e->module)return;
 /* A table packed into the loaded module too runs there (CO_LOCATE). */
 if(in_slot[e->slot]&&*e->also){const char *a=strstr(e->also,in_slot[e->slot]);size_t n=strlen(in_slot[e->slot]);
  if(a&&(a==e->also||a[-1]==' ')&&(a[n]==0||a[n]==' '))return;}
 in_slot[e->slot]=e->module;++nloads;key=point(w,sb,&in);
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
  if(word.open&&fixloc==word.fixloc)fprintf(word_out,"%lu %o %o %llu %llu\n",word.cs,(unsigned)word.loc,(unsigned)word.bankset,
                                            (unsigned long long)word.in,(unsigned long long)word.out);
  word.open=1;word.cs=cs;word.loc=(uint16_t)(*cell(0164)&077777);word.bankset=(uint16_t)(*cell(0165)&077777);word.fixloc=fixloc;word.in=word.out=0;}
 if(agc_engine(&m)){fclose(word_out);exit(2);}
 if(!isr&&!m.InIsr){if(at!=AGC_TRANSFER_NONE&&(at/1024u==0u||at/1024u==3u))word.in+=m.CycleCounter-c;else word.out+=m.CycleCounter-c;}}
int main(int argc,char **argv){static apollo_overlay_state s,undo,before;static agc_word ref[32768][2048],first[2048];
 flight_io io;agc_hardware hw;agc_native_program p;uint16_t pending=0;
 unsigned long cs=argc>2?strtoul(argv[2],0,0):100ul,steps,next,refs,limit,bridged=0,locked=0;
 unsigned i,j,kref=0,knat=0,uref=0,unat=0,cref=0,cnat=0,pref=0,pnat=0,gref=0,gnat=0;int latency=getenv("FLIGHT_NATIVE_LATENCY")!=0;unsigned long diffs=0,samples=0,lost_brupt=0;
 enum { TRACE=24 };uint16_t trace[TRACE][3];const char *names[TRACE];
 if(argc<2||!apollo_rope_load(MISSION,argv[1])||agc_load_binfile(&loaded,argv[1])){fprintf(stderr,"usage: flight-host-run ROPE.bin [CS [KEYS]]\n");return 2;}
 keys_parse(argc>3?argv[3]:"");
 if(!uplink_parse(getenv("FLIGHT_UPLINK"))){fprintf(stderr,"FLIGHT_UPLINK=CS:KEYS[;CS:KEYS...] (#OCTAL a raw word)\n");return 2;}
 if(!steal_parse(getenv("FLIGHT_STEAL"))){fprintf(stderr,"FLIGHT_STEAL=T0-T1:MCT[,T0-T1:MCT...]\n");return 2;}
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
   snprintf(rp_buf,sizeof rp_buf,"%s.end",snap_path);if(!(rp_ends=fopen(rp_buf,"wb"))){perror(rp_buf);return 2;}}}
 if(!load_parse(getenv("FLIGHT_LOAD")))return 2;
 if(!gojam_parse(getenv("FLIGHT_GOJAM"))){fprintf(stderr,"FLIGHT_GOJAM=CS[,CS...]\n");return 2;}
 dl_parse();
 pokes_sort();
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
 if(getenv("FLIGHT_SPS"))sps_kg=strtod(getenv("FLIGHT_SPS"),0);
 if(getenv("FLIGHT_IMU"))imu_model=atoi(getenv("FLIGHT_IMU"));
 if(getenv("FLIGHT_SATURN"))saturn_lo=strtoul(getenv("FLIGHT_SATURN"),0,10);
 if(!rr_parse(getenv("FLIGHT_RR"))){fprintf(stderr,"FLIGHT_RR=CS:rx,ry,rz,vx,vy,vz:rx,ry,rz,vx,vy,vz\n");return 2;}
 if(rr_on&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_RR needs FLIGHT_SCALED\n");return 2;}
 if(!drops_parse(&rr_drops,getenv("FLIGHT_RR_DROP"))){fprintf(stderr,"FLIGHT_RR_DROP=T0-T1[,T0-T1...]\n");return 2;}
 if(getenv("FLIGHT_SIVB")&&sscanf(getenv("FLIGHT_SIVB"),"%lu,%lf,%lf,%lf",&sivb_tig,&sivb_u[0],&sivb_u[1],&sivb_u[2])!=4){fprintf(stderr,"FLIGHT_SIVB=CS,UX,UY,UZ\n");return 2;}
#endif
 core_path=getenv("FLIGHT_CORE");
 if(core_path&&!getenv("FLIGHT_SCALED")){fprintf(stderr,"FLIGHT_CORE needs FLIGHT_SCALED\n");return 2;}
 /* core_start's yaAGC steps to the idle loop would land in the downlink
  * reference stream, so the two are not combined (no scenario sets both). */
 if(core_path&&getenv("FLIGHT_DOWNRUPT")&&*getenv("FLIGHT_DOWNRUPT")&&strcmp(getenv("FLIGHT_DOWNRUPT"),"0")){fprintf(stderr,"FLIGHT_CORE does not take FLIGHT_DOWNRUPT\n");return 2;}
 if(getenv("FLIGHT_PATH"))sscanf(getenv("FLIGHT_PATH"),"%lu,%lu",&path_from,&path_to);
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
    sps_due(now(w),&sps_ref,0);
    aero_due(now(w),&aero_ref,0);
    imu_due(0,0);
    saturn_due(now(w),&sat_ref,0,0);
    sivb_due(now(w),&sivb_ref,0);
    lr_due(now(w),&lr_ref,0);
    chan_due(now(w),&cref,0,0);
    if(gojam_due(now(w),&gref))gojam_yaagc(&m);
    else if((k=key_due(now(w),&kref))>=0){unsigned r=k&MARK_KEY?RK_RUPT_KEY2:RK_RUPT_KEY1;m.InputChannel[k&MARK_KEY?016:015]=(int16_t)(k&0377);
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
  if(agc_engine(&m))return 2;}
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
 for(steps=0,next=1;next<=refs&&steps<20000000ul;++steps){agc_routine call=0;const entry *e;flight_io_buffer b;void *x;enum exec_status st;
  steal_cs=now(s.words);
  if(!pending&&(scaled||(hw.channel4&020u))&&((at_advan(s.words,io.superbank)&&idle(s.words,io.superbank,hw.in_interrupt))||busy_due(scaled,s.words,hw.in_interrupt))){int k;busy_last=now(s.words);
   while(next<=refs&&now(s.words)>=next){if(scaled){++next;continue;}++samples;
    if((j=differs(s.words,ref[next]))!=0u&&++diffs<=(getenv("FLIGHT_SHOW")?strtoul(getenv("FLIGHT_SHOW"),0,0):20ul)){
     printf("%lu cs (native routine %lu):",next,steps);
     for(i=0,j=0;i<2048u;++i)if(compared(i)&&s.words[i]!=ref[next][i]&&j++<8u)printf(" %04o %05o/%05o",i,(unsigned)s.words[i],(unsigned)ref[next][i]);
     printf("%s (%u words, native/yaAGC)\n",j>8u?" ...":"",j);}
    ++next;}
   if(rp_out&&!snap_path&&now(s.words)-rp_t0>=rp_next&&now(s.words)-rp_t0<rp_cs){rp_end(now(s.words)-rp_t0,s.words,&io,&hw);
    while(rp_next<=now(s.words)-rp_t0)rp_next+=rp_every;}
   if(rp_out&&!snap_path&&now(s.words)-rp_t0>=rp_cs&&(rp_win+1u>=rp_chain||!pending)){
    rp_end(now(s.words)-rp_t0,s.words,&io,&hw);fclose(rp_out);fclose(rp_ends);printf("replay end at %lu cs (%lu cs after the snapshot): %lu input rows\n",now(s.words),now(s.words)-rp_t0,rp_rows);
    if(++rp_win>=rp_chain)return 0;
    /* CHAIN: the next window starts here (FLIGHT_REPLAY above). */
    snprintf(rp_path,sizeof rp_path,"%s-%u",rp_base,rp_win);snap_path=rp_path;snap_cs=now(s.words);snap_any=0;
    {char b[640];snprintf(b,sizeof b,"%s.replay",rp_path);rp_out=fopen(b,"wb");snprintf(b,sizeof b,"%s.end",rp_path);rp_ends=fopen(b,"wb");
     snprintf(b,sizeof b,"%s.seam",rp_path);rp_seam=fopen(b,"wb");if(!rp_out||!rp_ends||!rp_seam){perror(b);return 2;}}
    rp_mark(s.words,&hw,io.channel16,pending);}
   if(rp_out&&!snap_path)rp_mark(s.words,&hw,io.channel16,pending);
   /* A key is pressed at the first sample point at or after its time, on
    * both sides. */
   poke_due(now(s.words),&pnat,s.words);
   engine_due(now(s.words),&eng_nat,s.words);
   sps_due(now(s.words),&sps_nat,s.words);
   if(getenv("FLIGHT_DSKY"))dsky_trace(now(s.words),s.words);
   {int r=0;rr_due(now(s.words),&rr_nat,s.words,&hw.channel12,&hw.channel13,&hw.channel14,&hw.channel33,&r);if(r)pending|=RK_RUPT_BIT(RK_RUPT_RADAR);}
   watch(s.words);
   aero_due(now(s.words),&aero_nat,s.words);
   imu_due(s.words,&hw);
   saturn_due(now(s.words),&sat_nat,s.words,&hw);
   sivb_due(now(s.words),&sivb_nat,s.words);
   lr_due(now(s.words),&lr_nat,&hw);
   lrm_due(now(s.words),touchdown_cs,s.words,&hw.channel13,&hw.channel33);
   chan_due(now(s.words),&cnat,&hw,&io);
   if(rp_out&&!snap_path&&gojam_due(now(s.words),&gnat)){fprintf(stderr,"FLIGHT_REPLAY: a GOJAM at %lu cs is in the window\n",now(s.words));return 3;}
   if(gojam_due(now(s.words),&gnat)){printf("GOJAM at %lu cs (Q %05o)\n",now(s.words),(unsigned)s.words[5]);gojam_native(s.words,&io,&hw,&pending);substitute=0;}
   else if((k=key_due(now(s.words),&knat))>=0){if(k&MARK_KEY){io.channel16=(agc_word)(k&0377);pending|=RK_RUPT_BIT(RK_RUPT_KEY2);}
    else{hw.channel15=(agc_word)k;pending|=RK_RUPT_BIT(RK_RUPT_KEY1);}}
   else if((k=uplink_due(now(s.words),&unat))>=0){s.words[045]=(agc_word)k;pending|=RK_RUPT_BIT(RK_RUPT_UP);}
   if(rp_out&&!snap_path)rp_diff(now(s.words),0u,s.words,&hw,io.channel16,pending);
   if(rp_seam&&snap_path&&pending){fprintf(stderr,"FLIGHT_REPLAY: a key or interrupt request at the chain point %lu cs\n",now(s.words));return 4;}
   if(rp_seam&&snap_path){FILE *o=rp_out;rp_out=rp_seam;rp_t0=now(s.words);rp_diff(now(s.words),0u,s.words,&hw,io.channel16,pending);fclose(rp_seam);rp_seam=0;rp_out=o;rp_rows=0;rp_next=rp_every;}
   if(snap_path&&now(s.words)>=snap_cs&&(!pending||snap_any)){snapshot(snap_path,&s,&io,&hw);printf("snapshot at %lu cs\n",now(s.words));
    if(!rp_out)return 0;
    /* FLIGHT_REPLAY: on, on the C64 window's work clock. */
    snap_path=0;rp_t0=now(s.words);scaled=1;AgcFrozenClock=1;work=1;lag=~0u;slow=1.0;seed=rp_seed;jit_lo=0.8;jit_span=1.5625;consumed=0;wall=0.0;idling=0;}}
  {agc_word brupt=s.words[017];uint8_t got;
   if(scaled&&work){
    /* rk_rupts: a request that can be taken before each step due. */
    while(!(got=rk_rupt_take(&p,&s,io.superbank,io.interrupt_enabled,&pending))&&step_at(consumed)<=wall){
     rk_timer_phase(&s,&hw,&pending,(uint8_t)(consumed%3ul));if(consumed++%3ul==2ul){++taken;steal();}}
    if(got&&idling)wall=consumed?step_at(consumed-1ul):0.0;
    idling=0;}
   else{if(scaled){unsigned k;for(k=due();k;--k){rk_timer_tick(&s,&hw,&pending);steal();}}
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
  e=holder(s.words,io.superbank,&call);
  /* FLIGHT_REPLAY_TRACE: each routine's Z, FB, TIME1, work clock (step,
   * MCT since it), Q and table, as dap_pass_check.c DP_STEPS records the
   * C64's steps with DP_REPLAY. */
  if(rp_out&&!snap_path&&getenv("FLIGHT_REPLAY_TRACE"))printf("RT %05o %05o %03o %lu %.0f Q %05o %s\n",(unsigned)s.words[5],(unsigned)s.words[4],(unsigned)(s.words[025]&0377u),consumed%9ul,wall-(consumed?step_at(consumed-1ul):0.0),(unsigned)s.words[2],e?e->name:"(bridge)");
  if(e&&*e->module)module_load(e,s.words,io.superbank);
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
  if(e){flight_io io0=io;agc_hardware hw0=hw;long n;
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
    x=flight_io_enter(e->io,&io,&hw,&b);st=call(&p,&s,x);flight_io_leave(e->io,&io,&hw,&b);
    /* A pinned table that refuses the point (slot 3 or 4) leaves it to the
     * next holder, as rk_direct does on the C64 (a slot that runs no step). */
    if(st==EXEC_OK||e->slot<3u||e->slot>4u||!(next_e=holder_after(before.words,io0.superbank,&call,e)))break;
    s=before;io=io0;hw=hw0;e=next_e;names[t]=e->name;
    if(*e->module)module_load(e,s.words,io.superbank);}
   if(st==EXEC_OK){uint16_t pending0=pending;agc_hardware hw1=hw0;int interp;(void)point(before.words,io0.superbank,&interp);
    n=lockstep(&before,&io0,&hw0,&s,io.superbank,&hw,&pending,50000l,first,interp);
    /* Again with the scalers yaAGC read (scaler_latch). */
    if(n<0&&!scaled&&scaler_latch(&before,&io0,&hw1,&s,io.superbank,50000l)){
     s=before;io=io0;hw=hw1;pending=pending0;
     x=flight_io_enter(e->io,&io,&hw,&b);st=call(&p,&s,x);flight_io_leave(e->io,&io,&hw,&b);
     dl_clear();
     if(st==EXEC_OK)n=lockstep(&before,&io0,&hw0,&s,io.superbank,&hw,&pending,50000l,first,interp);}
    if(st==EXEC_OK){
     if(scaled){if(work&&!e->mct)idle_tick();else spend(work?e->mct:span_mct);}
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
   if((n=bridge(&s,&io,&hw,&pending,5000000l))<0){printf("bridge did not reach a registered point after 5000000 instructions\n");where(w,io.superbank,"from");where(s.words,io.superbank,"at");break;}
   dl_bridged(w);
   hole(w,io.superbank,(unsigned long)n,e?e->name:0);bridged+=(unsigned long)n;}}
 printf("%s: %lu native routines (%lu yaAGC instructions of time), %lu bridged yaAGC instructions; %lu samples: %lu equal, %lu differ\n",
  MISSION?"CM":"LM",steps,locked,bridged,samples,samples-diffs,diffs);
 if(scaled)printf("scaled time: K %.2f, divisor %u, lag %u, seed %lu: %lu CIA interrupts, %lu clock periods taken, %lu dropped (rk_dilation); AGC time %lu cs\n",
  slow,divisor,lag,seed,work?consumed/3ul:consumed,taken,dropped,now(s.words));   /* work: whole periods' steps */
 if(lost_brupt)printf("%lu interrupts at a RESUME point replaced BRUPT\n",lost_brupt);
 if(refs){const agc_word *r=ref[refs<sizeof ref/sizeof ref[0]?refs:0u];   /* scaled: no reference, refs may pass its end */
  printf("major mode (MODREG 01011) at %lu cs: native %02o, yaAGC %02o; FAILREG %05o/%05o\n",refs,(unsigned)s.words[01011],(unsigned)r[01011],(unsigned)s.words[0375],(unsigned)r[0375]);}
 printf("native routines yaAGC did not match (%u):\n",nmisses);
 for(i=0;i<nmisses;++i){unsigned k,d=0;printf("  %s %02o,%04o %s: %lu times;",misses[i].in?"interp":"basic ",misses[i].key/1024u,misses[i].key%1024u+02000u,misses[i].name,misses[i].count);
  if(!misses[i].arrived)printf(" yaAGC never reached the native continuation Z %05o\n",(unsigned)misses[i].word[5]);
  else{for(k=0;k<2048u;++k)if(!(k>=024u&&k<=031u)&&misses[i].word[k]!=misses[i].ref[k]&&d++<6u)printf(" %04o %05o/%05o",k,(unsigned)misses[i].word[k],(unsigned)misses[i].ref[k]);
   printf("%s (native/yaAGC)\n",d>6u?" ...":"");}}
 printf("unregistered or refused points (%u):\n",nholes);
 for(i=0;i<nholes;++i)printf("  %s %02o,%04o Z %05o FB %05o Q %05o: %lu arrivals, %lu instructions%s%s\n",holes[i].in?"interp":"basic ",
  holes[i].key/1024u,holes[i].key%1024u+02000u,(unsigned)holes[i].z,(unsigned)holes[i].fb,(unsigned)holes[i].q,holes[i].count,holes[i].instructions,
  holes[i].by?", refused by ":"",holes[i].by?holes[i].by:"");
 printf("module loads (%lu in %lu cs):\n",nloads,now(s.words));
 for(i=0;i<nload_points;++i)printf("  load %s slot %u %s %02o,%04o: %lu\n",loads[i].module,loads[i].slot,loads[i].in?"interp":"basic ",
  loads[i].key/1024u,loads[i].key%1024u+02000u,loads[i].count);
 dl_report();
 printf("last routines:\n");
 for(i=0;i<TRACE&&i<steps;++i){unsigned long t=(steps-1u-i)%TRACE;printf("  Z %05o FB %05o %s\n",trace[t][0],trace[t][1],names[t]);}
 return 0;}
