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
 * (TIME1-TIME6 and the scalers), the rest dropped (rk_rupts); the IMU CDU
 * drive, timed by yaAGC's cycle counter, stops with it. Bridged code
 * gets the same periods between its instructions. Every routine must then
 * match yaAGC run from the same state with the clock frozen, the clock
 * counters included, so the run is one yaAGC run under the same tick
 * schedule. No free-running reference is run.
 * FLIGHT_MCT=PATH: the machine cycles yaAGC took for each matched routine,
 * by table and entry (mct_write), for the work clock's weights.
 * FLIGHT_SNAPSHOT=CS,PATH: at the first native sample point at or after
 * centisecond CS (after that point's pokes), write the native state (the
 * erasable image, the superbank, the interrupt enable, agc_hardware and the
 * flight_io channels) as a C header for the C64 DAP pass check
 * (platform/c64/dap_pass_check.c), and stop.
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
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed) extern const agc_routine_table t;
#include FLIGHT_HOST_TABLES
#undef FLIGHT_TABLE
typedef struct { const agc_routine_table *t; const char *name; uint8_t interp; enum flight_io_kind io; const char *module; uint8_t slot;
                 const agc_site_operand *sites; uint16_t nsites; const char *also; uint8_t fixed; } entry;
#define FLIGHT_TABLE(t,i,k,mod,slot,sites,n,also,fixed) {&t,#t,i,k,mod,slot,sites,n,also,fixed},
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
static double slow=1.0,wall;
static unsigned divisor=1u,lag=2u;
static unsigned long seed,consumed,taken,dropped,span_mct;
static double jitter(void){if(!seed)return 1.0;
 seed^=seed<<13;seed^=seed>>7;seed^=seed<<17;return 0.25*pow(16.0,(double)(seed%10000ul)/9999.0);}
static void spend(unsigned long mct){wall+=(double)mct*slow*jitter();}
/* rk_rupts: the CIA1 interrupts since the last boundary, at most lag of
 * them as clock periods, the rest dropped. */
static unsigned due(void){unsigned long now=(unsigned long)(wall/(85333.0/100.0*divisor));unsigned n=0;
 for(;consumed<now;++consumed){if(n<lag){++n;++taken;}else ++dropped;}
 return n;}
/* Input channels 30 and 31 at start (FLIGHT_CH30, FLIGHT_CH31 below). */
static unsigned long chan30=037777u,chan31=077777u;
/* yaAGC's GOJAM state (agc_engine_init) without its first DOWNRUPT. */
static void cold(void){unsigned i;m=loaded;memset(m.Erasable,0,sizeof m.Erasable);
 for(i=0;i<NUM_CHANNELS;++i)m.InputChannel[i]=0;
 m.InputChannel[030]=(int16_t)chan30;m.InputChannel[031]=(int16_t)chan31;m.InputChannel[032]=m.InputChannel[033]=077777;
 m.Erasable[0][5]=04000;m.AllowInterrupt=1;}

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
static const entry *holder(const agc_word *w,agc_word sb,agc_routine *call){int in;unsigned i;uint16_t key=point(w,sb,&in);
 for(i=0;i<TABLES;++i)if(tables[i].interp==(uint8_t)in&&(*call=find(tables[i].t,key)))return &tables[i];
 return 0;}
static void where(const agc_word *w,agc_word sb,const char *what){int in;uint16_t k=point(w,sb,&in);
 printf("%s: %s Z %05o FB %05o EB %05o superbank %03o (key %u = %02o,%04o) Q %05o LOC %05o BANKSET %05o\n",what,
  in?"interpretive":"basic",(unsigned)w[5],(unsigned)w[4],(unsigned)w[3],(unsigned)sb,(unsigned)k,k/1024u,k%1024u+02000u,
  (unsigned)w[2],(unsigned)w[0164],(unsigned)w[0165]);}
static unsigned long now(const agc_word *w){return ((unsigned long)(w[024]&037777u)<<14)|(w[025]&037777u);}
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
static agc_word keycode(char c){return c=='V'?021u:c=='N'?037u:c=='E'?034u:c=='0'?020u:c=='+'?032u:c=='-'?033u:
 c=='C'?036u:c=='P'?022u:c=='K'?031u:c=='R'?022u:(agc_word)(c-'0');}
