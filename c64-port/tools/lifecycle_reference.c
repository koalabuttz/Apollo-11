/* SPDX-License-Identifier: GPL-2.0-or-later
 * ORIGINAL assembly, input-only fixtures. No native Executive is linked. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "lifecycle_cases.h"
static void load_state(const executive *s) {
    unsigned i,j;
    for(i=0;i<8;++i) for(j=0;j<12;++j) set(0154+i*12+j,s->core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) set(0400+i*44+j,s->vac[i][j]);
    set(0067,s->newjob); set(0063,s->newprio); set(0065,s->newloc[0]);
    set(0066,s->newloc[1]); set(0064,s->locctr); set(0120,s->fixloc); set(0121,s->ovfind);
}
static void read_state(executive *s) {
    unsigned i,j;
    for(i=0;i<8;++i) for(j=0;j<12;++j) s->core[i][j]=(agc_word)get(0154+i*12+j);
    for(i=0;i<5;++i) for(j=0;j<44;++j) s->vac[i][j]=(agc_word)get(0400+i*44+j);
    s->newjob=(agc_word)get(0067); s->newprio=(agc_word)get(0063);
    s->newloc[0]=(agc_word)get(0065); s->newloc[1]=(agc_word)get(0066);
    s->locctr=(agc_word)get(0064); s->fixloc=(agc_word)get(0120); s->ovfind=(agc_word)get(0121); s->alarm=0;
}
static void bank(agc_word b) {
    set(RegBB,(agc_word)(b & 076007)); set(RegFB,(agc_word)(b & 076000));
    set(RegEB,(agc_word)((b & 7u)<<8)); machine.InputChannel[7]=machine.OutputChannel7=b & 0160;
}
static void row(unsigned n,unsigned step,const executive *s,int status,exec_address next) {
    unsigned i,j;
    printf("%u %u %d",n,step,status);
    for(i=0;i<8;++i) for(j=0;j<12;++j) printf(" %u",s->core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) printf(" %u",s->vac[i][j]);
    printf(" %u %u %u %u %u %u %u %u %u %u\n",s->newjob,s->newprio,
           s->newloc[0],s->newloc[1],s->locctr,s->fixloc,s->ovfind,s->alarm,next.location,next.bank);
}
int main(int argc,char **argv) {
    unsigned n,step;
    if(argc!=2) return 2;
    for(n=0;n<LIFECYCLE_CASE_COUNT;++n) {
        executive state;
        memset(&machine,0,sizeof machine);
        if(agc_load_binfile(&machine,argv[1])!=0) return 2;
        lifecycle_prepare(&state,(uint8_t)n); load_state(&state);
        InhibitAlarms=1;
        for(step=0;step<lifecycle_steps((uint8_t)n);++step) {
            unsigned cycles,pc=0,i;
            int status=EXEC_OK,transfers;
            exec_address next={0,0};
            lifecycle_event e;
            lifecycle_event_prepare(&e,(uint8_t)n,(uint8_t)step);
            read_state(&state); dispatch_touch(&state,e.touch); load_state(&state);
            bank(e.address.bank); set(RegA,0); set(RegL,0); machine.AllowInterrupt=0;
            transfers=e.operation!=LIFE_ADMIT && e.operation!=LIFE_WAKE;
            if(e.operation==LIFE_SLEEP) { machine.Erasable[0][RegA]=(int16_t)SignExtend(e.cadr); set(RegZ,05133); }
            else if(e.operation==LIFE_WAKE) { machine.Erasable[0][RegA]=(int16_t)SignExtend(e.cadr); set(RegQ,01002); set(RegZ,05137); }
            else if(e.operation==LIFE_PRIORITY) { set(RegA,e.priority); set(RegQ,e.address.location); set(RegZ,05146); }
            else if(e.operation==LIFE_ADMIT) {
                set(RegA,e.priority); set(RegQ,01000);
                set(01000,e.address.location); set(01001,e.address.bank);
                set(RegZ,e.needs_vac ? 05105 : 05072);
            } else if(e.operation==LIFE_START) {
                bank(02000); set(RegA,2); set(RegZ,state.newjob==0 ? 03225 : 03011);
            } else { bank((agc_word)(02000 | (e.address.bank & 0160))); set(RegZ,03103); }
            for(cycles=0;cycles<4000;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(!transfers && pc==05160) {
                        if(e.operation==LIFE_WAKE && get(0064)==077776) status=EXEC_NOT_FOUND;
                        break;
                    }
                    if(transfers && pc==03210) { status=EXEC_IDLE; break; }
                    if(transfers) {
                        unsigned expected_bank=0;
                        int hit=0;
                        /* Stop at whichever actual transfer executes, not a
                         * native/predicted scan result. Check physical banks. */
                        if(e.operation==LIFE_PRIORITY && pc==e.address.location) {
                            hit=1; expected_bank=e.address.bank;
                        }
                        for(i=e.operation==LIFE_START ? 0 : 1;i<8;++i)
                            if(state.core[i][EXEC_PRIORITY]>0 && state.core[i][EXEC_PRIORITY]<040000 &&
                               pc==state.core[i][EXEC_LOC]) {
                                hit=1; expected_bank=state.core[i][EXEC_BANK]; break;
                            }
                        /* Locations can overlap Executive's switched-bank
                         * instructions (e.g. HIGH_ENTRY == JOBSLP2+1). */
                        if(hit && get(RegBB)==(expected_bank & 076007) &&
                           (unsigned)machine.OutputChannel7==(expected_bank & 0160)) {
                            next.location=(agc_word)pc;
                            next.bank=(agc_word)(get(RegBB) | (unsigned)machine.OutputChannel7);
                            break;
                        }
                    }
                }
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==4000) { fprintf(stderr,"lifecycle %u/%u timeout PC=%05o FB=%05o\n",n,step,pc,get(RegFB)); return 2; }
            read_state(&state); row(n,step,&state,status,next);
            fprintf(stderr,"lifecycle %u/%u: %u AGC cycles PC=%05o BB=%05o SB=%03o status=%d\n",
                    n,step,cycles,pc,get(RegBB),machine.OutputChannel7,status);
        }
    }
    return 0;
}
