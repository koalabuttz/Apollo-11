/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_native_guidance.h"
#include "p66_guidance_invoke.h"
static uint16_t linked(void *unused,apollo_overlay_state *image,uint16_t selector){(void)unused;return p66_native_guidance_entry(image,selector);}
static uint16_t whole(p66_guidance_modules *modules,apollo_overlay_state *image,uint16_t group){uint16_t result,phase,last;if(group==6){result=p66_guidance_invoke(modules,image,6);if(result!=0)return result;phase=136;last=186;}else if(group==7){phase=107;last=147;}else return p66_guidance_invoke(modules,image,group);for(;;){result=linked(0,image,phase);if(result!=0||phase==last)return result;phase=(uint16_t)(phase+10u);}}
static const uint16_t common[]={03746,03756,03644,03645,0,1,037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,01160,01161,01162,01254,01255,01256,0154,0155,0156,0157,0160,0161,0162,0163,03766,03767,03770,03771,03772,03773,0135,0121};
static const uint16_t norm[]={021,022,0116,0132,0141,0122,0123,0124,0125,0126,0127};
static const uint16_t trig[]={0133,0134,0117,0142,0143,0766,0767,0770,0771,0772,0773,0736,0737,0740,0741,0742,0743,0744,0745,0746,0747,0750,0751,032,033,034,0144,0145,03463,03464};
static void addresses(uint16_t map[134],unsigned group,uint16_t base){unsigned i;memcpy(map,common,sizeof common);map[41]=0120;map[42]=0166;for(i=0;i<53;++i)map[43+i]=(uint16_t)(base+i);map[86]=01234;map[87]=01235;map[88]=0130;map[89]=0131;map[90]=0136;map[91]=0137;map[92]=0140;
 if(group==6){for(i=0;i<6;++i){map[93+i]=(uint16_t)(01236+i);map[99+i]=(uint16_t)(02630+i);map[105+i]=(uint16_t)(03526+i);map[111+i]=(uint16_t)(03520+i);}memcpy(map+117,norm,sizeof norm);map[128]=03473;map[129]=03474;map[130]=03774;map[131]=03775;map[132]=02332;map[133]=02333;}
 else{memcpy(map+93,norm,sizeof norm);if(group==7){map[104]=02540;map[105]=02541;for(i=0;i<6;++i)map[106+i]=(uint16_t)(01236+i);}else memcpy(map+104,trig,sizeof trig);}}
int main(int argc,char **argv){unsigned group,count;p66_guidance_modules modules={linked,0};(void)argv;while(scanf("%u %u",&group,&count)==2){apollo_overlay_state image,before;uint16_t values[134],map[134],result;unsigned i;if(count>134)return 2;for(i=0;i<count;++i){unsigned value;if(scanf("%u",&value)!=1)return 2;values[i]=(uint16_t)value;}for(i=0;i<2048;++i)image.words[i]=(uint16_t)((i*31u+012345u)&077777u);addresses(map,group,values[41]);for(i=0;i<count;++i)image.words[map[i]]=values[i];image.words[values[41]-1u]=0;image.words[0167]=values[41];before=image;result=argc>1?whole(&modules,&image,(uint16_t)group):p66_native_guidance_entry(&image,(uint16_t)(group==6?206:group));
 for(i=0;i<2048;++i){unsigned j;if(image.words[i]==before.words[i]||i==2||i==3||i==4||i==5||i==6||i==7||i==023||i==0164||i==0165)continue;for(j=0;j<count&&map[j]!=i;++j){}if(j==count){fprintf(stderr,"group%u unexpected physical%04o\n",group,i);return 3;}}
 printf("%u",result);for(i=0;i<count;++i)printf(" %u",image.words[map[i]]);puts("");}return 0;}