static int key_due(unsigned long t,unsigned *pressed){if(!keys[*pressed]||t<KEY_AT+*pressed*KEY_GAP)return -1;return (int)keycode(keys[(*pressed)++]);}

/* FLIGHT_CH30, FLIGHT_CH31 (octal): the input channels at start, on both
 * sides (e.g. FLIGHT_CH30=036377 FLIGHT_CH31=057777: IMU operate, PGNCS
 * control, AUTO, so DAPIDLER starts the DAP). FLIGHT_POKE=CS:ADDR=VALUE,...
 * (octal address and value): erasable words set at the first sample point
 * at or after CS centiseconds, on both sides (e.g. 5000:035=02000 moves
 * CDUY to drive the DAP's jets). */
enum { POKES=8 };
static struct { unsigned long cs; unsigned addr,value; } pokes[POKES];
static unsigned npokes;
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
 F(channel30)F(channel33)F(channel31)F(channel3)F(channel4)
#undef F
#define F(x) fprintf(f,"(h)." #x "=%uu;",(unsigned)io->x);
 fprintf(f,"\n#define SNAP_FLIGHT(h) ");
 F(superbank)F(interrupt_enabled)F(channel16)F(channel34)F(channel35)F(trap31a)F(trap31b)F(trap32)F(executive_alarm)
 F(waitlist_alarm)F(warning)
#undef F
 fprintf(f,"\n");fclose(f);}
static void pokes_parse(const char *t){while(t&&*t&&npokes<POKES){unsigned long cs;unsigned a,v;int n=0;
  if(sscanf(t,"%lu:%o=%o%n",&cs,&a,&v,&n)<3||!n||a>=2048u)break;
  pokes[npokes].cs=cs;pokes[npokes].addr=a;pokes[npokes++].value=v&077777u;t+=n;if(*t==',')++t;}}
/* The pokes due at time t, once each side (done counts them). */
static void poke_due(unsigned long t,unsigned *done,agc_word *words){
 while(*done<npokes&&t>=pokes[*done].cs){unsigned a=pokes[*done].addr,v=pokes[*done].value;
  ++*done;if(words)words[a]=(agc_word)v;else *cell(a)=(int16_t)v;}}

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
/* Native state into yaAGC and back. */
static const unsigned short ch[]={3,4,5,6,010,011,012,013,014,015,030,031,032,033};
static agc_word *hwc(agc_hardware *hw,unsigned i){agc_word *c[]={&hw->channel3,&hw->channel4,&hw->channel5,&hw->channel6,&hw->channel10,
 &hw->channel11,&hw->channel12,&hw->channel13,&hw->channel14,&hw->channel15,&hw->channel30,&hw->channel31,&hw->channel32,&hw->channel33};return c[i];}
#define CHANNELS (sizeof ch/sizeof ch[0])
static void load(const apollo_overlay_state *s,const flight_io *io,agc_hardware *hw){unsigned i;
 for(i=0;i<2048u;++i)*cell(i)=(int16_t)s->words[i];
 for(i=0;i<CHANNELS;++i)m.InputChannel[ch[i]]=(int16_t)*hwc(hw,i);
 m.InputChannel[016]=(int16_t)io->channel16;m.InputChannel[034]=(int16_t)io->channel34;m.InputChannel[035]=(int16_t)io->channel35;
 m.InputChannel[7]=m.OutputChannel7=(int16_t)io->superbank;m.AllowInterrupt=io->interrupt_enabled!=0u;m.InIsr=hw->in_interrupt!=0u;
 m.Trap31A=io->trap31a;m.Trap31B=io->trap31b;m.Trap32=io->trap32;
 memset(m.InterruptRequests,0,sizeof m.InterruptRequests);m.DownruptTimeValid=0;
 m.SubstituteInstruction=(unsigned)substitute;substitute=0;m.PendFlag=0;m.PendDelay=0;m.ExtraDelay=0;m.IndexValue=0;m.ExtraCode=0;}
