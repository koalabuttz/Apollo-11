/* SPDX-License-Identifier: GPL-2.0-or-later
 * Original WAITLIST/interrupt lead-in/TASKOVER with input-only fixtures.
 * No native Executive, cm_waitlist, dispatcher or arithmetic implementation linked. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_waitlist_cases.h"
extern int CounterPINC(int16_t *counter);
static void load(const cm_waitlist *s) {
    unsigned i;
    set(026,s->time3); set(0734,s->ruptagn);
    for(i=0;i<8;++i) set(01400+i,s->interval[i]);
    for(i=0;i<9;++i) { set(01410+i*2,s->task[i].location); set(01411+i*2,s->task[i].bank); }
}
static void read_wait(cm_waitlist *s) {
    unsigned i;
    s->time3=(agc_word)get(026); s->ruptagn=(agc_word)get(0734);
    for(i=0;i<8;++i) s->interval[i]=(agc_word)get(01400+i);
    for(i=0;i<9;++i) { s->task[i].location=(agc_word)get(01410+i*2); s->task[i].bank=(agc_word)get(01411+i*2); }
}
static void row(unsigned n,unsigned step,const cm_waitlist *s,int status,exec_address next) {
    unsigned i;
    printf("%u %u %d %u",n,step,status,s->time3);
    for(i=0;i<8;++i) printf(" %u",s->interval[i]);
    for(i=0;i<9;++i) printf(" %u %u",s->task[i].location,s->task[i].bank);
    printf(" %u %u %u %u\n",s->ruptagn,s->alarm,next.location,next.bank);
}
int main(int argc,char **argv) {
    unsigned n,step;
    if(argc!=2) return 2;
    for(n=0;n<CM_WAIT_CASE_COUNT;++n) {
        cm_waitlist s;
        memset(&machine,0,sizeof machine);
        if(agc_load_binfile(&machine,argv[1])!=0) return 2;
        cm_wait_prepare(&s,(uint16_t)n); load(&s); InhibitAlarms=1;
        for(step=0;step<cm_wait_steps((uint16_t)n);++step) {
            unsigned cycles=0,pc=0,i;
            cm_wait_event e; exec_address next={0,0}; int status=CM_WL_OK;
            cm_wait_event_prepare(&e,(uint16_t)n,(uint8_t)step);
            set(RegBB,06005); set(RegFB,06000); set(RegEB,02400);
            machine.InputChannel[7]=machine.OutputChannel7=0100;
            set(RegA,0); set(RegL,0); machine.AllowInterrupt=0;
            set(061,012345); set(062,023456); set(063,034567); set(064,045670);
            for(i=0;i<16;++i) set(0300+i,01000+i);
            if(e.operation==CM_WAIT_TICK) {
                for(i=0;i<e.ticks;++i) CounterPINC(memory(026));
                for(i=0;i<16;++i) if(get(0300+i)!=01000+i) { fprintf(stderr,"CM wait guard changed\n"); return 2; }
            read_wait(&s); row(n,step,&s,status,next); continue;
            }
            if(e.operation==CM_WAIT_INSERT) {
                machine.Erasable[0][RegA]=(int16_t)SignExtend(e.delay);
                set(RegQ,01000); set(01000,e.address.location); set(01001,e.address.bank); set(RegZ,05245);
            } else if(e.operation==CM_WAIT_BEGIN) {
                /* Real fixed-fixed T3 lead-in. Its A/L and caller bank are saved. */
                set(RegZ,04014);
            } else {
                set(016,06105); set(012,01007); set(RegBB,02063); set(RegFB,02000); set(RegEB,01400);
                set(RegZ,05324);
            }
            for(cycles=0;cycles<4000;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(e.operation==CM_WAIT_INSERT && pc==05263) break;
                    if(e.operation==CM_WAIT_INSERT && (pc==07755 || pc==05721)) {
                        unsigned q=get(RegQ), bank=q>=06000 ? 3u : q>=04000 ? 2u : get(RegFB)>>10;
                        s.alarm=(agc_word)machine.Fixed[bank][q&01777]; status=s.alarm;
                        if(status==CM_WL_BAD_DELAY && (get(061)!=012345 || get(062)!=023456 || get(063)!=034567 || get(064)!=045670)) { fprintf(stderr,"CM bad delay scratch mutation %u/%u\n",n,step); return 2; }
                        break;
                    }
                    if(e.operation==CM_WAIT_OVER && pc==05340) {
                        if(get(RegBB)!=06005 || (unsigned)machine.OutputChannel7!=0100) {
                            fprintf(stderr,"taskover bank restore failed %u/%u\n",n,step); return 2;
                        }
                        status=CM_WL_IDLE; break;
                    }
                    if(e.operation!=CM_WAIT_INSERT) {
                        for(i=0;i<9;++i) {
                            exec_address target=s.task[i];
                            if(pc==target.location && get(RegBB)==(target.bank & 076007) &&
                               (unsigned)machine.OutputChannel7==(target.bank & 0160)) {
                                next.location=(agc_word)pc; next.bank=(agc_word)(get(RegBB) | (unsigned)machine.OutputChannel7);
                                goto stopped;
                            }
                        }
                    }
                }
                /* Counter progress is explicit fixture input, not oracle CPU time. */
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
stopped:
            if(cycles==4000) { fprintf(stderr,"wait %u/%u timeout PC=%05o BB=%05o\n",n,step,pc,get(RegBB)); return 2; }
            if(e.operation==CM_WAIT_BEGIN && get(016)!=06105) {
                fprintf(stderr,"T3 lead-in bank save failed %u/%u\n",n,step); return 2;
            }
            for(i=0;i<16;++i) if(get(0300+i)!=01000+i) { fprintf(stderr,"CM wait guard changed\n"); return 2; }
            read_wait(&s); row(n,step,&s,status,next);
            fprintf(stderr,"wait %u/%u: %u AGC cycles PC=%05o BB=%05o SB=%03o status=%d\n",n,step,cycles,pc,get(RegBB),machine.OutputChannel7,status);
        }
    }
    return 0;
}
