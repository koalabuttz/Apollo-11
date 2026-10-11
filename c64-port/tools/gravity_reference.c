/* SPDX-License-Identifier: GPL-2.0-or-later
 * Genuine unchanged PDVL GDT/2 and RODCOMP; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "gravity_cases.h"
static agc_t baseline;
static unsigned char coverage[5][42];
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static void emit(gravity_case *c,uint16_t n,uint8_t step,uint8_t stage) {
    uint16_t w[GV_WORD_COUNT]; unsigned i;
    for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
    w[41]=(uint16_t)get(0120); w[42]=(uint16_t)get(0166);
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(c->previous.previous.previous.push.base+i);
    w[86]=(uint16_t)get(01234); w[87]=(uint16_t)get(01235); w[88]=(uint16_t)get(0130);
    w[89]=(uint16_t)get(0131); w[90]=(uint16_t)get(0136); w[91]=(uint16_t)get(0137); w[92]=(uint16_t)get(0140);
    for(i=0;i<6;++i) w[93+i]=(uint16_t)get(01236+i);
    gv_from_words(c,w); printf("%u %u %u",n,step,stage);
    for(i=0;i<GV_WORD_COUNT;++i) printf(" %u",w[i]);
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
    for(n=0;n<GV_CASE_COUNT;++n) {
        gravity_case c; uint8_t step;
        gv_prepare(&c,n); machine=baseline;
        for(step=0;step<gv_steps(n);++step) {
            static const unsigned guards[11]={03643,03646,03760,03774,0153,01233,01244,0132,0133,0134,0141};
            uint16_t w[GV_WORD_COUNT]; unsigned i,pc=0,cycles,entered=0,divided=0,loaded=0;

            unsigned initial_push=c.previous.previous.previous.push.pushloc,base=c.previous.previous.previous.push.base;
            gv_next(&c,n,step); gv_words(&c,w);
            for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
            set(0120,base); set(0166,initial_push);
            for(i=0;i<P66_VAC_WORDS;++i) set(base+i,w[43+i]);
            set(01234,w[86]); set(01235,w[87]); set(0130,w[88]);
            for(i=0;i<6;++i) set(01236+i,w[93+i]);
            set(0131,w[89]); set(0136,w[90]); set(0137,w[91]); set(0140,w[92]);
            for(i=0;i<11;++i) set(guards[i],012345u+i);
            set(base-1u,054321); set(base+P66_VAC_WORDS,023456);
            set(RegFB,064000); set(RegQ,01030); set(067,0);
            if(gv_integrated(n)) {
                set(RegEB,03400); set(RegBB,064007); set(RegZ,03275); machine.AllowInterrupt=1;
            } else {
                set(RegFB,064000); set(RegEB,01000); set(RegBB,064002); set(RegZ,06557);
                set(0116,01636); set(0164,03364); set(0165,064007); set(023,0);
                machine.AllowInterrupt=0;
            }
            for(cycles=0;cycles<2500;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(pc==07577) divided=1;
                    if(pc==06061 && divided && !entered) { emit(&c,n,step,0); divided=0; }
                    if(pc==06557) {
                        if(get(0116)!=01636 || get(RegFB)!=064000 || get(RegEB)!=01000 || get(0163)!=0) { fprintf(stderr,"PDVL entry %u/%u addr %05o FB %05o EB %05o MODE %05o\n",n,step,get(0116),get(RegFB),get(RegEB),get(0163)); return 2; }
                        coverage[(base-0401u)/054u][get(0166)-base]=1;
                        entered=loaded=1;
                    }
                    if(pc==06061 && loaded) break;
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==2500 || !loaded || get(RegBB)!=064002 || get(0163)!=077776 ||
               get(0166)!=initial_push+(gv_integrated(n)?8u:2u) ||
               machine.AllowInterrupt!=(gv_integrated(n)?1:0) || get(7)!=0 || get(0116)!=01636 || get(0164)!=03364 || get(RegFB)!=064000 || get(RegEB)!=01000) {
                fprintf(stderr,"case %u/%u: boundary failure at %05o BB %05o LOC %05o addr %05o Q %05o\n",n,step,pc,get(RegBB),get(0164),get(0116),get(RegQ)); return 2;
            }
            for(i=0;i<11;++i) if(get(guards[i])!=012345u+i) return 2;
            if(get(base-1u)!=054321 || get(base+P66_VAC_WORDS)!=023456) return 2;
            emit(&c,n,step,1);
            if(n<GV_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, PDVL GDT/2 -> DANZIG before VSU; integrated %u\n",n,step,cycles,gv_integrated(n));
        }
    }
    {
        unsigned vac,offset;
        for(vac=0;vac<5;++vac) for(offset=0;offset<42;++offset) if(!coverage[vac][offset]) return 2;
    }
    fprintf(stderr,"All five VACs at all 42 legal DP push offsets verified; guards retained.\n");
    return 0;
}