static agc_word mword(unsigned i){return (agc_word)(*cell(i)&(i<2u?0177777u:077777u));}
static void store(apollo_overlay_state *s,flight_io *io,agc_hardware *hw){unsigned i;
 for(i=0;i<2048u;++i)s->words[i]=mword(i);
 for(i=0;i<CHANNELS;++i)*hwc(hw,i)=(agc_word)(m.InputChannel[ch[i]]&077777);
 io->channel16=(agc_word)m.InputChannel[016];io->channel34=(agc_word)m.InputChannel[034];io->channel35=(agc_word)m.InputChannel[035];
 io->superbank=(agc_word)(m.OutputChannel7&0160);io->interrupt_enabled=m.AllowInterrupt;hw->in_interrupt=m.InIsr;
 io->trap31a=m.Trap31A;io->trap31b=m.Trap31B;io->trap32=m.Trap32;}
/* The words point() and idle() read, from yaAGC. */
static const agc_word *view(void){static agc_word v[2048];static const unsigned short at[]={3,4,5,AGC_EDOP,NEWJOB,0164,0165};unsigned i;
 for(i=0;i<sizeof at/sizeof at[0];++i)v[at[i]]=mword(at[i]);
 return v;}
/* One clock period (rk_timer_tick) on yaAGC's state, for bridged code. */
static void ref_tick(void){static apollo_overlay_state t;agc_hardware h;uint16_t p=0;unsigned i;
 for(i=024u;i<=031u;++i)t.words[i]=mword(i);
 memset(&h,0,sizeof h);h.channel13=(agc_word)(m.InputChannel[013]&077777);
 h.channel3=(agc_word)(m.InputChannel[3]&077777);h.channel4=(agc_word)(m.InputChannel[4]&077777);
 rk_timer_tick(&t,&h,&p);
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
   if(quiet()){for(k=due();k;--k)ref_tick();}
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
static void adopt(apollo_overlay_state *after,agc_hardware *hw){unsigned i;
 if(scaled)return;   /* the clock is frozen: the routine's own counters stand */
 for(i=024u;i<=031u;++i)after->words[i]=mword(i);
 hw->channel3=(agc_word)(m.InputChannel[3]&077777);hw->channel4=(agc_word)(m.InputChannel[4]&077777);
 if(!(m.InputChannel[013]&040000))hw->channel13&=037777u;}
static long lockstep(const apollo_overlay_state *before,const flight_io *io0,agc_hardware *hw0,apollo_overlay_state *after,
                     agc_word superbank,agc_hardware *hw,uint16_t *pending,long limit,agc_word *first){
 static apollo_overlay_state timed;static agc_hardware timed_hw;
 long n;int in,tin;uint16_t target=point(after->words,superbank,&tin),got=0,got_first=0;unsigned i;uint64_t c0;unsigned long first_mct=0;
 first[5]=077777u;load(before,io0,hw0);c0=m.CycleCounter;
 for(n=0;n<limit;++n){
  /* After RESUME, yaAGC is at ZRUPT with BRUPT substituted for the next
   * instruction: the native continuation is that point. */
  if(n&&(quiet()||(m.SubstituteInstruction&&!m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode))
     &&point(view(),(agc_word)m.OutputChannel7,&in)==target&&in==tin){
   for(i=0;i<2048u;++i)if((scaled||!(i>=024u&&i<=031u))&&i!=5u&&mword(i)!=after->words[i])break;
   if(i==2048u){adopt(after,hw);*pending|=got;substitute=m.SubstituteInstruction;span_mct=(unsigned long)(m.CycleCounter-c0);return n;}
   if(first[5]==077777u){for(i=0;i<2048u;++i)first[i]=mword(i);first_mct=(unsigned long)(m.CycleCounter-c0);timed=*after;timed_hw=*hw;adopt(&timed,&timed_hw);got_first=got;}}
  if(step(&got))return -1;}
 /* A miss: the routine took as long as yaAGC to its first arrival there
  * (none when it never arrived). */
 span_mct=first[5]!=077777u?first_mct:0ul;
 if(first[5]!=077777u&&!scaled){for(i=024u;i<=031u;++i)after->words[i]=timed.words[i];
  hw->channel3=timed_hw.channel3;hw->channel4=timed_hw.channel4;hw->channel13=timed_hw.channel13;*pending|=got_first;}
 return -1;}

/* Comanche's channels 5, 6 and 30-33 live in cm_channels (include/
 * cm_native_image.h) until agc_hardware carries them: a CM routine reads
 * them there and writes 5 and 6 there, so they follow hw around each call. */
static void channels_in(const agc_hardware *hw){
#if RK_LM
 (void)hw;
#else
 cm_channels.channel05=hw->channel5;cm_channels.channel06=hw->channel6;cm_channels.channel30=hw->channel30;
 cm_channels.channel31=hw->channel31;cm_channels.channel32=hw->channel32;cm_channels.channel33=hw->channel33;
#endif
}
static void channels_out(agc_hardware *hw){
#if RK_LM
 (void)hw;
#else
 hw->channel5=cm_channels.channel05;hw->channel6=cm_channels.channel06;
#endif
}

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
 for(i=0;i<nmisses;++i)if(misses[i].name==name&&misses[i].key==key&&misses[i].in==(uint8_t)in){++misses[i].count;return;}
 if(nmisses<MISSES){misses[nmisses].name=name;misses[nmisses].key=key;misses[nmisses].in=(uint8_t)in;misses[nmisses].count=1;
  memcpy(misses[nmisses].word,after,sizeof misses[nmisses].word);memcpy(misses[nmisses].ref,first,sizeof misses[nmisses].ref);
  misses[nmisses].arrived=first[5]!=077777u;++nmisses;}}

