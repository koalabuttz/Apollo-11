/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only sequential ORIGINAL assembly adapter. Not linked to native code.
 * Shared files supply INPUT fixtures only, compiled DISPATCH_INPUT_ONLY. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_dispatch_cases.h"
static void load_state(const cm_executive *s) {
    unsigned i,j;
    for (i=0;i<CM_EXEC_CORE_COUNT;++i)
        for (j=0;j<CM_EXEC_CORE_WORDS;++j) set(0154+i*12+j,s->core[i][j]);
    for (i=0;i<CM_EXEC_VAC_COUNT;++i)
        for (j=0;j<CM_EXEC_VAC_WORDS;++j) set(0400+i*44+j,s->vac[i][j]);
    set(0067,s->newjob); set(0063,s->newprio);
    set(0065,s->newloc[0]); set(0066,s->newloc[1]); set(0064,s->locctr);
    set(0120,s->fixloc); set(0121,s->ovfind);
}
static void read_state(cm_executive *s) {
    unsigned i,j;
    for (i=0;i<CM_EXEC_CORE_COUNT;++i)
        for (j=0;j<CM_EXEC_CORE_WORDS;++j) s->core[i][j]=(agc_word)get(0154+i*12+j);
    for (i=0;i<CM_EXEC_VAC_COUNT;++i)
        for (j=0;j<CM_EXEC_VAC_WORDS;++j) s->vac[i][j]=(agc_word)get(0400+i*44+j);
    s->newjob=(agc_word)get(0067); s->newprio=(agc_word)get(0063);
    s->newloc[0]=(agc_word)get(0065); s->newloc[1]=(agc_word)get(0066);
    s->locctr=(agc_word)get(0064); s->fixloc=(agc_word)get(0120);
    s->ovfind=(agc_word)get(0121); s->alarm=0;
}
static void bank(unsigned bbcon) {
    set(RegBB,(agc_word)(bbcon & 076007)); set(RegFB,(agc_word)(bbcon & 076000));
    set(RegEB,(agc_word)((bbcon & 7u)<<8));
    machine.InputChannel[7]=machine.OutputChannel7=(int)(bbcon & 0160);
}
static void row(unsigned n,unsigned step,const cm_executive *s, int status,exec_address next) {
    unsigned i,j;
    printf("%u %u %d",n,step,status);
    for (i=0;i<CM_EXEC_CORE_COUNT;++i)
        for (j=0;j<CM_EXEC_CORE_WORDS;++j) printf(" %u",s->core[i][j]);
    for (i=0;i<CM_EXEC_VAC_COUNT;++i)
        for (j=0;j<CM_EXEC_VAC_WORDS;++j) printf(" %u",s->vac[i][j]);
    printf(" %u %u %u %u %u %u %u %u %u %u\n",s->newjob,s->newprio,
           s->newloc[0],s->newloc[1],s->locctr,s->fixloc,s->ovfind,s->alarm,next.location,next.bank);
}
int main(int argc,char **argv) {
    unsigned n,step;
    cm_executive s;
    if (argc!=2) return 2;
    for (n=0;n<DISPATCH_CASE_COUNT;++n) {
        memset(&machine,0,sizeof machine);
        if (agc_load_binfile(&machine,argv[1])!=0) return 2;
        cm_dispatch_prepare(&s,(uint8_t)n); load_state(&s);
        for(unsigned guard=0;guard<16;++guard) set(0300+guard,(agc_word)(012345+guard));
        machine.AllowInterrupt=0; InhibitAlarms=1;
        for (step=0;step<cm_dispatch_steps((uint8_t)n);++step) {
            unsigned cycles,pc=0;
            int status=EXEC_OK;
            exec_address next={0,0};
            cm_dispatch_event e;
            unsigned pending=get(0067);
            cm_dispatch_event_prepare(&e,(uint8_t)n,(uint8_t)step);
            read_state(&s); cm_dispatch_touch(&s,e.touch); load_state(&s);
            machine.Erasable[0][RegA]=0; set(RegL,0);
            if (e.operation==DISPATCH_ADMIT) {
                bank(0); set(RegZ,e.needs_vac ? 05147 : 05134);
                machine.Erasable[0][RegA]=(int16_t)SignExtend(e.priority);
                set(RegQ,01000); set(01000,e.address.location); set(01001,e.address.bank);
            } else if (e.operation==DISPATCH_YIELD) {
                next=e.address;
                bank(e.address.bank);
                if (pending!=0) {
                    unsigned slot=pending/12;
                    next.location=s.core[slot][CM_EXEC_LOC]; next.bank=s.core[slot][CM_EXEC_BANK];
                    set(RegQ,e.address.location); set(RegZ,05164); /* CHANG1 */
                } else {
                    /* Actual basic CCS NEWJOB polling, +0 skips TC CHANG1.
                     * The tiny erasable caller is fixture code, not Executive. */
                    set(01010,010000+0067); set(01011,05164);
                    set(01012,(agc_word)(010000+e.address.location)); set(RegZ,01010);
                }
            } else if (e.operation==DISPATCH_START) {
                unsigned slot=pending/12;
                next.location=s.core[slot][CM_EXEC_LOC]; next.bank=s.core[slot][CM_EXEC_BANK];
                bank(02000); set(RegL,0);
                if (pending==0) { set(RegZ,03242); machine.Erasable[0][RegA]=2; }
                else set(RegZ,03032); /* NUCHANG2; address checked by prepare script */
            } else {
                unsigned slot=0,highest=0,i;
                for(i=1;i<CM_EXEC_CORE_COUNT;++i)
                    if (s.core[i][CM_EXEC_PRIORITY]>highest && s.core[i][CM_EXEC_PRIORITY]<040000) {
                        highest=s.core[i][CM_EXEC_PRIORITY]; slot=i;
                    }
                if (slot!=0) { next.location=s.core[slot][CM_EXEC_LOC]; next.bank=s.core[slot][CM_EXEC_BANK]; }
                else status=EXEC_IDLE;
                bank(02000 | e.superbank); set(RegZ,03124);
            }
            for (cycles=0;cycles<4000;++cycles) {
                pc=get(RegZ);
                if (!machine.PendFlag && !machine.ExtraDelay) {
                    if (e.operation==DISPATCH_ADMIT && pc==05222) break;
                    if (e.operation!=DISPATCH_ADMIT && status==EXEC_IDLE && pc==03225) break;
                    if (e.operation!=DISPATCH_ADMIT && status!=EXEC_IDLE &&
                        pc==next.location && get(RegBB)==(next.bank & 076007) &&
                        (unsigned)machine.OutputChannel7==(next.bank & 0160)) break;
                }
                if (agc_engine(&machine)!=0) return 2;
            }
            if (cycles==4000) { fprintf(stderr,"case %u step %u timeout at %05o FB=%05o\n",n,step,pc,get(RegFB)); return 2; }
            for(unsigned guard=0;guard<16;++guard) if(get(0300+guard)!=012345+guard) return 2;
            read_state(&s); row(n,step,&s,status,next);
            fprintf(stderr,"case %u step %u: %u AGC cycles stop %05o BB=%05o SB=%03o\n",
                    n,step,cycles,pc,get(RegBB),machine.OutputChannel7);
        }
    }
    return 0;
}
