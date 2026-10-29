/* SPDX-License-Identifier: GPL-2.0-or-later
 * Genuine unchanged STORE 30D and RODCOMP; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "store_cases.h"
static agc_t baseline;
static unsigned paths[2][8], slots[5][38];
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static void emit(store_case *c,uint16_t n,uint8_t step,uint8_t stage,uint8_t path) {
    uint16_t w[ST_WORD_COUNT]; unsigned i;
    for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
    w[41]=(uint16_t)get(0120); w[42]=(uint16_t)get(0166);
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(c->previous.push.base+i);
    w[86]=(uint16_t)get(01234); w[87]=(uint16_t)get(01235); w[88]=(uint16_t)get(0130);
    el_from_words(c,w); printf("%u %u %u %u",n,step,stage,path);
    for(i=0;i<ST_WORD_COUNT;++i) printf(" %u",w[i]);
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
    for(n=0;n<ST_CASE_COUNT;++n) {
        store_case c; uint8_t step;
        st_prepare(&c,n); machine=baseline;
        for(step=0;step<st_steps(n);++step) {
            static const unsigned guards[9]={03643,03646,03760,03774,0153,01233,01236,0131,0132};
            uint16_t w[ST_WORD_COUNT]; unsigned i,pc=0,cycles,entered=0,stored=0,pending_ts=0,negative=0;
            uint8_t path=0;
            unsigned initial_push=c.previous.push.pushloc,base=c.previous.push.base;
            st_next(&c,n,step); el_words(&c,w);
            if(n>=163u && n<353u) {
                unsigned vac=(base-0401u)/054u,slot=initial_push-base;
                if(vac>=5u || slot>=38u) return 2;
                ++slots[vac][slot];
            }
            for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
            set(0120,base); set(0166,initial_push);
            for(i=0;i<P66_VAC_WORDS;++i) set(base+i,w[43+i]);
            set(01234,w[86]); set(01235,w[87]); set(0130,w[88]);
            for(i=0;i<9;++i) set(guards[i],012345u+i);
            set(base-1u,054321); set(base+P66_VAC_WORDS,023456);
            set(RegFB,064000); set(RegQ,01030); set(067,0);
            if(st_integrated(n)) {
                set(RegEB,03400); set(RegBB,064007); set(RegZ,03275); machine.AllowInterrupt=1;
            } else {
                set(RegEB,01000); set(RegBB,064002); set(RegZ,06061);
                set(0116,01634); set(0164,03360); set(0165,064007); set(023,0);
                machine.AllowInterrupt=0;
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
                    if(pc==06061 && entered && !stored) { emit(&c,n,step,0,path); entered=0; }
                    if(pc==06422) {
                        if(get(0116)!=036 || get(0164)!=03361 || get(RegBB)!=064007 || get(RegQ)!=06373) return 2;
                        stored=1;
                    }
                    if(pc==06434 && (get(0116)!=base+30u || get(0163)!=0)) return 2;
                    if(pc==07005) {
                        if(get(0116)!=01634 || get(RegEB)!=01000 || get(RegFB)!=064000 || get(0163)!=0) return 2;
                        if(st_integrated(n) && (!machine.AllowInterrupt || get(0164)!=03360 || get(0165)!=064007)) return 2;
                        entered=1;
                    }
                    if(pc==07003 && entered) { path|=1u; negative=get(RegA)!=1; }
                    if(pc==06061 && stored) break;
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==2500 || !stored || get(RegBB)!=064007 || get(0163)!=0 ||
               get(0166)!=initial_push+(st_integrated(n)?6u:0u) ||
               machine.AllowInterrupt!=(st_integrated(n)?1:0) || get(7)!=0 || get(0116)!=base+30u || get(0164)!=03361 ||
               get(RegQ)!=3) {
                fprintf(stderr,"case %u/%u: boundary failure at %05o: stored %u BB %05o MODE %05o push %05o/%05o IRQ %u ZERO %05o addr %05o/%05o LOC %05o Q %05o\n",n,step,pc,stored,get(RegBB),get(0163),get(0166),initial_push,machine.AllowInterrupt,get(7),get(0116),base+30u,get(0164),get(RegQ)); return 2;
            }
            for(i=0;i<9;++i) if(get(guards[i])!=012345u+i) return 2;
            if(get(base-1u)!=054321 || get(base+P66_VAC_WORDS)!=023456) return 2;
            ++paths[negative][path]; emit(&c,n,step,1,path);
            if(n<ST_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, STORE 30D -> DANZIG before DDV; path %u, overflow sign %u, integrated %u\n",n,step,cycles,path,negative,st_integrated(n));
        }
    }
    {
        unsigned sign,vac,slot;
        for(vac=0;vac<5;++vac) for(slot=0;slot<38;++slot)
            if(slots[vac][slot]!=1) return 2;
        fprintf(stderr,"all 190 VAC/push slots observed, including 35 overlapping saves\n");
        for(sign=0;sign<2;++sign) {
            fprintf(stderr,"initial overflow sign %u: paths 1/5/7 = %u/%u/%u\n",sign,paths[sign][1],paths[sign][5],paths[sign][7]);
            if(!paths[sign][5] || !paths[sign][7]) return 2;
        }
    }
    return 0;
}