/* The flight resident's fixed_data hook (platform/c64/resident_flight.c
 * rk_fixed_word): the rope word at every point where it stops between
 * routines, the keys of every table it can resolve
 * (tools/resident_boundary_words.py with the store's resolver records). */
static agc_site_operand boundary[8192];
static uint16_t nboundary;
static int by_key(const void *a,const void *b){return (int)((const agc_site_operand *)a)->physical-(int)((const agc_site_operand *)b)->physical;}
/* Interrupts take the word at Z: at a basic routine Z is its key, at an
 * interpretive one Z is the interpreter fetch point or DANZIG (the key is
 * LOC's), so those two words are boundary words too. */
static void boundary_build(void){unsigned i,j;
 for(i=0;i<=TABLES;++i)for(j=0;j<(i<TABLES?tables[i].t->count:2u);++j){uint16_t k=i<TABLES?tables[i].t->entries[j].physical:j?DANZIG:FETCH;agc_word w=0;
  if(k<APOLLO_ROPE_WORDS&&nboundary<sizeof boundary/sizeof boundary[0]&&apollo_rope_read(MISSION,k,&w)){boundary[nboundary].physical=k;boundary[nboundary++].word=w;}}
 qsort(boundary,nboundary,sizeof boundary[0],by_key);
 for(i=j=0;i<nboundary;++i)if(!j||boundary[j-1].physical!=boundary[i].physical)boundary[j++]=boundary[i];
 nboundary=(uint16_t)j;}
static bool boundary_word(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;return agc_site_lookup(boundary,nboundary,physical,word);}
static bool rope_data(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;return apollo_rope_read(MISSION,physical,word);}

/* Module loads: the C64 keeps one module per slot (job, task); a routine of
 * another module in its slot loads that module (a swap). Counted at each
 * native call, refused or not, by module and by the point that entered it. */
