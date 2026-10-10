/* SPDX-License-Identifier: MIT
 * Manually constructed initial states. This is input data, NOT a reference oracle. */
#include "scenarios.h"
static void occupy(scenario *s, uint8_t i, agc_word p) {
    s->state.core[i][EXEC_PRIORITY]=(agc_word)(p+EXEC_FAKEPRET);
    s->state.core[i][EXEC_PUSH]=EXEC_FAKEPRET;
    s->state.core[i][EXEC_LOC]=(agc_word)(02000+i);
    s->state.core[i][EXEC_BANK]=06000;
}
void scenario_prepare(scenario *s, uint8_t n) {
    uint8_t i,j;
    /* Explicit initial state independent of exec_init. */
    for (i=0;i<EXEC_CORE_COUNT;++i) {
        for(j=0;j<EXEC_CORE_WORDS;++j) s->state.core[i][j]=0;
        s->state.core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
    }
    for(i=0;i<EXEC_VAC_COUNT;++i) {
        for(j=0;j<EXEC_VAC_WORDS;++j) s->state.vac[i][j]=0;
        s->state.vac[i][0]=(agc_word)(EXEC_VAC1USE+i*EXEC_VAC_WORDS);
    }
    s->state.newjob=AGC_NEG_ZERO; s->state.newprio=0;
    s->state.newloc[0]=0; s->state.newloc[1]=0; s->state.locctr=0;
    s->state.fixloc=0; s->state.ovfind=0; s->state.alarm=0;
    s->priority=02000; s->location=02345; s->bank=06000;
    s->needs_vac=0; s->complete=0; s->name="admission";
    if(n==0) s->name="idle-novac";
    else if(n>=1 && n<=5) {
        s->name="vac-scan"; s->needs_vac=1;
        for(i=0;i<n-1;++i) s->state.vac[i][0]=0;
    } else if(n==6) {
        s->name="vac-1201"; s->needs_vac=1;
        for(i=0;i<EXEC_VAC_COUNT;++i) s->state.vac[i][0]=0;
    } else if(n>=7 && n<=10) {
        s->name="priority-selection";
        occupy(s,0,04000); occupy(s,1,02000); s->state.newjob=0;
        s->priority=n==7 ? 03000 : n==8 ? 04000 : n==9 ? 05000 : 037000;
        s->needs_vac=n==10;
    } else if(n==11) {
        s->name="queued-highest";
        occupy(s,0,02000); occupy(s,1,06000); s->state.newjob=12;
        s->priority=05000;
    } else if(n==12 || n==13) {
        s->name=n==12 ? "eighth-core-admission" : "eighth-core-vac";
        for(i=0;i<7;++i) occupy(s,i,02000);
        s->state.newjob=0; s->needs_vac=n==13;
    } else if(n==14) {
        s->name="negative-and-minus-zero-core-scan";
        occupy(s,0,02000); occupy(s,1,01000);
        s->state.core[1][EXEC_PRIORITY]^=AGC_MASK;
        s->state.newjob=0; s->priority=03000;
    } else if(n==15) {
        s->name="last-admission-slot";
        for(i=0;i<6;++i) occupy(s,i,02000);
        s->state.newjob=0; s->priority=03000;
    } else if(n>=16 && n<=21) {
        s->name="endjob-selection"; s->complete=1;
        occupy(s,0,02000);
        if(n>=17) { s->state.core[0][EXEC_PRIORITY]=02401;
                    s->state.core[0][EXEC_PUSH]=0401; s->state.vac[0][0]=0; }
        if(n>=18) occupy(s,1,03000);
        if(n>=19) occupy(s,2,03000);
        if(n==20) occupy(s,7,04000);
        if(n==21) s->state.core[1][EXEC_PRIORITY]=(03110 ^ AGC_MASK);
        s->state.newjob=0;
    } else if(n==22) {
        s->name="core-zero-with-queued-newjob";
        occupy(s,1,05000); s->state.newjob=12; s->priority=04000;
    } else if(n==23) {
        s->name="reuse-stale-location";
        s->state.core[0][EXEC_LOC]=07777; s->state.core[0][EXEC_BANK]=01234;
    } else if(n==24 || n==25) {
        s->name=n==24 ? "core-1202" : "vac-reserved-on-1202";
        for(i=0;i<EXEC_ADMISSION_COUNT;++i) occupy(s,i,02000);
        s->state.newjob=0; s->needs_vac=n==25;
    }
}
int scenario_expected(uint8_t n,const executive *s,int r) {
    /* Small diagnostic expectations are MANUAL; differential test checks all state. */
    if(n==6) return r==EXEC_NO_VAC && s->alarm==01201;
    if(n==24 || n==25) return r==EXEC_NO_CORE && s->alarm==01202 &&
                                   (n==24 || s->vac[0][0]==0);
    if(n>=16 && n<=21) {
        int expected=n<=17 ? -1 : n==20 ? 7 : n==21 ? 2 : 1;
        return r==expected && s->core[0][EXEC_PRIORITY]==AGC_NEG_ZERO &&
               (n==16 || s->vac[0][0]==0400);
    }
    return r==EXEC_OK && s->core[s->locctr/12][EXEC_LOC]==02345;
}
