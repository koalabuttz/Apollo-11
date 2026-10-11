/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "norm_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t index;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
if((unsigned)machine.Fixed[0][0314]!=022650u||(unsigned)machine.Fixed[0][01007]!=032506u||(unsigned)machine.Fixed[0][0270]!=04604u||(unsigned)machine.Fixed[0][0566]!=06552u)return 2;
for(index=0;index<NORM_CASES;++index){uint16_t w[20];unsigned i,pc,cycles,path=0;static const unsigned guards[]={0120,0132,0133,0134,0141,0122,0123,0124,0125,0126,0127,0166};machine=baseline;norm_input(index,w);
    for(i=0;i<8;++i)set(0154+i,w[i]);*memory(RegA)=(int16_t)w[8];*memory(RegL)=(int16_t)w[9];set(0135,w[10]);set(0121,w[11]);set(0130,w[12]);set(0131,w[13]);set(0136,w[14]);set(0137,w[15]);set(0140,w[16]);set(021,w[17]);set(022,w[18]);set(0116,w[19]);set(RegFB,0);set(RegEB,03400);set(RegBB,7);set(RegZ,(index&1u)?03343:03207);set(RegQ,06030);set(0164,03363);set(0165,064007);set(023,0);set(067,0);for(i=0;i<sizeof guards/sizeof guards[0];++i)set(guards[i],012345u+i);machine.AllowInterrupt=0;
    for(cycles=0;cycles<4000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(pc==03402){path=1;break;}if(pc==06030)break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
    if(cycles==4000){fprintf(stderr,"case %u timeout %05o\n",(unsigned)index,pc);return 2;}
    if(get(RegFB)!=0||get(RegEB)!=03400||get(RegBB)!=7||get(0163)!=w[7]||get(0164)!=03363||machine.AllowInterrupt||get(7))return 2;
    for(i=0;i<sizeof guards/sizeof guards[0];++i)if(get(guards[i])!=012345u+i)return 2;
    printf("%u %u",(unsigned)index,path);for(i=0;i<8;++i)printf(" %u",get(0154+i));printf(" %u %u %u %u %u %u %u %u %u %u %u %u\n",(unsigned)*memory(RegA)&0177777u,(unsigned)*memory(RegL)&0177777u,get(0135),get(0121),get(0130),get(0131),get(0136),get(0137),get(0140),get(021),get(022),get(0116));
}return 0;}
