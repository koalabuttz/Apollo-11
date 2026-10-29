/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only adapter: unchanged VXSC/DMP1 and DMPSUB/VROUND assembly, no native math. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "product_cases.h"
static const unsigned addresses[50]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121,
    01234,01235,0130,01270,01271,01272,01273,01274,01275
};
int main(int argc,char **argv) {
    agc_t baseline; uint16_t n; unsigned sub,round_skips[3]={0},round_noskips=0;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    for(n=0;n<PR_CASE_COUNT;++n) for(sub=0;sub<2;++sub) {
        product_case c; uint16_t words[VE_WORD_COUNT];
        unsigned pc=0,cycles,i,pending_ts=0,prior_low_carry=round_skips[1];
        static const unsigned guards[8]={0153,0164,0165,0166,01267,01276,0131,0132};
        pr_prepare(&c,n,(uint8_t)sub); ve_words(&c,words); machine=baseline;
        for(i=0;i<VE_WORD_COUNT;++i) *memory(addresses[i])=(int16_t)words[i];
        for(i=0;i<8;++i) set(guards[i],012345u+i);
        set(0116,01270); set(RegEB,01000); set(RegFB,064000); set(RegBB,064002);
        set(RegQ,01030); set(RegZ,sub ? 07401 : 07572); machine.AllowInterrupt=0;
        for(cycles=0;cycles<1000;++cycles) {
            pc=get(RegZ);
            if(!machine.PendFlag && !machine.ExtraDelay) {
                if(pending_ts) {
                    if(pc==pending_ts+2u) ++round_skips[pending_ts==07142 ? 0u : pending_ts==07145 ? 1u : 2u];
                    else if(pc==pending_ts+1u) { if(pending_ts==07142) ++round_noskips; }
                    else return 2;
                    pending_ts=0;
                }
                if(pc==07142 || pc==07145 || pc==07150) pending_ts=pc;
                if(pc==06061) break;
            }
            machine.ScalerCounter=0;
            if(agc_engine(&machine)!=0) return 2;
        }
        if(cycles==1000 || get(RegBB)!=064002 || get(0163)!=(sub ? 077776u : 0u) || machine.AllowInterrupt || get(7)!=0) {
            fprintf(stderr,"case %u/%u boundary failure %05o\n",n,sub,pc); return 2;
        }
        for(i=0;i<8;++i) if(get(guards[i])!=012345u+i) return 2;
        printf("%u %u",n,sub);
        for(i=0;i<VE_WORD_COUNT;++i) {
            words[i]=(uint16_t)((unsigned)*memory(addresses[i])&(i==4 || i==5 ? 0177777u : 077777u));
            printf(" %u",words[i]);
        }
        puts("");
        if(round_skips[1]!=prior_low_carry) fprintf(stderr,"low rounding carry case %u/%u\n",n,sub);
        if(n<400u) fprintf(stderr,"case %u/%u: %u cycles %s -> DANZIG\n",n,sub,cycles,sub ? "VXSC" : "DMP1");
    }
    fprintf(stderr,"VROUND first TS no-skip %u, round-bit/low-carry/high-overflow skips %u/%u/%u\n",round_noskips,round_skips[0],round_skips[1],round_skips[2]);
    if(!round_noskips || !round_skips[0] || !round_skips[1]) return 2;
    return 0;
}
