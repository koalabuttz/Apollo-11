/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only reference for flight-restart-vice (tools/flight_restart_check.py,
 * docs/FLIGHT-RESTART.md), linked to Virtual AGC. NEVER part of the native
 * port.
 *
 * yaAGC, its clock frozen (AgcFrozenClock, as under FLIGHT_SCALED) and no
 * interrupt taken (every request dropped, as the C64 runs the burst with
 * CIA1 interrupts off), from the native state tools/flight_host_run.c wrote
 * where FLIGHT_GOJAM restarted the AGC (RS_SNAP, FLIGHT_SNAPSHOT's header),
 * until it is idle at ADVAN with NEWJOB -0 (DUMMYJOB's; DUMMYJOB +2 reaches
 * ADVAN with NEWJOB +0, a job ready to run), or, given STOP (octal Z FB SUPERBANK), until
 * its first arrival there at an instruction boundary. Writes the 2,048
 * erasable words (A, L and Q at 16 bits) and its summary to OUT.
 *
 *   flight-restart-reference ROPE.bin OUT [Z FB SUPERBANK]
 *   (build with -DRK_LM=1 or -DRK_CM=1 and -DRS_SNAP="header") */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "flight_io.h"
#define SNAP_DATA
#include RS_SNAP
#if RK_LM
#define ADVAN 03214u
#else
#define ADVAN 03231u
#endif
void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){if(c==7)s->InputChannel[7]=s->OutputChannel7=v&0160;}
void ChannelRoutine(agc_t *s){(void)s;}
void RequestRadarData(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int data){(void)s;(void)data;}
void UnblockSocket(int n){(void)n;}
extern int AgcFrozenClock;
static agc_t m;
static int quiet(void){return !m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode&&!m.SubstituteInstruction;}
static unsigned word(unsigned a){return (unsigned)m.Erasable[a/0400u][a%0400u]&(a<3u?0177777u:077777u);}
int main(int argc,char **argv){
 struct { agc_word superbank,interrupt_enabled; } dev;
 agc_hardware hw;flight_io io;FILE *f;unsigned i,z=0,fb=0,sb=0;long n,limit=50000000l;int stop=argc>5,at=0;
 if(argc<3||agc_engine_init(&m,argv[1],0,0)){fprintf(stderr,"usage: flight-restart-reference ROPE.bin OUT [Z FB SUPERBANK]\n");return 2;}
 if(stop){z=(unsigned)strtoul(argv[3],0,8);fb=(unsigned)strtoul(argv[4],0,8);sb=(unsigned)strtoul(argv[5],0,8);}
 memset(&dev,0,sizeof dev);memset(&hw,0,sizeof hw);memset(&io,0,sizeof io);
 SNAP_IO(dev)
 SNAP_HW(hw)
 SNAP_FLIGHT(io)
 InhibitAlarms=1;AgcFrozenClock=1;
 for(i=0;i<2048u;++i)m.Erasable[i/0400u][i%0400u]=(int16_t)snap_words[i];
 for(i=0;i<NUM_CHANNELS;++i)m.InputChannel[i]=0;
 m.InputChannel[3]=(int16_t)hw.channel3;m.InputChannel[4]=(int16_t)hw.channel4;m.InputChannel[5]=(int16_t)hw.channel5;
 m.InputChannel[6]=(int16_t)hw.channel6;m.InputChannel[010]=(int16_t)hw.channel10;m.InputChannel[011]=(int16_t)hw.channel11;
 m.InputChannel[012]=(int16_t)hw.channel12;m.InputChannel[013]=(int16_t)hw.channel13;m.InputChannel[014]=(int16_t)hw.channel14;
 m.InputChannel[015]=(int16_t)hw.channel15;m.InputChannel[030]=(int16_t)hw.channel30;m.InputChannel[031]=(int16_t)hw.channel31;
 m.InputChannel[032]=(int16_t)hw.channel32;m.InputChannel[033]=(int16_t)hw.channel33;
 m.InputChannel[016]=(int16_t)io.channel16;m.InputChannel[034]=(int16_t)io.channel34;m.InputChannel[035]=(int16_t)io.channel35;
 m.InputChannel[7]=m.OutputChannel7=(int16_t)dev.superbank;m.AllowInterrupt=dev.interrupt_enabled!=0u;m.InIsr=hw.in_interrupt!=0u;
 m.Trap31A=io.trap31a;m.Trap31B=io.trap31b;m.Trap32=io.trap32;
 memset(m.InterruptRequests,0,sizeof m.InterruptRequests);m.DownruptTimeValid=0;
 for(n=0;n<limit;++n){
  if(n&&quiet()&&!m.InIsr){unsigned zz=word(5)&07777u,ff=word(4);
   if(stop?(zz==z&&(zz<02000u||zz>=04000u||((ff^fb)&076000u)==0u)&&(unsigned)(m.OutputChannel7&0160)==sb)
          :(zz==ADVAN&&((ff>>10)&037u)==1u&&word(067)==077777u)){at=1;break;}}
  if(agc_engine(&m))return 2;
  memset(m.InterruptRequests,0,sizeof m.InterruptRequests);}
 if(!(f=fopen(argv[2],"wb"))){perror(argv[2]);return 2;}
 for(i=0;i<2048u;++i){unsigned w=word(i);fputc((int)(w&0377u),f);fputc((int)(w>>8),f);}
 fclose(f);
 printf("reference: %s after %ld instructions: Z %05o FB %05o superbank %03o, NEWJOB %05o, interrupts %s\n",
  at?"arrived":"did not arrive",n,word(5),word(4),(unsigned)(m.OutputChannel7&0160),word(067),m.AllowInterrupt?"enabled":"inhibited");
 return at?0:1;}
