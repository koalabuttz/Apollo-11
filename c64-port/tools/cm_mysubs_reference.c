/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_mysubs_cases.h"
static agc_t baseline;
static uint32_t visited[02000],first[02000];
static void row(uint32_t n,uint8_t selector,uint8_t phase){uint16_t w[2053];unsigned i;w[0]=(uint16_t)n;w[1]=(uint16_t)(n>>16);w[2]=selector;w[3]=phase;w[4]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint8_t selector;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(selector=0;selector<2u;++selector){uint32_t n;for(n=0;n<CM_MYSUBS_CASES;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0,phase=0;cm_mysubs_input(&m,&irq,n,selector);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<300u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(!phase&&(pc==02053u||pc==02073u)){row(n,selector,0);phase=1;}else if(phase&&pc==m.words[2]&&get(2)==3){row(n,selector,1);break;}if(pc>=02042u&&pc<=02104u&&get(RegFB)==042000u){++visited[pc-02000u];if(!first[pc-02000u])first[pc-02000u]=n+1u;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==300u){fprintf(stderr,"selector%u case%u timeoutPC%o Q%o phase%u\n",selector,(unsigned)n,pc,get(2),phase);return 2;}}}{unsigned i;for(i=02042u;i<=02104u;++i)fprintf(stderr,"sourcePC%04o completed_instruction_visits%u firstcase%u\n",i,(unsigned)visited[i-02000u],(unsigned)(first[i-02000u]-1u));}return 0;}
