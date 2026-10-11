/* SPDX-License-Identifier: GPL-2.0-or-later
 * Unchanged VXSC KPIP1 in genuine Virtual AGC, no native arithmetic linked. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "scale_cases.h"
static agc_t baseline;
static unsigned rounding[2][3], branches[3];
static const unsigned addresses[SC_WORD_COUNT]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static const unsigned ts_pc[6]={03317,03322,03325,03331,03335,03341};
static const unsigned guards[5]={03643,03646,03760,03774,0153};
int main(int argc,char **argv) {
    uint16_t n;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    if((unsigned)machine.Fixed[033][021]!=00321u ||
       (unsigned)machine.Fixed[033][022]!=026706u) return 2;
    for(n=0;n<SC_CASE_COUNT;++n) {
        scale_case c; uint8_t step;
        sc_prepare(&c,n); machine=baseline;
        for(step=0;step<sc_steps(n);++step) {
            uint16_t w[SC_WORD_COUNT]; unsigned i,cycles,pc=0,entered=0,pending_ts=0,skip_mask=0;
            sc_next(&c,n,step); sc_words(&c,w);
            for(i=0;i<SC_WORD_COUNT;++i) *memory(addresses[i])=(int16_t)w[i];
            for(i=0;i<5;++i) set(guards[i],012345u+i);
            set(RegEB,03400); set(RegQ,01030);
            if(sc_integrated(n)) {
                set(RegFB,064000); set(RegBB,064007); set(RegZ,03275);
                machine.AllowInterrupt=1;
            } else {
                set(RegFB,066000); set(RegBB,066007); set(0116,02021);
                set(RegZ,07401); machine.AllowInterrupt=0;
            }
            for(cycles=0;cycles<2000;++cycles) {
                pc=get(RegZ);
                if(!machine.PendFlag && !machine.ExtraDelay) {
                    if(pending_ts) {
                        if(pc==pending_ts+2u) {
                            for(i=0;i<6;++i) if(ts_pc[i]==pending_ts) skip_mask|=1u<<i;
                        } else if(pc!=pending_ts+1u) return 2;
                        pending_ts=0;
                    }
                    if(sc_integrated(n)) for(i=0;i<6;++i) if(pc==ts_pc[i]) pending_ts=pc;
                    if(pc==07140) {
                        unsigned word=get(0156), negative=(word & 040000u)!=0;
                        unsigned magnitude=negative ? (word ^ 077777u) : word;
                        ++rounding[negative][magnitude<020000u ? 0 : magnitude==020000u ? 1 : 2];
                    }
                    if(pc==07144) ++branches[0];
                    if(pc==07147) ++branches[1];
                    if(pc==07152) ++branches[2];
                    if(pc==07401) {
                        if(get(0116)!=02021 || get(RegFB)!=066000 || get(0163)!=077776) return 2;
                        if(sc_integrated(n) && (!machine.AllowInterrupt || get(0164)!=03355 || get(0165)!=064007)) {
                            fprintf(stderr,"interpreter entry failure %u: LOC %05o BANKSET %05o\n",n,get(0164),get(0165)); return 2;
                        }
                        entered=1;
                    }
                    if(pc==06061 && entered) break;
                }
                machine.ScalerCounter=0;
                if(agc_engine(&machine)!=0) return 2;
            }
            if(cycles==2000 || !entered || get(RegEB)!=03400 ||
               machine.AllowInterrupt!=(sc_integrated(n)?1:0) || get(RegBB)!=066007) {
                fprintf(stderr,"case %u/%u failed at %05o after %u cycles\n",n,step,pc,cycles); return 2;
            }
            if(n>=152u && n<280u && skip_mask!=(n-152u)%64u) return 2;
            /* Interpreter LOC/BANKSET at 0164/0165 are outside numeric state. */
            for(i=0;i<5;++i) if(get(guards[i])!=012345u+i) return 2;
            for(i=0;i<SC_WORD_COUNT;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
            sc_from_words(&c,w);
            printf("%u %u",n,step);
            for(i=0;i<SC_WORD_COUNT;++i) printf(" %u",w[i]);
            puts("");
            if(n<SC_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, VXSC KPIP1 -> DANZIG, integrated %u\n",n,step,cycles,sc_integrated(n));
        }
    }
    fprintf(stderr,"VROUND magnitudes below/equal/above .5, positive: %u/%u/%u; negative: %u/%u/%u; low adjustments/high propagation/OVFIND: %u/%u/%u\n",
            rounding[0][0],rounding[0][1],rounding[0][2],rounding[1][0],rounding[1][1],rounding[1][2],branches[0],branches[1],branches[2]);
    if(!rounding[0][0] || !rounding[0][1] || !rounding[0][2] ||
       !rounding[1][0] || !rounding[1][1] || !rounding[1][2] || !branches[1]) return 2;
    return 0;
}
