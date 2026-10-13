/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only yaAGC adapter for tests/resident_rupt_cases.c. NEVER part of the
 * native port; no AGC instruction implementation is copied here.
 *   resident-rupt-reference lm|cm ROPE.bin
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
int main(int argc,char **argv){unsigned mission,i;uint32_t n,h;const char *show=getenv("RR_SHOW");
 if(argc!=3||(strcmp(argv[1],"lm")&&strcmp(argv[1],"cm")))return 2;
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
