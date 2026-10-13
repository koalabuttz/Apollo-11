/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only yaAGC adapter for tests/resident_rupt_cases.c. NEVER part of the
 * native port; no AGC instruction implementation is copied here.
 *   resident-rupt-reference lm|cm ROPE.bin [STOPS]
 * prints one line per entry case ("E n digest") and per timer case
 * ("T n digest"): the interrupt agc_engine takes before the instruction at
 * Z (the vector, and when it takes one Z, ZRUPT, BRUPT, the requests left
 * and InIsr; otherwise the instruction runs and only the decision counts),
 * the T3RUPT cases run to RESUME ("R n digest": all 2048 words, channel 7,
 * interrupts enabled, InIsr; or not finished),
 * and one 10 ms scaler period of TIME1-TIME6 through yaAGC's own
 * CounterPINC and CounterDINC, in agc_engine's order and with its overflow
 * requests and T6 stop. RR_SHOW=n prints T3RUPT case n's final state to
 * stderr. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "resident_rupt_cases.h"
#include "resident_vector_cases.h"
void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){if(c==7)s->InputChannel[7]=s->OutputChannel7=v&0160;}
void ChannelRoutine(agc_t *s){(void)s;}
void RequestRadarData(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int data){(void)s;(void)data;}
void UnblockSocket(int n){(void)n;}
int CounterPINC(int16_t *counter);
int CounterDINC(agc_t *state,int counter,int16_t *value);
static agc_t machine,baseline;
static apollo_overlay_state image;
static int16_t *memory(unsigned a){return &machine.Erasable[a/0400u][a%0400u];}
static unsigned get(unsigned a){return (unsigned)*memory(a)&077777u;}
static agc_word rope(unsigned mission,uint16_t physical){(void)mission;
 return physical<36u*1024u?(agc_word)((unsigned)baseline.Fixed[physical/1024u][physical%1024u]&077777u):0u;}
/* The point the native trampoline compares (agc_transfer_point) for the
 * machine's Z, FB, EB and channel 7. */
static uint16_t point(void){unsigned z=get(5)&07777u;
 return z<02000u?agc_erasable_point((agc_word)z,(agc_word)get(3)):agc_fixed_physical((agc_word)z,(agc_word)get(4),(agc_word)machine.OutputChannel7);}
