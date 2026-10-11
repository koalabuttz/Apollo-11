/* SPDX-License-Identifier: GPL-2.0-or-later
 * Actual unchanged RODCOMP executions. Input constructors and Virtual AGC only;
 * no native preparation, numerical primitive or TS implementation linked. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "prepare_cases.h"
static agc_t baseline;
static const unsigned addresses[PP_WORD_COUNT]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773
};
static const unsigned ts_pc[6]={03317,03322,03325,03331,03335,03341};
static const unsigned guards[6]={03643,03646,03760,03774,0153,0164};
static void load_case(const prepare_case *c) {
    uint16_t words[PP_WORD_COUNT]; unsigned i;
    pp_words(c,words);
    for(i=0;i<PP_WORD_COUNT;++i) *memory(addresses[i])=(int16_t)words[i];
}
static void read_case(prepare_case *c) {
    uint16_t words[PP_WORD_COUNT]; unsigned i;
    for(i=0;i<PP_WORD_COUNT;++i) {
        unsigned mask=i==4 || i==5 ? 0177777u : 077777u;
        words[i]=(uint16_t)((unsigned)*memory(addresses[i]) & mask);
    }
    pp_from_words(c,words);
}
int main(int argc,char **argv) {
    uint16_t n;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    for(n=0;n<PP_CASE_COUNT;++n) {
        prepare_case c; uint8_t step;
        pp_prepare(&c,n); machine=baseline;
        for(step=0;step<pp_steps(n);++step) {
            uint8_t stage; unsigned i;
            pp_next(&c,n,step); load_case(&c);
            for(i=0;i<6;++i) set(guards[i],012345u+i);
            set(RegFB,064000); set(RegEB,03400); set(RegBB,064007);
            set(RegQ,01030);
            set(RegZ,n<144u ? 03303 : 03275);
            machine.AllowInterrupt=n<144u ? 0 : 1;
            for(stage=n<144u ? 1 : 0;stage<3;++stage) {
                static const unsigned stops[3]={03303,03315,03353};
                unsigned cycles,pc=0,pending_ts=0;
                uint8_t mask=0;
                for(cycles=0;cycles<250;++cycles) {
                    pc=get(RegZ);
                    if(!machine.PendFlag && !machine.ExtraDelay) {
                        if(pending_ts) {
                            if(pc==pending_ts+2u) {
                                for(i=0;i<6;++i) if(ts_pc[i]==pending_ts) mask=(uint8_t)(mask | (uint8_t)(1u<<i));
                            } else if(pc!=pending_ts+1u) return 2;
                            pending_ts=0;
                        }
                        if(pc==stops[stage]) break;
                        for(i=0;i<6;++i) if(pc==ts_pc[i]) pending_ts=pc;
                    }
                    machine.ScalerCounter=0;
                    if(agc_engine(&machine)!=0) return 2;
                }
                if(cycles==250 || get(RegBB)!=064007 || machine.AllowInterrupt) {
                    fprintf(stderr,"case %u/%u stage %u failed boundary at %05o\n",n,step,stage,pc); return 2;
                }
                for(i=0;i<6;++i) if(get(guards[i])!=012345u+i) return 2;
                read_case(&c);
                {
                    uint16_t words[PP_WORD_COUNT]; pp_words(&c,words);
                    printf("%u %u %u %u",n,step,stage,mask);
                    for(i=0;i<PP_WORD_COUNT;++i) printf(" %u",words[i]);
                    puts("");
                }
                if(n<PP_TARGET_COUNT) fprintf(stderr,"case %u/%u stage %u: %u AGC cycles, stop 32,%04o, skip %02x\n",n,step,stage,cycles,pc,mask);
            }
        }
    }
    return 0;
}
