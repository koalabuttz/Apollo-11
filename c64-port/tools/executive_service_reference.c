/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only adapter. Runs unchanged SPVAC and CHANG2 in the pinned engine.
 * Input constructors contain no native Executive implementation. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "executive_service_cases.h"
static void load_state(const executive *s) {
    unsigned i,j;
    for (i=0;i<EXEC_CORE_COUNT;++i)
        for (j=0;j<EXEC_CORE_WORDS;++j) set(0154+i*12+j,s->core[i][j]);
    for (i=0;i<EXEC_VAC_COUNT;++i)
        for (j=0;j<EXEC_VAC_WORDS;++j) set(0400+i*44+j,s->vac[i][j]);
    set(0067,s->newjob); set(0063,s->newprio);
    set(0065,s->newloc[0]); set(0066,s->newloc[1]); set(0064,s->locctr);
    set(0120,s->fixloc); set(0121,s->ovfind);
}
static void read_state(executive *s) {
    unsigned i,j;
    for (i=0;i<EXEC_CORE_COUNT;++i)
        for (j=0;j<EXEC_CORE_WORDS;++j) s->core[i][j]=(agc_word)get(0154+i*12+j);
    for (i=0;i<EXEC_VAC_COUNT;++i)
        for (j=0;j<EXEC_VAC_WORDS;++j) s->vac[i][j]=(agc_word)get(0400+i*44+j);
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

int main(int argc,char **argv) {
    unsigned n;
    if(argc!=2) return 2;
    InhibitAlarms=1;
    for(n=0;n<EXEC_SERVICE_CASE_COUNT;++n) {
        exec_service_case c;
        exec_transfer transfer={{0,0},0};
        enum exec_status status=EXEC_OK;
        unsigned cycles,pc=0,spvac_q=0;
        exec_service_prepare(&c,n);
        memset(&machine,0,sizeof machine);
        if(agc_load_binfile(&machine,argv[1])!=0) return 2;
        load_state(&c.state);
        set(0115,c.control.intbit15); set(RegEDOP,c.control.edop);
        bank(n<EXEC_SERVICE_SPVAC_COUNT ? 06000 :
             (unsigned)(c.state.core[0][EXEC_BANK] | c.superbank));
        if(n<EXEC_SERVICE_SPVAC_COUNT) {
            machine.Erasable[0][RegA]=(int16_t)SignExtend(c.address.location);
            machine.Erasable[0][RegL]=(int16_t)SignExtend(c.address.bank);
            set(RegQ,c.q); set(RegZ,05116);
        } else {
            unsigned slot=c.state.newjob/EXEC_CORE_WORDS;
            transfer.address.location=c.state.core[slot][EXEC_LOC];
            transfer.address.bank=c.state.core[slot][EXEC_BANK];
            transfer.interpretive=(uint8_t)((transfer.address.location&040000)!=0);
            if(transfer.interpretive)
                transfer.address.location=(agc_word)((transfer.address.location^077777)+1u);
            set(RegZ,05126);
        }
        machine.AllowInterrupt=0;
        for(cycles=0;cycles<4000;++cycles) {
            pc=get(RegZ);
            if(!machine.PendFlag && !machine.ExtraDelay) {
                if(n<EXEC_SERVICE_SPVAC_COUNT && pc==05112) spvac_q=get(RegQ);
                if(n<EXEC_SERVICE_SPVAC_COUNT && pc==05160) break;
                if(n<EXEC_SERVICE_SPVAC_COUNT && pc==05716) {
                    unsigned q=get(RegQ);
                    unsigned alarm=(unsigned)machine.Fixed[1][q&01777]&077777u;
                    c.state.alarm=(agc_word)alarm; status=(enum exec_status)alarm;
                    break;
                }
                if(n>=EXEC_SERVICE_SPVAC_COUNT &&
                   pc==(transfer.interpretive ? 06070u : transfer.address.location)) {
                    if(get(RegBB)!=(transfer.address.bank&076007u) ||
                       (unsigned)machine.OutputChannel7!=(transfer.address.bank&0160u)) {
                        fprintf(stderr,"bank mismatch case %u\n",n); return 2;
                    }
                    break;
                }
            }
            if(agc_engine(&machine)!=0) return 2;
        }
        if(cycles==4000) { fprintf(stderr,"case %u timeout at %05o\n",n,pc); return 2; }
        {
            agc_word alarm=c.state.alarm;
            read_state(&c.state); c.state.alarm=alarm;
        }
        /* Q is ABI scratch for CHANG2; SPVAC's adjusted return is a contract. */
        if(n<EXEC_SERVICE_SPVAC_COUNT) c.q=(agc_word)spvac_q;
        c.control.intbit15=(agc_word)get(0115); c.control.edop=(agc_word)get(RegEDOP);
        exec_service_row(n,&c,status,transfer);
        fprintf(stderr,"case %u %u cycles stop %05o\n",n,cycles,pc);
    }
    return 0;
}
