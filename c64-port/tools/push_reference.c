/* SPDX-License-Identifier: GPL-2.0-or-later
 * Genuine unchanged PDDL and RODCOMP executions, input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "push_cases.h"
static agc_t baseline;
static unsigned char coverage[5][38];
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static void emit(push_case *c,uint16_t n,uint8_t step,uint8_t stage) {
    uint16_t w[PS_WORD_COUNT]; unsigned i;
    for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
    w[41]=(uint16_t)get(0120); w[42]=(uint16_t)get(0166);
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(c->push.base+i);
    ps_from_words(c,w); printf("%u %u %u",n,step,stage);
    for(i=0;i<PS_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(int argc,char **argv) {
    uint16_t n;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    for(n=0;n<PS_CASE_COUNT;++n) {
        push_case c; uint8_t step;
        ps_prepare(&c,n); machine=baseline;
        for(step=0;step<ps_steps(n);++step) {
            static const unsigned guards[5]={03643,03646,03760,03774,0153};
            uint16_t w[PS_WORD_COUNT]; unsigned i,pc=0,cycles,entered=0,scaled=0;
            unsigned initial_push=c.push.pushloc;
            ps_next(&c,n,step); ps_words(&c,w);
            for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
            set(0120,c.push.base); set(0166,initial_push);
            coverage[(c.push.base-0401u)/054u][initial_push-c.push.base]=1;
            for(i=0;i<P66_VAC_WORDS;++i) set(c.push.base+i,w[43+i]);
            for(i=0;i<5;++i) set(guards[i],012345u+i);
            set(c.push.base-1u,054321); set(c.push.base+P66_VAC_WORDS,023456);
            set(RegEB,03400); set(RegFB,064000); set(RegBB,064007); set(RegQ,01030);
            set(RegZ,ps_integrated(n) ? 03275 : 06523);
            machine.AllowInterrupt=ps_integrated(n) ? 1 : 0;
            if(!ps_integrated(n)) set(0116,01761);
            for(cycles=0;cycles<2200;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(pc==07401) {
                        if(!ps_integrated(n) || get(0116)!=02021 || get(RegFB)!=066000 || get(0163)!=077776) return 2;
                        scaled=1;
                    }
                    if(pc==06061 && scaled && !entered) { emit(&c,n,step,0); scaled=0; }
                    if(pc==06523) {
                        if(get(0116)!=01761 || get(RegEB)!=03400 || get(RegFB)!=064000 ||
                           get(0163)!=077776 || get(0166)!=initial_push) return 2;
                        if(ps_integrated(n) && (!machine.AllowInterrupt || get(0164)!=03356 || get(0165)!=064007)) return 2;
                        entered=1;
                    }
                    if(pc==06061 && entered) break;
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==2200 || !entered || get(RegBB)!=064007 ||
               get(0166)!=initial_push+6u || get(0163)!=0 ||
               machine.AllowInterrupt!=(ps_integrated(n)?1:0)) {
                fprintf(stderr,"case %u/%u: boundary failure at %05o\n",n,step,pc); return 2;
            }
            for(i=0;i<5;++i) if(get(guards[i])!=012345u+i) return 2;
            if(get(c.push.base-1u)!=054321 || get(c.push.base+P66_VAC_WORDS)!=023456) return 2;
            emit(&c,n,step,1);
            if(n<PS_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, PDDL -> DANZIG before DSU; VAC %04o, push %04o -> %04o, integrated %u\n",n,step,cycles,c.push.base,initial_push,c.push.pushloc,ps_integrated(n));
        }
    }
    {
        unsigned vac,offset;
        for(vac=0;vac<5;++vac) for(offset=0;offset<38;++offset) if(!coverage[vac][offset]) return 2;
    }
    fprintf(stderr,"All five VAC payloads at all 38 legal offsets verified; all guards preserved.\n");
    return 0;
}
