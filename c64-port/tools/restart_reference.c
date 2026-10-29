/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only ORIGINAL assembly adapter. Links engine and input constructors only.
 * Peripheral inputs are explicit; no native recovery code is linked. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "restart_cases.h"
#include "restart_addresses.h"
static void load_case(const restart_case *c) {
    unsigned i,j;
    for(i=0;i<8;++i) for(j=0;j<12;++j) set(0154+i*12+j,c->exec.core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) set(0400+i*44+j,c->exec.vac[i][j]);
    set(0067,c->exec.newjob); set(0063,c->exec.newprio); set(0065,c->exec.newloc[0]); set(0066,c->exec.newloc[1]);
    set(0064,c->exec.locctr); set(0120,c->exec.fixloc); set(0121,c->exec.ovfind);
    set(026,c->wait.time3); set(0734,c->wait.ruptagn);
    for(i=0;i<8;++i) set(01400+i,c->wait.interval[i]);
    for(i=0;i<9;++i) { set(01410+i*2,c->wait.task[i].location); set(01411+i*2,c->wait.task[i].bank); }
    for(i=0;i<3;++i) set(0375+i,c->record.failreg[i]);
    for(i=0;i<2;++i) set(01363+i,c->record.almcadr[i]);
    for(i=0;i<12;++i) set(01023+i,c->record.dsptab[i]);
    for(i=0;i<RS_WORD_COUNT;++i) set(rs_addresses[i],c->record.word[i]);
    machine.InputChannel[011]=c->record.channel11; machine.InputChannel[012]=c->record.channel12;
    machine.Trap31A=c->record.trap31a; machine.Trap31B=c->record.trap31b; machine.Trap32=c->record.trap32;
    machine.InputChannel[013]=c->record.channel13; machine.InputChannel[014]=c->record.channel14;
    for(i=0;i<6;++i) {
        set(0752+i*2,c->phases.complement[i]); set(0753+i*2,c->phases.phase[i]);
        set(01436+i*2,c->phases.address[i].location); set(01437+i*2,c->phases.address[i].bank);
        set(01054+i*2,c->phases.priority[i]);
    }
}
static void read_case(restart_case *c) {
    unsigned i,j;
    for(i=0;i<8;++i) for(j=0;j<12;++j) c->exec.core[i][j]=(agc_word)get(0154+i*12+j);
    for(i=0;i<5;++i) for(j=0;j<44;++j) c->exec.vac[i][j]=(agc_word)get(0400+i*44+j);
    c->exec.newjob=(agc_word)get(0067); c->exec.newprio=(agc_word)get(0063);
    c->exec.newloc[0]=(agc_word)get(0065); c->exec.newloc[1]=(agc_word)get(0066);
    c->exec.locctr=(agc_word)get(0064); c->exec.fixloc=(agc_word)get(0120); c->exec.ovfind=(agc_word)get(0121);
    c->wait.time3=(agc_word)get(026); c->wait.ruptagn=(agc_word)get(0734);
    for(i=0;i<8;++i) c->wait.interval[i]=(agc_word)get(01400+i);
    for(i=0;i<9;++i) { c->wait.task[i].location=(agc_word)get(01410+i*2); c->wait.task[i].bank=(agc_word)get(01411+i*2); }
    for(i=0;i<3;++i) c->record.failreg[i]=(agc_word)get(0375+i);
    for(i=0;i<2;++i) c->record.almcadr[i]=(agc_word)get(01363+i);
    for(i=0;i<12;++i) c->record.dsptab[i]=(agc_word)get(01023+i);
    for(i=0;i<RS_WORD_COUNT;++i) c->record.word[i]=(agc_word)get(rs_addresses[i]);
    c->record.channel11=(agc_word)machine.InputChannel[011]; c->record.channel12=(agc_word)machine.InputChannel[012];
    c->record.channel13=(agc_word)machine.InputChannel[013]; c->record.channel14=(agc_word)machine.InputChannel[014];
    c->record.trap31a=(agc_word)machine.Trap31A; c->record.trap31b=(agc_word)machine.Trap31B; c->record.trap32=(agc_word)machine.Trap32;
    for(i=0;i<6;++i) {
        c->phases.complement[i]=(agc_word)get(0752+i*2); c->phases.phase[i]=(agc_word)get(0753+i*2);
        c->phases.address[i].location=(agc_word)get(01436+i*2); c->phases.address[i].bank=(agc_word)get(01437+i*2);
        c->phases.priority[i]=(agc_word)get(01054+i*2);
    }
}
static void print_word(void *user,agc_word word) { (void)user; printf(" %u",word); }
int main(int argc,char **argv) {
    unsigned n;
    if(argc!=2) return 2;
    for(n=0;n<RS_CASE_COUNT;++n) {
        restart_case c; unsigned step;
        memset(&machine,0,sizeof machine); if(agc_load_binfile(&machine,argv[1])!=0) return 2;
        rs_prepare(&c,(uint16_t)n); load_case(&c); InhibitAlarms=1;
        for(step=0;step<rs_steps((uint16_t)n);++step) {
            unsigned cycles,pc=0,operation=n<96 ? 0 : n<112 ? 1 : n<138 ? 2 : 3;
            int status=0;
            if(n>=186) operation=step==0 ? 4 : step==1 ? 0 : step==2 ? 1 : step==3 ? 2 : 3;
            set(RegBB,06005); set(RegFB,06000); set(RegEB,02400); machine.InputChannel[7]=machine.OutputChannel7=0100;
            set(RegA,0); set(RegL,0); set(RegQ,01010); set(RegZ,0); machine.AllowInterrupt=0;
            if(operation==0) {
                unsigned entry=n>=186 ? 1 : n/32;
                if(n>=186) c.code=c.exec.alarm;
                machine.Erasable[0][RegA]=(int16_t)SignExtend(c.caller.location); set(RegL,c.caller.bank);
                set(01010,c.code); set(RegZ,entry==0 ? 05732 : entry==1 ? 05716 : 05726);
            } else if(operation==1) { set(RegFB,012000); set(RegZ,03116); }
            else if(operation==2) { set(RegFB,012000); set(RegZ,02775); }
            else if(operation==3) {
                unsigned group=n>=186 ? 9-step : (n-138)/8;
                set(0154,c.phases.phase[group]); set(0161,group); set(RegFB,02000); set(RegEB,01400); set(RegZ,03526);
            } else {
                machine.Erasable[0][RegA]=(int16_t)SignExtend(03000);
                set(01010,03200); set(01011,06005); set(RegZ,05105);
            }
            for(cycles=0;cycles<8000;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(operation==0 && (pc==01011 || pc==05644 || pc==05661)) break;
                    if(operation==1 && pc==01010) { c.exec.alarm=0; c.wait.alarm=0; break; }
                    if(operation==2 && pc==05315) break;
                    if(operation==2 && pc==03054 && get(RegFB)==012000) { status=RS_PHASE_BAD; break; }
                    if((operation==3 || operation==4) && pc==05160) break;
                    if((operation==3 || operation==4) && pc==05716) {
                        status=(int)((unsigned)machine.Fixed[1][get(RegQ)&01777] & 077777u);
                        c.exec.alarm=(agc_word)status; break;
                    }
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==8000) { fprintf(stderr,"restart %u/%u timeout PC=%05o BB=%05o\n",n,step,pc,get(RegBB)); return 2; }
            read_case(&c); printf("%u %u %d",n,step,status); rs_words(&c,print_word,0); putchar('\n');
            fprintf(stderr,"restart %u/%u: %u AGC cycles PC=%05o BB=%05o SB=%03o status=%d\n",n,step,cycles,pc,get(RegBB),machine.OutputChannel7,status);
        }
    }
    return 0;
}
