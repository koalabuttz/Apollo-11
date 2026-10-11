/* SPDX-License-Identifier: GPL-2.0-or-later
 * Original unchanged DOT arithmetic; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "dot_cases.h"
static agc_t baseline;
static unsigned intermediate_overflow[2],final_overflow[2];
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
int main(int argc,char **argv) {
    uint16_t n;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    for(n=0;n<DT_CASE_COUNT;++n) {
        dot_case c; uint16_t w[DT_WORD_COUNT]; unsigned i,cycles,pc=0;
        static const unsigned guards[10]={03643,03646,03760,03774,0153,01233,01236,0133,0134,0141};
        unsigned base,push;
        dt_prepare(&c,n); dt_words(&c,w); machine=baseline;
        base=c.previous.previous.push.base; push=c.previous.previous.push.pushloc;
        for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
        set(0120,base); set(0166,push);
        for(i=0;i<P66_VAC_WORDS;++i) set(base+i,w[43+i]);
        set(01234,w[86]); set(01235,w[87]); set(0130,w[88]);
        set(0131,w[89]);set(0136,w[90]);set(0137,w[91]);set(0140,w[92]);set(0132,w[93]);
        for(i=0;i<6;++i) set(03400u+i,w[94+i]);
        for(i=0;i<10;++i) set(guards[i],012345u+i);
        set(base-1u,054321);set(base+P66_VAC_WORDS,023456);
        set(RegEB,03400); set(RegFB,0); set(RegBB,7); set(RegQ,01030);
        set(7,0); machine.AllowInterrupt=0;
        set(RegFB,066000);set(RegBB,066007);set(0116,01400);set(RegZ,07331);
        for(cycles=0;cycles<5000;++cycles) {
            pc=get(RegZ);
            if(!machine.PendFlag&&!machine.ExtraDelay) {
                if(pc==06061)break;
                if(pc==07177)++intermediate_overflow[get(RegA)==1u?0:1];
                if(pc==07213)++final_overflow[get(RegA)==1u?0:1];
            }
            machine.ScalerCounter=0;
            if(agc_engine(&machine)!=0) return 2;
        }
        if(cycles==5000||get(RegEB)!=03400||get(RegFB)!=066000||get(RegBB)!=066007||get(0116)!=01404||get(0163)!=0||get(0166)!=push||get(0120)!=base||get(7)!=0) {
            fprintf(stderr,"case %u boundary failure %05o cycles%u\n",n,pc,cycles); return 2;
        }
        for(i=0;i<10;++i) if(get(guards[i])!=012345u+i) return 2;
        if(get(base-1u)!=054321||get(base+P66_VAC_WORDS)!=023456||machine.AllowInterrupt) return 2;
        for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i])&(i==4||i==5?0177777u:077777u));
        w[41]=(uint16_t)get(0120);w[42]=(uint16_t)get(0166);
        for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(base+i);
        w[86]=(uint16_t)get(01234);w[87]=(uint16_t)get(01235);w[88]=(uint16_t)get(0130);
        w[89]=(uint16_t)get(0131);w[90]=(uint16_t)get(0136);w[91]=(uint16_t)get(0137);w[92]=(uint16_t)get(0140);w[93]=(uint16_t)get(0132);
        for(i=0;i<6;++i) w[94+i]=(uint16_t)get(03400u+i);
        printf("%u",n);for(i=0;i<DT_WORD_COUNT;++i) printf(" %u",w[i]);puts("");
        if(n<32u) fprintf(stderr,"case%u DOT cycles%u BUF%05o OVFIND%05o A%06o L%06o\n",n,cycles,w[88],w[40],w[4],w[5]);
    }
    fprintf(stderr,"Intermediate positive/negative overflows %u/%u; final positive/negative overflows %u/%u\n",intermediate_overflow[0],intermediate_overflow[1],final_overflow[0],final_overflow[1]);
    if(!intermediate_overflow[0]||!intermediate_overflow[1]||!final_overflow[0]||!final_overflow[1])return 2;
    return 0;
}