int main(int argc,char **argv){unsigned mission,i;uint32_t n,h;const char *show=getenv("RR_SHOW");FILE *stops=0;
 if((argc!=3&&argc!=4)||(strcmp(argv[1],"lm")&&strcmp(argv[1],"cm")))return 2;
 if(argc==4&&!(stops=fopen(argv[3],"r")))return 2;
 mission=argv[1][0]=='c';
 if(agc_load_binfile(&baseline,argv[2]))return 2;
 InhibitAlarms=1;
 for(n=0;n<RR_ENTRY_CASES;++n){rr_case c;
  c.s=&image;rr_entry_input(mission,(uint16_t)n,rope,&c);machine=baseline;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)c.s->words[i];
  machine.InputChannel[7]=machine.OutputChannel7=c.superbank;
  machine.AllowInterrupt=c.interrupt_enabled;machine.InIsr=c.in_interrupt;
  for(i=0;i<=NUM_INTERRUPT_TYPES;++i)machine.InterruptRequests[i]=(int8_t)(i&&(c.pending>>i&1u));
  machine.ExtraCode=0;machine.IndexValue=0;machine.SubstituteInstruction=0;machine.PendFlag=0;machine.PendDelay=0;
  machine.ExtraDelay=0;machine.ScalerCounter=0;machine.DownruptTimeValid=0;machine.CycleCounter=1;
  if(agc_engine(&machine))return 2;
  h=rr_mix(2166136261u,(uint16_t)machine.InterruptRequests[0]);
  /* Not taken, the instruction at Z runs: only the decision is compared. */
  if(machine.InterruptRequests[0]){h=rr_mix(h,(uint16_t)get(5));h=rr_mix(h,(uint16_t)get(015));h=rr_mix(h,(uint16_t)get(017));
   for(i=1;i<=NUM_INTERRUPT_TYPES;++i)h=rr_mix(h,(uint16_t)machine.InterruptRequests[i]);
   h=rr_mix(h,(uint16_t)machine.InIsr);}
  printf("E %u %08x\n",(unsigned)n,(unsigned)h);}
 for(n=0;n<RR_T3_CASES;++n){rr_case c;unsigned cycles;
  c.s=&image;rr_t3_input(mission,(uint16_t)n,&c);machine=baseline;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)c.s->words[i];
  machine.InputChannel[7]=machine.OutputChannel7=c.superbank;machine.AllowInterrupt=1;machine.InIsr=0;
  for(i=0;i<=NUM_INTERRUPT_TYPES;++i)machine.InterruptRequests[i]=(int8_t)(i==3u);
  machine.DownruptTimeValid=0;machine.CycleCounter=1;
  for(cycles=0;cycles<50000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&machine.SubstituteInstruction)break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  h=rr_mix(2166136261u,(uint16_t)(cycles<50000u));
  if(cycles<50000u){for(i=0;i<2048u;++i)h=rr_mix(h,(uint16_t)((unsigned)*memory(i)&(i<3u?0177777u:077777u)));
   h=rr_mix(h,(uint16_t)machine.OutputChannel7);h=rr_mix(h,(uint16_t)machine.AllowInterrupt);h=rr_mix(h,(uint16_t)machine.InIsr);}
  if(show&&(unsigned)atoi(show)==n){fprintf(stderr,"yaAGC T3RUPT %u: %s cycles %u ch7 %o inten %d isr %d\n",(unsigned)n,cycles<50000u?"RESUME":"no RESUME",cycles,
    (unsigned)machine.OutputChannel7,machine.AllowInterrupt,machine.InIsr);
   for(i=0;i<2048u;++i)fprintf(stderr,"%04o %06o\n",i,(unsigned)*memory(i)&(i<3u?0177777u:077777u));}
  printf("R %u %08x\n",(unsigned)n,(unsigned)h);}
 /* The other vectors, run until RESUME substitutes BRUPT (a native run
  * that resumed) or Z first reaches the point where the native run
  * stopped (stops file from tools/resident_vector_stops.c). */
 for(n=0;stops&&n<RR_VECTOR_CASES;++n){static rr_vcase c;unsigned nn,kind,p,cycles=0;
  if(fscanf(stops,"%u %u %u",&nn,&kind,&p)!=3||nn!=n)return 2;
  c.s=&image;rr_vector_input(mission,(uint16_t)n,&c);
  if(kind<=RR_V_STOPPED){agc_word sb=mission?c.cm.superbank:c.lm.superbank;
   machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)c.s->words[i];
   machine.InputChannel[7]=machine.OutputChannel7=sb;machine.AllowInterrupt=1;machine.InIsr=0;
   machine.InputChannel[5]=(int16_t)c.hw.channel5;machine.InputChannel[6]=(int16_t)c.hw.channel6;
   machine.InputChannel[010]=(int16_t)c.hw.channel10;machine.InputChannel[011]=(int16_t)c.hw.channel11;
   machine.InputChannel[013]=(int16_t)c.hw.channel13;machine.InputChannel[015]=(int16_t)c.hw.channel15;
   machine.InputChannel[032]=(int16_t)c.hw.channel32;
   for(i=0;i<=NUM_INTERRUPT_TYPES;++i)machine.InterruptRequests[i]=(int8_t)(i==rr_vector((uint16_t)n));
   machine.DownruptTimeValid=0;machine.CycleCounter=1;
   for(cycles=0;cycles<50000u;++cycles){
    if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&
       (kind==RR_V_RESUMED?machine.SubstituteInstruction!=0:point()==p))break;
    machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==50000u)kind=5u;   /* the original never got there */
   for(i=0;i<2048u;++i)c.s->words[i]=(agc_word)((unsigned)*memory(i)&(i<3u?0177777u:077777u));
   c.lm.superbank=c.cm.superbank=(agc_word)machine.OutputChannel7;
   c.lm.interrupt_enabled=c.cm.interrupt_enabled=(agc_word)machine.AllowInterrupt;c.hw.in_interrupt=(uint16_t)machine.InIsr;
   c.hw.channel5=(agc_word)machine.InputChannel[5];c.hw.channel6=(agc_word)machine.InputChannel[6];
   c.hw.channel10=(agc_word)machine.InputChannel[010];c.hw.channel11=(agc_word)machine.InputChannel[011];
   c.hw.channel13=(agc_word)machine.InputChannel[013];}
  if(getenv("RR_SHOW_V")&&(unsigned)atoi(getenv("RR_SHOW_V"))==n){fprintf(stderr,"yaAGC vector %u: kind %u cycles %u ch7 %o inten %o isr %u ch5 %o ch6 %o ch10 %o ch11 %o ch13 %o\n",(unsigned)n,kind,cycles,
    (unsigned)c.cm.superbank,(unsigned)c.cm.interrupt_enabled,(unsigned)c.hw.in_interrupt,(unsigned)c.hw.channel5,(unsigned)c.hw.channel6,
    (unsigned)c.hw.channel10,(unsigned)c.hw.channel11,(unsigned)c.hw.channel13);
   for(i=0;i<2048u;++i)fprintf(stderr,"%04o %06o\n",i,(unsigned)c.s->words[i]);}
  printf("V %u %08x\n",(unsigned)n,(unsigned)rr_vector_digest(mission,&c,(uint8_t)kind));}
 for(n=0;n<RR_TIMER_CASES;++n){agc_word t[6],ch13;int16_t r[6];uint16_t req=0;
  rr_timer_input(n,t,&ch13);for(i=0;i<6u;++i)r[i]=(int16_t)t[i];
  machine=baseline;
  /* agc_engine's scaler order over SCALER1 0-037: TIME5 at 000, TIME4 at
   * 010, TIME1/TIME2 and TIME3 at 020, TIME6 on every odd count. */
  for(i=0;i<32u;++i){
   if(i==020u){if(CounterPINC(&r[1]))CounterPINC(&r[0]);if(CounterPINC(&r[2]))req|=1u<<3;}
   if(i==000u&&CounterPINC(&r[4]))req|=1u<<2;
   if(i==010u&&CounterPINC(&r[3]))req|=1u<<4;
   if((ch13&040000u)&&(i&1u)&&CounterDINC(&machine,0,&r[5])){req|=1u<<1;ch13&=037777u;}}
  h=2166136261u;for(i=0;i<6u;++i)h=rr_mix(h,(uint16_t)((unsigned)r[i]&077777u));
  h=rr_mix(h,req);h=rr_mix(h,ch13);
  printf("T %u %08x\n",(unsigned)n,(unsigned)h);}
 return 0;
}