#define LOADS 512u
static const char *in_slot[2];
static unsigned long nloads;
static struct { const char *module; uint16_t key; uint8_t in, slot; unsigned long count; } loads[LOADS];
static unsigned nload_points;
static void module_load(const entry *e,const agc_word *w,agc_word sb){int in;uint16_t key;unsigned i;
 if(in_slot[e->slot]==e->module)return;
 /* A table packed into the loaded module too runs there (CO_LOCATE). */
 if(in_slot[e->slot]&&*e->also){const char *a=strstr(e->also,in_slot[e->slot]);size_t n=strlen(in_slot[e->slot]);
  if(a&&(a==e->also||a[-1]==' ')&&(a[n]==0||a[n]==' '))return;}
 in_slot[e->slot]=e->module;++nloads;key=point(w,sb,&in);
 /* Scaled time: a swap is about 20 ms of C64 time (docs/FLIGHT-STORE.md:
  * 7,459 cycles plus 1.1 a byte), 1,707 AGC machine cycles of wall. */
 if(scaled)wall+=1707.0*jitter();
 for(i=0;i<nload_points;++i)if(loads[i].module==e->module&&loads[i].key==key){++loads[i].count;return;}
 if(nload_points<LOADS){loads[nload_points].module=e->module;loads[nload_points].key=key;loads[nload_points].in=(uint8_t)in;
  loads[nload_points].slot=e->slot;loads[nload_points++].count=1;}}
/* FLIGHT_MCT=PATH: each matched routine's yaAGC machine cycles by table and
 * entry key (count, sum, sum of squares, least, most), the calibration of
 * the flight resident's work clock (tools/flight_mct.py). */
enum { MCTS=8192 };
static struct { const char *table; uint16_t key; uint8_t in; unsigned long n,sum,least,most; double sq; } mcts[MCTS];
static unsigned nmcts;
static void mct_note(const char *table,const agc_word *w,agc_word sb,unsigned long mct){int in;uint16_t key=point(w,sb,&in);unsigned i;
 for(i=0;i<nmcts;++i)if(mcts[i].table==table&&mcts[i].key==key&&mcts[i].in==(uint8_t)in)break;
 if(i==nmcts){if(nmcts==MCTS)return;mcts[i].table=table;mcts[i].key=key;mcts[i].in=(uint8_t)in;mcts[i].least=mct;++nmcts;}
 ++mcts[i].n;mcts[i].sum+=mct;mcts[i].sq+=(double)mct*(double)mct;
 if(mct<mcts[i].least)mcts[i].least=mct;
 if(mct>mcts[i].most)mcts[i].most=mct;}
static void mct_write(const char *path){FILE *f=fopen(path,"w");unsigned i;
 if(!f){perror(path);return;}
 for(i=0;i<nmcts;++i)fprintf(f,"%s %u %u %lu %lu %.0f %lu %lu\n",mcts[i].table,(unsigned)mcts[i].key,(unsigned)mcts[i].in,
  mcts[i].n,mcts[i].sum,mcts[i].sq,mcts[i].least,mcts[i].most);
 fclose(f);}
