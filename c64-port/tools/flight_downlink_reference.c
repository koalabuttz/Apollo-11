/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only reference for flight-downlink-vice (tools/flight_downlink_check.py
 * --vice, docs/FLIGHT-DOWNLINK.md), linked to Virtual AGC. NEVER part of the
 * native port.
 *
 * yaAGC, its clock frozen (AgcFrozenClock, as under FLIGHT_SCALED), from the
 * native state tools/flight_host_run.c wrote at a sample point (DL_SNAP,
 * FLIGHT_SNAPSHOT's header), takes N DOWNRUPTs one after another: each is
 * requested (InterruptRequests[8]), every other request dropped (the C64
 * runs the window with CIA1 interrupts off), and yaAGC runs until it has
 * taken it and returned from it (RESUME). After each one it prints channels
 * 34 and 35 and the word-order bit (channel 13 bit 7), in octal, one
 * DOWNRUPT a line.
 *
 *   flight-downlink-reference ROPE.bin N
 *   (build with -DRK_LM=1 or -DRK_CM=1 and -DDL_SNAP="header") */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "flight_io.h"
#define SNAP_DATA
#include DL_SNAP
void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){if(c==7)s->InputChannel[7]=s->OutputChannel7=v&0160;}
void ChannelRoutine(agc_t *s){(void)s;}
void RequestRadarData(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int data){(void)s;(void)data;}
void UnblockSocket(int n){(void)n;}
extern int AgcFrozenClock;
static agc_t m;
int main(int argc,char **argv){
 struct { agc_word superbank,interrupt_enabled; } dev;
 agc_hardware hw;flight_io io;unsigned i;long n,k,count;
 if(argc<3||agc_engine_init(&m,argv[1],0,0)){fprintf(stderr,"usage: flight-downlink-reference ROPE.bin N\n");return 2;}
 count=strtol(argv[2],0,10);
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
 for(k=0;k<count;++k){int taken=0;
  memset(m.InterruptRequests,0,sizeof m.InterruptRequests);m.InterruptRequests[8]=1;
  for(n=0;n<200000l&&!(taken&&!m.InIsr);++n){
   if(m.InIsr)taken=1;
   if(agc_engine(&m))return 2;
   m.DownruptTimeValid=0;
   for(i=1;i<=NUM_INTERRUPT_TYPES;++i)if(i!=8u||taken||m.InIsr)m.InterruptRequests[i]=0;}
  if(!taken||m.InIsr){fprintf(stderr,"DOWNRUPT %ld: %s after %ld instructions\n",k,taken?"no RESUME":"not taken",n);return 1;}
  printf("%05o %05o %o\n",(unsigned)m.InputChannel[034]&077777u,(unsigned)m.InputChannel[035]&077777u,(m.InputChannel[013]&0100)!=0);}
 return 0;}
