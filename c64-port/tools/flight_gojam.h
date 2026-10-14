/* SPDX-License-Identifier: GPL-2.0-or-later
 * FLIGHT_GOJAM=CS[,CS...] for tools/flight_host_run.c (host only, NEVER part
 * of the native port; docs/FLIGHT-RESTART.md): a hardware restart at the
 * first sample point at or after each CS centiseconds, on both sides.
 *
 * The sample point is a clean one: idle at ADVAN, no interrupt in progress,
 * none pending, no instruction half done (no DOWNRUPT either: the harness
 * runs none), after that point's pokes and before its key. There the
 * restart is yaAGC's own GOJAM (agc_engine.c, "If we triggered any
 * alarms, simulate a GOJAM": the GOJAM and TC 4000 sequences), the same
 * thing a TC trap (V69), night watchman, rupt lock or parity alarm does:
 * Q = Z, Z = 04000, interrupts enabled and none in progress, every request
 * and HANDRUPT trap cleared, output channels 5, 6, 10-14, 34 and 35 zeroed
 * and channel 33 bit 11 set. The superbank (channel 7) and every erasable
 * word but Z and Q stay. Then Luminary's or Comanche's own code runs from
 * 04000: GOPROG, the phase-table check (PCLOOP), STARTSUB/STARTSB2 and each
 * active group's restart (NXTRST, RESTARTS), native routines checked
 * against yaAGC as everywhere else in the run. */
#ifndef FLIGHT_GOJAM_H
#define FLIGHT_GOJAM_H
#include <stdio.h>
#include <stdlib.h>
#include "agc_engine.h"
#include "agc_transfer.h"
#include "flight_io.h"

enum { GOJAMS=16 };
static unsigned long gojam_at[GOJAMS];
static unsigned ngojam;
static int gojam_parse(const char *t){char *e;
 while(t&&*t&&ngojam<GOJAMS){gojam_at[ngojam++]=strtoul(t,&e,10);if(e==t)return 0;t=*e==','?e+1:e;}
 return !t||!*t;}
/* Whether a GOJAM is due at time t (done counts them, once each side). */
static int gojam_due(unsigned long t,unsigned *done){
 if(*done>=ngojam||t<gojam_at[*done])return 0;
 while(*done<ngojam&&t>=gojam_at[*done])++*done;
 return 1;}
/* Native side: the image, the flight_io record, agc_hardware and the
 * pending interrupt requests. */
static void gojam_native(agc_word *w,flight_io *io,agc_hardware *hw,uint16_t *pending){
 w[2]=w[5];w[5]=04000u;
 io->interrupt_enabled=1u;hw->in_interrupt=0u;*pending=0u;
 io->trap31a=io->trap31b=io->trap32=0u;
 hw->channel5=hw->channel6=hw->channel10=hw->channel11=hw->channel12=hw->channel13=hw->channel14=0u;
 io->channel34=io->channel35=0u;
 hw->channel33|=02000u;}
/* yaAGC's side, as agc_engine.c does it (its CpuWriteIO is static: the
 * channel writes here have no side effect but the restart light's, and the
 * harness drives no DSKY). */
static void gojam_yaagc(agc_t *s){int i;
 s->Erasable[0][2]=s->Erasable[0][5];s->Erasable[0][5]=04000;
 s->InIsr=0;s->AllowInterrupt=1;s->ParityFail=0;
 s->Trap31A=s->Trap31B=s->Trap32=0;
 for(i=1;i<=NUM_INTERRUPT_TYPES;++i)s->InterruptRequests[i]=0;
 s->InputChannel[005]=s->InputChannel[006]=s->InputChannel[010]=s->InputChannel[011]=0;
 s->InputChannel[012]=s->InputChannel[013]=s->InputChannel[014]=0;
 s->InputChannel[033]|=002000;
 s->InputChannel[034]=s->InputChannel[035]=0;s->Downlink=0;s->DownruptTimeValid=0;
 s->IndexValue=0;   /* AGC_P0 */
 s->ExtraCode=0;s->SubstituteInstruction=0;s->PendFlag=0;s->PendDelay=0;
 s->TookBZF=0;s->TookBZMF=0;
 s->RestartLight=1;}
#endif
