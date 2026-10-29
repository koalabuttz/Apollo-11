/* SPDX-License-Identifier: GPL-2.0-or-later
 * Genuine unchanged DSU PIPTIME and RODCOMP; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "elapsed_cases.h"
static agc_t baseline;
static unsigned paths[2][8];
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static void emit(elapsed_case *c,uint16_t n,uint8_t step,uint8_t stage,uint8_t path) {
    uint16_t w[EL_WORD_COUNT]; unsigned i;
    for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
    w[41]=(uint16_t)get(0120); w[42]=(uint16_t)get(0166);
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(c->previous.push.base+i);
    w[86]=(uint16_t)get(01234); w[87]=(uint16_t)get(01235); w[88]=(uint16_t)get(0130);
    el_from_words(c,w); printf("%u %u %u %u",n,step,stage,path);
    for(i=0;i<EL_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(int argc,char **argv) {
    uint16_t n;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    if((unsigned)machine.Fixed[02][0733]!=037777u ||
       (unsigned)machine.Fixed[02][0735]!=040000u) return 2;
    for(n=0;n<EL_CASE_COUNT;++n) {
        elapsed_case c; uint8_t step;
        el_prepare(&c,n); machine=baseline;
        for(step=0;step<el_steps(n);++step) {
            static const unsigned guards[9]={03643,03646,03760,03774,0153,01233,01236,0131,0132};
            uint16_t w[EL_WORD_COUNT]; unsigned i,pc=0,cycles,entered=0,pushed=0,pending_ts=0,negative=0;
            uint8_t path=0;
            unsigned initial_push=c.previous.push.pushloc,base=c.previous.push.base;
            el_next(&c,n,step); el_words(&c,w);
            for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
            set(0120,base); set(0166,initial_push);
            for(i=0;i<P66_VAC_WORDS;++i) set(base+i,w[43+i]);
            set(01234,w[86]); set(01235,w[87]); set(0130,w[88]);
            for(i=0;i<9;++i) set(guards[i],012345u+i);
            set(base-1u,054321); set(base+P66_VAC_WORDS,023456);
            set(RegFB,064000); set(RegQ,01030);
            if(el_integrated(n)) {
                set(RegEB,03400); set(RegBB,064007); set(RegZ,03275); machine.AllowInterrupt=1;
            } else {
                set(RegEB,01000); set(RegBB,064002); set(RegZ,07005);
                set(0116,01634); machine.AllowInterrupt=0;
            }
            for(cycles=0;cycles<2500;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(pending_ts) {
                        if(pc==pending_ts+2u) path=(uint8_t)(path | (pending_ts==07026 ? 2u : 4u));
                        else if(pc!=pending_ts+1u) return 2;
                        pending_ts=0;
                    }
                    if(entered && (pc==07026 || pc==07033)) pending_ts=pc;
                    if(pc==06523) pushed=1;
                    if(pc==06061 && pushed && !entered) { emit(&c,n,step,0,0); pushed=0; }
                    if(pc==07005) {
                        if(get(0116)!=01634 || get(RegEB)!=01000 || get(RegFB)!=064000 || get(0163)!=0) return 2;
                        if(el_integrated(n) && (!machine.AllowInterrupt || get(0164)!=03360 || get(0165)!=064007)) return 2;
                        entered=1;
                    }
                    if(pc==07003 && entered) { path|=1u; negative=get(RegA)!=1; }
                    if(pc==06061 && entered) break;
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==2500 || !entered || get(RegBB)!=064002 || get(0163)!=0 ||
               get(0166)!=initial_push+(el_integrated(n)?6u:0u) ||
               machine.AllowInterrupt!=(el_integrated(n)?1:0) || get(7)!=0) {
                fprintf(stderr,"case %u/%u: boundary failure at %05o\n",n,step,pc); return 2;
            }
            for(i=0;i<9;++i) if(get(guards[i])!=012345u+i) return 2;
            if(get(base-1u)!=054321 || get(base+P66_VAC_WORDS)!=023456) return 2;
            ++paths[negative][path]; emit(&c,n,step,1,path);
            if(n<EL_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, DSU -> DANZIG before STORE 30D; path %u, overflow sign %u, integrated %u\n",n,step,cycles,path,negative,el_integrated(n));
        }
    }
    {
        unsigned sign;
        for(sign=0;sign<2;++sign) {
            fprintf(stderr,"initial overflow sign %u: paths 1/5/7 = %u/%u/%u\n",sign,paths[sign][1],paths[sign][5],paths[sign][7]);
            if(!paths[sign][1] || !paths[sign][5] || !paths[sign][7]) return 2;
        }
    }
    return 0;
}
