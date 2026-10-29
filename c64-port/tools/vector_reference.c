/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only adapter: unchanged VAD/VSU and OVERFLOW assembly, no native math. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "vector_cases.h"
static const unsigned addresses[50]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121,
    01234,01235,0130,01270,01271,01272,01273,01274,01275
};
static unsigned shift(unsigned offset) { return offset==3u ? 3u : offset==5u ? 6u : 0u; }
int main(int argc,char **argv) {
    agc_t baseline; uint16_t n; unsigned sub,coverage[3][8]={{0}};
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    for(n=0;n<VE_CASE_COUNT;++n) for(sub=0;sub<2;++sub) {
        vector_case c; uint16_t words[VE_WORD_COUNT],path=0;
        unsigned pc=0,cycles,i,pending_ts=0,component_shift=0;
        static const unsigned guards[8]={0153,0164,0165,0166,01267,01276,0131,0132};
        ve_prepare(&c,n,(uint8_t)sub); ve_words(&c,words); machine=baseline;
        for(i=0;i<VE_WORD_COUNT;++i) *memory(addresses[i])=(int16_t)words[i];
        for(i=0;i<8;++i) set(guards[i],012345u+i);
        set(0116,01270); set(RegEB,01000); set(RegFB,064000); set(RegBB,064002);
        set(RegQ,01030); set(RegZ,sub ? 06747 : 06751); machine.AllowInterrupt=0;
        for(cycles=0;cycles<1000;++cycles) {
            pc=get(RegZ);
            if(!machine.PendFlag && !machine.ExtraDelay) {
                if(pending_ts) {
                    if(pc==pending_ts+2u) path=(uint16_t)(path | ((pending_ts==07026 ? 2u : 4u)<<component_shift));
                    else if(pc!=pending_ts+1u) return 2;
                    pending_ts=0;
                }
                if(pc==07014) path=(uint16_t)(path | (1u<<3));
                if(pc==07011) path=(uint16_t)(path | (1u<<6));
                if(pc==07003) path=(uint16_t)(path | 1u);
                if(pc==07026 || pc==07033) { pending_ts=pc; component_shift=shift(get(RegL)); }
                if(pc==06061) break;
            }
            machine.ScalerCounter=0;
            if(agc_engine(&machine)!=0) return 2;
        }
        if(cycles==1000 || get(RegBB)!=064002 || get(0163)!=077776 || machine.AllowInterrupt || get(7)!=0) {
            fprintf(stderr,"case %u/%u boundary failure %05o\n",n,sub,pc); return 2;
        }
        for(i=0;i<8;++i) if(get(guards[i])!=012345u+i) return 2;
        printf("%u %u %u",n,sub,path);
        for(i=0;i<VE_WORD_COUNT;++i) {
            words[i]=(uint16_t)((unsigned)*memory(addresses[i])&(i==4 || i==5 ? 0177777u : 077777u));
            printf(" %u",words[i]);
        }
        puts("");
        for(i=0;i<3;++i) ++coverage[i][(path>>(i*3u))&7u];
        if(n<400u) fprintf(stderr,"case %u/%u: %u cycles V%s -> DANZIG, paths %u\n",n,sub,cycles,sub ? "SU" : "AD",path);
    }
    for(sub=0;sub<3;++sub) {
        fprintf(stderr,"component %u paths 0/1/5/7=%u/%u/%u/%u\n",sub,coverage[sub][0],coverage[sub][1],coverage[sub][5],coverage[sub][7]);
        if(!coverage[sub][0] || !coverage[sub][1] || !coverage[sub][5] || !coverage[sub][7]) return 2;
    }
    return 0;
}
