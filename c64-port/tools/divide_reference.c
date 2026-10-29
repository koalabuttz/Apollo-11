/* SPDX-License-Identifier: GPL-2.0-or-later
 * Genuine unchanged DDV 4SEC(28) and RODCOMP; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "divide_cases.h"
static agc_t baseline;
static unsigned results[3];
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static void emit(divide_case *c,uint16_t n,uint8_t step,uint8_t stage,uint8_t path) {
    uint16_t w[DV_WORD_COUNT]; unsigned i;
    for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
    w[41]=(uint16_t)get(0120); w[42]=(uint16_t)get(0166);
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(c->previous.previous.push.base+i);
    w[86]=(uint16_t)get(01234); w[87]=(uint16_t)get(01235); w[88]=(uint16_t)get(0130);
    w[89]=(uint16_t)get(0131); w[90]=(uint16_t)get(0136); w[91]=(uint16_t)get(0137); w[92]=(uint16_t)get(0140);
    dv_from_words(c,w); printf("%u %u %u %u",n,step,stage,path);
    for(i=0;i<DV_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(int argc,char **argv) {
    uint16_t n;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    if((unsigned)machine.Fixed[033][01152]!=0 || (unsigned)machine.Fixed[033][01153]!=00620) return 2;
    if((unsigned)machine.Fixed[02][0733]!=037777u ||
       (unsigned)machine.Fixed[02][0735]!=040000u) return 2;
    for(n=0;n<DV_CASE_COUNT;++n) {
        divide_case c; uint8_t step;
        dv_prepare(&c,n); machine=baseline;
        for(step=0;step<dv_steps(n);++step) {
            static const unsigned guards[11]={03643,03646,03760,03774,0153,01233,01236,0132,0133,0134,0141};
            uint16_t w[DV_WORD_COUNT]; unsigned i,pc=0,cycles,entered=0,stored=0,divided=0;
            uint8_t path=0;
            unsigned initial_push=c.previous.previous.push.pushloc,base=c.previous.previous.push.base;
            dv_next(&c,n,step); dv_words(&c,w);
            for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
            set(0120,base); set(0166,initial_push);
            for(i=0;i<P66_VAC_WORDS;++i) set(base+i,w[43+i]);
            set(01234,w[86]); set(01235,w[87]); set(0130,w[88]);
            set(0131,w[89]); set(0136,w[90]); set(0137,w[91]); set(0140,w[92]);
            for(i=0;i<11;++i) set(guards[i],012345u+i);
            set(base-1u,054321); set(base+P66_VAC_WORDS,023456);
            set(RegFB,064000); set(RegQ,01030); set(067,0);
            if(dv_integrated(n)) {
                set(RegEB,03400); set(RegBB,064007); set(RegZ,03275); machine.AllowInterrupt=1;
            } else {
                set(RegFB,066000); set(RegEB,03400); set(RegBB,066007); set(RegZ,07577);
                set(0116,03152); set(0164,03363); set(0165,064007); set(023,0);
                machine.AllowInterrupt=0;
            }
            for(cycles=0;cycles<2500;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(pc==06422) stored=1;
                    if(pc==06061 && stored && !divided) { emit(&c,n,step,0,0); stored=0; }
                    if(pc==07577) {
                        if(get(0116)!=03152 || get(RegFB)!=066000 || get(RegEB)!=03400 || get(0163)!=0) return 2;
                        entered=divided=1;
                    }
                    if(pc==02405 && entered) path=2;
                    if(pc==02505 && entered) path=1;
                    if(pc==06061 && divided) break;
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==2500 || !divided || get(RegBB)!=7 || get(0163)!=0 ||
               get(0166)!=initial_push+(dv_integrated(n)?6u:0u) ||
               machine.AllowInterrupt!=(dv_integrated(n)?1:0) || get(7)!=0 || get(0116)!=03152 || get(0164)!=03363 || get(RegFB)!=0 || get(RegEB)!=03400 ||
               get(RegQ)!=(path==0 ? 2u : path==1 ? 3u : 02413u)) {
                fprintf(stderr,"case %u/%u: boundary failure at %05o BB %05o LOC %05o addr %05o Q %05o\n",n,step,pc,get(RegBB),get(0164),get(0116),get(RegQ)); return 2;
            }
            for(i=0;i<11;++i) if(get(guards[i])!=012345u+i) return 2;
            if(get(base-1u)!=054321 || get(base+P66_VAC_WORDS)!=023456) return 2;
            ++results[path]; emit(&c,n,step,1,path);
            if(n<DV_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, DDV 4SEC(28) -> DANZIG before PDVL; path %u, integrated %u\n",n,step,cycles,path,dv_integrated(n));
        }
    }
    fprintf(stderr,"paths zero/quotient/overflow = %u/%u/%u\n",results[0],results[1],results[2]);
    if(!results[0] || !results[1] || !results[2]) return 2;
    return 0;
}
