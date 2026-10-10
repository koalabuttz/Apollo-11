/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only test adapter linked to Virtual AGC. NEVER part of the native port.
 * No AGC instruction implementation is copied here. Peripheral inputs and
 * trace callbacks are disabled in this deliberately interrupt-free fixture. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "scenarios.h"
#define ARITH_EXTEND(x) SignExtend((int16_t)(x))
#define ARITH_CORRECT(x) OverflowCorrected(x)
#define ARITH_ADD(a,b) AddSP16(a,b)
#include "arithmetic_stream.h"
void BacktraceAdd(agc_t *s,int cause) { (void)s; (void)cause; }
int ChannelInput(agc_t *s) { (void)s; return 0; }
void ChannelOutput(agc_t *s,int c,int v) { (void)s; (void)c; (void)v; }
void ChannelRoutine(agc_t *s) { (void)s; }
void RequestRadarData(agc_t *s) { (void)s; }
void ShiftToDeda(agc_t *s,int data) { (void)s; (void)data; }
void UnblockSocket(int n) { (void)n; }
static agc_t machine;
static int16_t *memory(unsigned address) {
    return &machine.Erasable[address/0400][address%0400];
}
static void set(unsigned a,agc_word v) { *memory(a)=(int16_t)v; }
static unsigned get(unsigned a) { return (unsigned)*memory(a)&077777u; }
int main(int argc,char **argv) {
    scenario s;
    unsigned n,i,j;
    if(argc!=2) { fprintf(stderr,"usage: reference-runner ROM\n"); return 2; }
    if(strcmp(argv[1],"--arithmetic")==0) return arithmetic_stream();
    for(n=0;n<SCENARIO_COUNT;++n) {
        unsigned alarm=0,pc=0;
        int result=0;
        unsigned cycles;
        memset(&machine,0,sizeof machine);
        if(agc_load_binfile(&machine,argv[1])!=0) return 2;
        scenario_prepare(&s,(uint8_t)n);
        for(i=0;i<EXEC_CORE_COUNT;++i)
            for(j=0;j<EXEC_CORE_WORDS;++j) set(0154+i*12+j,s.state.core[i][j]);
        for(i=0;i<EXEC_VAC_COUNT;++i)
            for(j=0;j<EXEC_VAC_WORDS;++j) set(0400+i*44+j,s.state.vac[i][j]);
        set(0067,s.state.newjob); set(0063,s.state.newprio);
        set(0065,s.state.newloc[0]); set(0066,s.state.newloc[1]);
        set(0064,s.state.locctr); set(0120,s.state.fixloc); set(0121,s.state.ovfind);
        machine.Erasable[0][RegA]=(int16_t)SignExtend(s.priority);
        set(RegQ,01000); set(01000,s.location); set(01001,s.bank);
        set(RegFB,s.complete ? 02000 : 0);
        set(RegZ,s.complete ? 03103 : s.needs_vac ? 05105 : 05072);
        machine.AllowInterrupt=0;
        InhibitAlarms=1; /* Disable emulator hardware watchdog alarms only. */
        for(cycles=0;cycles<4000;++cycles) {
            pc=get(RegZ);
            if(!machine.PendFlag && !machine.ExtraDelay) {
                if(!s.complete && pc==05160) break; /* ENDFIND, before bank restore */
                if(!s.complete && pc==05716) {
                    unsigned q=get(RegQ);
                    alarm=(unsigned)machine.Fixed[1][q&01777]&077777u;
                    result=(int)alarm; break; /* BAILOUT1 entry, no recovery */
                }
                if(s.complete && (pc==02704 || pc==03206)) {
                    result=pc==03206 ? -1 : (int)get(0067)/12; break;
                }
            }
            if(agc_engine(&machine)!=0) { fprintf(stderr,"engine failed\n"); return 2; }
        }
        if(cycles==4000) {
            fprintf(stderr,"scenario %u timed out at %05o FB=%05o\n",n,pc,get(RegFB));
            return 2;
        }
        fprintf(stderr,"scenario %u: %u AGC cycles, stop %05o, alarm %04o\n",n,cycles,pc,alarm);
        printf("%u %d",n,result);
        for(i=0;i<EXEC_CORE_COUNT;++i)
            for(j=0;j<EXEC_CORE_WORDS;++j) printf(" %u",get(0154+i*12+j));
        for(i=0;i<EXEC_VAC_COUNT;++i)
            for(j=0;j<EXEC_VAC_WORDS;++j) printf(" %u",get(0400+i*44+j));
        printf(" %u %u %u %u %u %u %u %u\n",get(0067),get(0063),get(0065),get(0066),
               get(0064),get(0120),get(0121),alarm);
    }
    return 0;
}