static unsigned long snap_cs;
static const char *snap_path;
int main(int argc,char **argv){static apollo_overlay_state s,undo,before;static agc_word ref[8192][2048],first[2048];
 flight_io io;agc_hardware hw;agc_native_program p;uint16_t pending=0;
 unsigned long cs=argc>2?strtoul(argv[2],0,0):100ul,steps,next,refs,limit,bridged=0,locked=0;
 unsigned i,j,kref=0,knat=0,pref=0,pnat=0;int latency=getenv("FLIGHT_NATIVE_LATENCY")!=0;unsigned long diffs=0,samples=0,lost_brupt=0;
 enum { TRACE=24 };uint16_t trace[TRACE][3];const char *names[TRACE];
 if(argc<2||!apollo_rope_load(MISSION,argv[1])||agc_load_binfile(&loaded,argv[1])){fprintf(stderr,"usage: flight-host-run ROPE.bin [CS [KEYS]]\n");return 2;}
 if(argc>3)keys=argv[3];
 if(getenv("FLIGHT_CH30"))chan30=strtoul(getenv("FLIGHT_CH30"),0,8)&077777u;
 if(getenv("FLIGHT_CH31"))chan31=strtoul(getenv("FLIGHT_CH31"),0,8)&077777u;
 pokes_parse(getenv("FLIGHT_POKE"));
 if(getenv("FLIGHT_SNAPSHOT")){static char path_buf[512];
  if(sscanf(getenv("FLIGHT_SNAPSHOT"),"%lu,%511s",&snap_cs,path_buf)!=2){fprintf(stderr,"FLIGHT_SNAPSHOT=CS,PATH\n");return 2;}
  snap_path=path_buf;}
 if(getenv("FLIGHT_PATH"))sscanf(getenv("FLIGHT_PATH"),"%lu,%lu",&path_from,&path_to);
 if(cs>=sizeof ref/sizeof ref[0])cs=sizeof ref/sizeof ref[0]-1u;
 if(getenv("FLIGHT_SCALED")){scaled=1;AgcFrozenClock=1;
  if(sscanf(getenv("FLIGHT_SCALED"),"%lf,%u,%u,%lu",&slow,&divisor,&lag,&seed)<3||slow<=0.0||!divisor||!lag){fprintf(stderr,"FLIGHT_SCALED=K,D,LAG[,SEED]\n");return 2;}}
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
    if((k=key_due(now(w),&kref))>=0){m.InputChannel[015]=(int16_t)k;
     if(m.AllowInterrupt){*cell(015)=(int16_t)(ADVAN+1u);*cell(017)=(int16_t)rope_word(ADVAN,(agc_word)(*cell(4)&077777),(agc_word)m.OutputChannel7);*cell(5)=(int16_t)(04000u+4u*RK_RUPT_KEY1);m.InIsr=1;}
     else m.InterruptRequests[RK_RUPT_KEY1]=1;}}}
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
  if(agc_engine(&m))return 2;}
 refs=scaled?cs:next-1u;
 if(refs<cs)printf("reference: only %lu of %lu centiseconds reached an idle boundary\n",refs,cs);
 /* Native. */
 cold();
 memset(&s,0,sizeof s);memset(&io,0,sizeof io);memset(&hw,0,sizeof hw);memset(&p,0,sizeof p);
 io.undo=&undo;s.words[5]=04000u;io.interrupt_enabled=1;
 hw.channel30=(agc_word)chan30;hw.channel31=(agc_word)chan31;hw.channel32=hw.channel33=077777u;
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
  if(!pending&&(scaled||(hw.channel4&020u))&&at_advan(s.words,io.superbank)&&idle(s.words,io.superbank,hw.in_interrupt)){int k;
   while(next<=refs&&now(s.words)>=next){if(scaled){++next;continue;}++samples;
    if((j=differs(s.words,ref[next]))!=0u&&++diffs<=(getenv("FLIGHT_SHOW")?strtoul(getenv("FLIGHT_SHOW"),0,0):20ul)){
     printf("%lu cs (native routine %lu):",next,steps);
     for(i=0,j=0;i<2048u;++i)if(compared(i)&&s.words[i]!=ref[next][i]&&j++<8u)printf(" %04o %05o/%05o",i,(unsigned)s.words[i],(unsigned)ref[next][i]);
     printf("%s (%u words, native/yaAGC)\n",j>8u?" ...":"",j);}
    ++next;}
   /* A key is pressed at the first sample point at or after its time, on
    * both sides. */
   poke_due(now(s.words),&pnat,s.words);
   if((k=key_due(now(s.words),&knat))>=0){hw.channel15=(agc_word)k;pending|=RK_RUPT_BIT(RK_RUPT_KEY1);}
   if(snap_path&&now(s.words)>=snap_cs&&!pending){snapshot(snap_path,&s,&io,&hw);printf("snapshot at %lu cs\n",now(s.words));return 0;}}
  if(scaled){unsigned k;for(k=due();k;--k)rk_timer_tick(&s,&hw,&pending);}
  {agc_word brupt=s.words[017];
   if(rk_rupt_take(&p,&s,io.superbank,io.interrupt_enabled,&pending)){
    /* At a RESUME point the instruction to run is BRUPT, not the word at
     * Z; rk_rupt_take saves the word at Z. */
    if(substitute&&brupt!=s.words[017]&&++lost_brupt==1ul)printf("interrupt at a RESUME point replaced BRUPT %05o with %05o at %lu cs\n",(unsigned)brupt,(unsigned)s.words[017],now(s.words));
    substitute=0;}}
  e=holder(s.words,io.superbank,&call);
  if(e&&*e->module)module_load(e,s.words,io.superbank);
  if(path_to)path("NAT",s.words,io.superbank,hw.channel4,hw.in_interrupt,e?e->name:"(bridge)");
  if(e){flight_io io0=io;agc_hardware hw0=hw;long n;
   unsigned long t=steps%TRACE;trace[t][0]=(uint16_t)s.words[5];trace[t][1]=(uint16_t)s.words[4];names[t]=e->name;
   before=s;p.operands=e->sites;p.operand_count=e->nsites;
   /* The module's fixed_data (flight_store_build.py FIXED_DATA): the
    * resident's boundary words, self-check's rope hook, or every rope word. */
#if RK_LM
   p.fixed_data=e->fixed==1u?lm_self_check_rope_data:e->fixed==2u?rope_data:boundary_word;
#else
   p.fixed_data=e->fixed==1u?cm_self_check_rope_data:e->fixed==2u?rope_data:boundary_word;
#endif
x=flight_io_enter(e->io,&io,&hw,&b);channels_in(&hw);st=call(&p,&s,x);channels_out(&hw);flight_io_leave(e->io,&io,&hw,&b);
   if(st==EXEC_OK){uint16_t pending0=pending;agc_hardware hw1=hw0;
    n=lockstep(&before,&io0,&hw0,&s,io.superbank,&hw,&pending,50000l,first);
    /* Again with the scalers yaAGC read (scaler_latch). */
    if(n<0&&!scaled&&scaler_latch(&before,&io0,&hw1,&s,io.superbank,50000l)){
     s=before;io=io0;hw=hw1;pending=pending0;
     x=flight_io_enter(e->io,&io,&hw,&b);channels_in(&hw);st=call(&p,&s,x);channels_out(&hw);flight_io_leave(e->io,&io,&hw,&b);
     if(st==EXEC_OK)n=lockstep(&before,&io0,&hw0,&s,io.superbank,&hw,&pending,50000l,first);}
    if(st==EXEC_OK){
     if(scaled)spend(span_mct);
     if(n>=0){locked+=(unsigned long)n;mct_note(e->name,before.words,io0.superbank,span_mct);}
     else miss(e->name,before.words,io0.superbank,s.words,first);
     continue;}}
   /* A refusal: undo whatever the routine did and bridge the point. */
   s=before;io=io0;hw=hw0;}
  {agc_word w[2048];long n;memcpy(w,s.words,sizeof w);
   if((n=bridge(&s,&io,&hw,&pending,5000000l))<0){printf("bridge did not reach a registered point after 5000000 instructions\n");where(w,io.superbank,"from");where(s.words,io.superbank,"at");break;}
   hole(w,io.superbank,(unsigned long)n,e?e->name:0);bridged+=(unsigned long)n;}}
 if(getenv("FLIGHT_MCT"))mct_write(getenv("FLIGHT_MCT"));
 printf("%s: %lu native routines (%lu yaAGC instructions of time), %lu bridged yaAGC instructions; %lu samples: %lu equal, %lu differ\n",
  MISSION?"CM":"LM",steps,locked,bridged,samples,samples-diffs,diffs);
 if(scaled)printf("scaled time: K %.2f, divisor %u, lag %u, seed %lu: %lu CIA interrupts, %lu clock periods taken, %lu dropped (rk_dilation); AGC time %lu cs\n",
  slow,divisor,lag,seed,consumed,taken,dropped,now(s.words));
 if(lost_brupt)printf("%lu interrupts at a RESUME point replaced BRUPT\n",lost_brupt);
 if(refs)printf("major mode (MODREG 01011) at %lu cs: native %02o, yaAGC %02o; FAILREG %05o/%05o\n",refs,(unsigned)s.words[01011],(unsigned)ref[refs][01011],(unsigned)s.words[0375],(unsigned)ref[refs][0375]);
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
 printf("last routines:\n");
 for(i=0;i<TRACE&&i<steps;++i){unsigned long t=(steps-1u-i)%TRACE;printf("  Z %05o FB %05o %s\n",trace[t][0],trace[t][1],names[t]);}
 return 0;}
