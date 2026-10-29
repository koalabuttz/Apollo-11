/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only unchanged-original-assembly oracle. Input constructors and row
 * serialization are shared; no native Executive/runtime implementation links. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "interpreter_native_cases.h"

static void load_state(const executive *s) {
    unsigned i,j;
    for(i=0;i<EXEC_CORE_COUNT;++i)
        for(j=0;j<EXEC_CORE_WORDS;++j) set(0154+i*12+j,s->core[i][j]);
    for(i=0;i<EXEC_VAC_COUNT;++i)
        for(j=0;j<EXEC_VAC_WORDS;++j) set(0400+i*44+j,s->vac[i][j]);
    set(0067,s->newjob); set(0063,s->newprio);
    set(0065,s->newloc[0]); set(0066,s->newloc[1]); set(0064,s->locctr);
    set(0120,s->fixloc); set(0121,s->ovfind);
}
static void read_state(executive *s) {
    unsigned i,j;
    for(i=0;i<EXEC_CORE_COUNT;++i)
        for(j=0;j<EXEC_CORE_WORDS;++j) s->core[i][j]=(agc_word)get(0154+i*12+j);
    for(i=0;i<EXEC_VAC_COUNT;++i)
        for(j=0;j<EXEC_VAC_WORDS;++j) s->vac[i][j]=(agc_word)get(0400+i*44+j);
    s->newjob=(agc_word)get(0067); s->newprio=(agc_word)get(0063);
    s->newloc[0]=(agc_word)get(0065); s->newloc[1]=(agc_word)get(0066);
    s->locctr=(agc_word)get(0064); s->fixloc=(agc_word)get(0120);
    s->ovfind=(agc_word)get(0121); s->alarm=0;
}
static void bank(unsigned b) {
    set(RegBB,(agc_word)(b&076007)); set(RegFB,(agc_word)(b&076000));
    set(RegEB,(agc_word)((b&7u)<<8));
    machine.InputChannel[7]=machine.OutputChannel7=(int)(b&0160);
}
static int physical_bank(unsigned b) {
    return get(RegBB)==(b&076007u) &&
        (unsigned)machine.OutputChannel7==(b&0160u);
}
int main(int argc,char **argv) {
    unsigned n,step;
    if(argc!=2) return 2;
    InhibitAlarms=1;
    for(n=0;n<IN_CASE_COUNT;++n) {
        executive state;
        exec_address current={0,0};
        exec_interpreter control={0,0};
        enum exec_context_kind kind=EXEC_CONTEXT_BASIC;
        if((n>=7u && n<21u) || n>=28u) {
            current.location=n>=28u ? IN_HIGH_PRIORITY :
                n<14u ? IN_HIGH_BRIDGE : IN_HIGH_SLEEP;
            current.bank=IN_HIGH_BANK;
        }
        memset(&machine,0,sizeof machine);
        if(agc_load_binfile(&machine,argv[1])!=0) return 2;
        in_prepare(&state,n); load_state(&state);
        for(step=0;step<in_steps(n);++step) {
            in_event e;
            unsigned cycles,pc=0,i;
            enum exec_status status=EXEC_OK;
            int transfers;
            in_event_prepare(&e,n,step);
            read_state(&state); in_touch(&state,e.touch); load_state(&state);
            set(RegA,0); set(RegL,0); machine.AllowInterrupt=0;
            set(0115,control.intbit15); set(RegEDOP,control.edop);
            transfers=e.operation!=IN_ADMIT && e.operation!=IN_WAKE;
            if(e.operation==IN_ADMIT) {
                bank(0); set(RegA,e.priority); set(RegQ,01000);
                set(01000,e.address.location); set(01001,e.address.bank);
                set(RegZ,05105);
            } else if(e.operation==IN_START) {
                bank(02000); set(RegA,2);
                set(RegZ,state.newjob==0 ? 03225 : 03011);
            } else if(e.operation==IN_ENTER) {
                bank(e.address.bank); set(RegQ,e.address.location); set(RegZ,06037);
            } else if(e.operation==IN_PRIO) {
                bank(current.bank);
                machine.Erasable[0][RegA]=(int16_t)SignExtend(e.priority);
                set(RegQ,e.address.location); set(RegZ,05146);
            } else if(e.operation==IN_YIELD || e.operation==IN_EXIT) {
                bank(current.bank);
                if(e.operation==IN_YIELD && kind==EXEC_CONTEXT_BASIC) {
                    set(RegQ,e.address.location); set(RegZ,05122);
                } else {
                    set(0164,(agc_word)(e.address.location-1u));
                    state.core[0][EXEC_LOC]=(agc_word)(e.address.location-1u);
                    set(RegZ,e.operation==IN_YIELD ? 06061 : 06743);
                }
            } else if(e.operation==IN_SLEEP || e.operation==IN_WAKE) {
                unsigned cadr=(e.address.location-02000u)|(e.address.bank&076000u);
                bank(current.bank);
                machine.Erasable[0][RegA]=(int16_t)SignExtend((int)cadr);
                set(RegQ,01002); set(RegZ,e.operation==IN_SLEEP ? 05133 : 05137);
            } else {
                bank((current.bank&0177u)|02000u); set(RegZ,03103);
            }
            for(cycles=0;cycles<4000;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(!transfers && pc==05160) {
                        if(e.operation==IN_WAKE && get(0064)==077776) status=EXEC_NOT_FOUND;
                        break;
                    }
                    if(transfers && pc==03210 && get(RegFB)==02000) {
                        status=EXEC_IDLE; current.location=0; current.bank=0; break;
                    }
                    if(transfers && pc==06070) {
                        unsigned loc=get(0164),expected_bank=0;
                        int hit=0;
                        if(e.operation==IN_ENTER ||
                           (e.operation==IN_YIELD && state.newjob==0)) {
                            hit=loc==e.address.location;
                            expected_bank=e.address.bank;
                        }
                        for(i=1;i<EXEC_CORE_COUNT;++i)
                            if(state.core[i][EXEC_PRIORITY]>0 &&
                               state.core[i][EXEC_PRIORITY]<040000 &&
                               (state.core[i][EXEC_LOC]&040000)!=0 &&
                               loc==((state.core[i][EXEC_LOC]^077777u)+1u)) {
                                hit=1; expected_bank=state.core[i][EXEC_BANK]; break;
                            }
                        if(hit && physical_bank(expected_bank)) {
                            current.location=(agc_word)loc;
                            current.bank=(agc_word)(get(RegBB)|(unsigned)machine.OutputChannel7);
                            kind=EXEC_CONTEXT_INTERPRETIVE;
                            break;
                        }
                    }
                    if(transfers) {
                        unsigned expected_bank=0;
                        int hit=0;
                        if(e.operation==IN_EXIT && pc==e.address.location) {
                            hit=1; expected_bank=e.address.bank;
                        }
                        for(i=e.operation==IN_START ? 0u : 1u;i<EXEC_CORE_COUNT;++i)
                            if(state.core[i][EXEC_PRIORITY]>0 &&
                               state.core[i][EXEC_PRIORITY]<040000 &&
                               (state.core[i][EXEC_LOC]&040000)==0 &&
                               pc==state.core[i][EXEC_LOC]) {
                                hit=1; expected_bank=state.core[i][EXEC_BANK]; break;
                            }
                        if(hit && physical_bank(expected_bank)) {
                            current.location=(agc_word)pc;
                            current.bank=(agc_word)(get(RegBB)|(unsigned)machine.OutputChannel7);
                            kind=EXEC_CONTEXT_BASIC; break;
                        }
                    }
                }
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==4000) {
                fprintf(stderr,"mixed %u/%u timeout PC=%05o BB=%05o SB=%03o\n",
                    n,step,pc,get(RegBB),machine.OutputChannel7); return 2;
            }
            read_state(&state);
            control.intbit15=(agc_word)get(0115); control.edop=(agc_word)get(RegEDOP);
            in_row(n,step,&state,status,current,kind,control);
            fprintf(stderr,"mixed %u/%u op=%u %u cycles PC=%05o BB=%05o SB=%03o status=%u kind=%u\n",
                n,step,(unsigned)e.operation,cycles,pc,get(RegBB),machine.OutputChannel7,
                (unsigned)status,(unsigned)kind);
        }
    }
    return 0;
}
