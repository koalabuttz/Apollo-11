/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only original-assembly oracle; constructors contain no native arithmetic. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "dp_divide_cases.h"
static agc_t baseline;
int main(int argc,char **argv) {
    uint32_t n; if(argc!=2) return 2; memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2; baseline=machine; InhibitAlarms=1;
    for(n=0;n<DP_DIVIDE_CASES;++n) {
        uint16_t w[20]; unsigned i,cycles,pc,path=0;
        static const unsigned guards[]={0120,0132,0133,0134,0141,0153,0166,01002};
        machine=baseline; dp_divide_input(n,w); for(i=0;i<8;++i) set(0154+i,w[i]);
        *memory(RegA)=(int16_t)w[8]; *memory(RegL)=(int16_t)w[9];
        set(0130,w[10]);set(0131,w[11]);set(0136,w[12]);set(0137,w[13]);set(0140,w[14]);set(0121,w[15]);set(0135,w[16]);
        for(i=0;i<sizeof guards/sizeof guards[0];++i) set(guards[i],012345u+i);
        set(01000,w[17]);set(01001,w[18]); set(RegFB,066000);set(RegEB,03400);set(RegBB,066007);
        set(RegZ,w[19]?07603:07577);set(0116,01000);set(0164,03363);set(0165,064007);set(023,0);set(067,0); machine.AllowInterrupt=0;
        for(cycles=0;cycles<2500;++cycles) {
            pc=get(RegZ); if(!machine.PendFlag && !machine.ExtraDelay) {
                if(pc==02405) path=2; if(pc==02505) path=1; if(pc==06061) break;
            }
            machine.ScalerCounter=0; if(agc_engine(&machine)!=0) return 2;
        }
        if(cycles==2500) {fprintf(stderr,"case %u timeout %05o\n",(unsigned)n,pc);return 2;}
        if(get(RegFB)!=0 || get(RegEB)!=03400 || get(RegBB)!=7 || get(0116)!=01000 || get(0164)!=03363 || machine.AllowInterrupt || get(7)!=0 || get(01000)!=w[17] || get(01001)!=w[18]) return 2;
        for(i=0;i<sizeof guards/sizeof guards[0];++i) if(get(guards[i])!=012345u+i) return 2;
        printf("%u %u",(unsigned)n,path); for(i=0;i<8;++i) printf(" %u",get(0154+i));
        printf(" %u %u %u %u %u %u %u %u %u\n",(unsigned)*memory(RegA)&0177777u,(unsigned)*memory(RegL)&0177777u,get(0130),get(0131),get(0136),get(0137),get(0140),get(0121),get(0135));
    } return 0;
}
