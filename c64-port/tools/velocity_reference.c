/* SPDX-License-Identifier: GPL-2.0-or-later
 * Genuine unchanged updated-velocity group and RODCOMP; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "velocity_cases.h"
static agc_t baseline;
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static void emit(velocity_case *c,uint16_t n,uint8_t step,uint8_t stage) {
    uint16_t w[VL_WORD_COUNT]; unsigned i;
    for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
    w[41]=(uint16_t)get(0120); w[42]=(uint16_t)get(0166);
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(c->previous.previous.previous.previous.push.base+i);
    w[86]=(uint16_t)get(01234); w[87]=(uint16_t)get(01235); w[88]=(uint16_t)get(0130);
    w[89]=(uint16_t)get(0131); w[90]=(uint16_t)get(0136); w[91]=(uint16_t)get(0137); w[92]=(uint16_t)get(0140);
    for(i=0;i<6;++i) w[93+i]=(uint16_t)get(01236+i);
    for(i=0;i<6;++i) { w[99+i]=(uint16_t)get(02630+i); w[105+i]=(uint16_t)get(03526+i); w[111+i]=(uint16_t)get(03520+i); }
    vl_from_words(c,w); printf("%u %u %u",n,step,stage);
    for(i=0;i<VL_WORD_COUNT;++i) printf(" %u",w[i]);
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
    for(n=0;n<VL_CASE_COUNT;++n) {
        velocity_case c; uint8_t step;
        vl_prepare(&c,n); machine=baseline;
        for(step=0;step<vl_steps(n);++step) {
            static const unsigned guards[15]={03643,03646,03760,03774,0153,01233,01244,0132,0133,0134,0141,02627,02636,03517,03534};
            uint16_t w[VL_WORD_COUNT]; unsigned i,pc=0,cycles,entered=0,loaded=0,prefix_emitted=0;

            unsigned base=c.previous.previous.previous.previous.push.base;
            vl_next(&c,n,step); vl_words(&c,w);
            for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
            set(0120,base); set(0166,w[42]);
            for(i=0;i<P66_VAC_WORDS;++i) set(base+i,w[43+i]);
            set(01234,w[86]); set(01235,w[87]); set(0130,w[88]);
            for(i=0;i<6;++i) { set(01236+i,w[93+i]); set(02630+i,w[99+i]); set(03526+i,w[105+i]); set(03520+i,w[111+i]); }
            set(0131,w[89]); set(0136,w[90]); set(0137,w[91]); set(0140,w[92]);
            for(i=0;i<15;++i) set(guards[i],012345u+i);
            set(base-1u,054321); set(base+P66_VAC_WORDS,023456);
            set(RegFB,064000); set(RegQ,01030); set(067,0);
            if(vl_integrated(n)) {
                set(RegEB,03400); set(RegBB,064007); set(RegZ,03275); machine.AllowInterrupt=1;
            } else {
                set(RegFB,064000); set(RegEB,01000); set(RegBB,064002); set(RegZ,06061);
                set(0116,0); set(0115,040000); set(0164,03364); set(0165,064007); set(023,0);
                machine.AllowInterrupt=0;
            }
            for(cycles=0;cycles<5000;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(pc==06061 && get(0164)==03364 && get(0163)==077776 && vl_integrated(n) && !prefix_emitted) { emit(&c,n,step,0); prefix_emitted=1; }
                    if(pc==06747 && !entered) {
                        if(get(0163)!=077776 || get(0166)!=base+8u || get(0116)!=01630 || get(RegEB)!=02400) return 2;
                        entered=1;
                    }
                    if(pc==06061 && entered && get(0164)==03373) { loaded=1; break; }
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==5000 || !loaded || get(RegBB)!=064007 || get(0163)!=077776 ||
               get(0166)!=base || machine.AllowInterrupt!=(vl_integrated(n)?1:0) || get(7)!=0 || get(023)!=0 || get(0116)!=01520 || get(0164)!=03373 || get(RegFB)!=064000 || get(RegEB)!=03400) {
                fprintf(stderr,"case %u/%u: boundary failure at %05o BB %05o LOC %05o addr %05o Q %05o\n",n,step,pc,get(RegBB),get(0164),get(0116),get(RegQ)); return 2;
            }
            for(i=0;i<15;++i) if(get(guards[i])!=012345u+i) return 2;
            if(get(base-1u)!=054321 || get(base+P66_VAC_WORDS)!=023456) return 2;
            emit(&c,n,step,1);
            if(n<VL_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, velocity STOVL24D/R -> DANZIG before UNIT; integrated %u\n",n,step,cycles,vl_integrated(n));
        }
    }
    return 0;
}
