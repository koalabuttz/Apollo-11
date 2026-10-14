/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only yaAGC adapter for tests/resident_cdu_cases.c. NEVER part of the
 * native port; no AGC instruction implementation is copied here.
 *   resident-cdu-reference ROPE.bin
 * runs the IMU CDU drive timeline on agc_engine (its own BurstOutput) and
 * prints one line per period ("C n digest"). The machine runs TC 01000 at
 * 01000 with interrupts inhibited, so nothing but the drive touches
 * 050-052; a period is 1024000 / 12 / 100 = 853 1/3 MCTs, its boundaries
 * at ceil(n * 2560 / 3) cycles. */
#include <stdio.h>
#include <stdlib.h>
#include "agc_engine.h"
#include "resident_cdu_cases.h"
void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){(void)s;(void)c;(void)v;}
void ChannelRoutine(agc_t *s){(void)s;}
void RequestRadarData(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int data){(void)s;(void)data;}
void UnblockSocket(int n){(void)n;}
static agc_t machine;
int main(int argc,char **argv){uint32_t n;uint64_t base;unsigned i,k;
 if(argc!=2||agc_load_binfile(&machine,argv[1]))return 2;
 InhibitAlarms=1;
 for(i=0;i<8u;++i)for(k=0;k<0400u;++k)machine.Erasable[i][k]=0;
 machine.Erasable[01000/0400][01000%0400]=01000;   /* TC 01000 */
 machine.Erasable[0][5]=01000;machine.AllowInterrupt=0;machine.InIsr=0;machine.DownruptTimeValid=0;
 machine.ExtraCode=0;machine.IndexValue=0;machine.SubstituteInstruction=0;machine.PendFlag=0;machine.PendDelay=0;machine.ExtraDelay=0;
 machine.CycleCounter=1;base=machine.CycleCounter;
 for(n=0;n<RC_PERIODS;++n){agc_word ch14,write[3],cmd[3];uint8_t set[3];uint64_t end=base+((uint64_t)(n+1u)*2560u+2u)/3u;
  rc_input(n,&ch14,set,write);
  machine.InputChannel[014]=(int16_t)ch14;
  for(k=0;k<3u;++k)if(set[k])machine.Erasable[0][050u+k]=(int16_t)write[k];
  while(machine.CycleCounter<end){machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  for(k=0;k<3u;++k)cmd[k]=(agc_word)((unsigned)machine.Erasable[0][050u+k]&077777u);
  printf("C %u %08x\n",(unsigned)n,(unsigned)rc_digest(cmd));}
 return 0;
}
