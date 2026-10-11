/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_native_cases.h"
static apollo_overlay_state image;static executive state;static exec_native runtime;static velocity_case inputs;
static int mixed;
static uint16_t number;static unsigned stage;
static uint8_t enter(void *u){(void)u;return 0;}static void leave(void*u,uint8_t t){(void)u;(void)t;}
static void observe(void*u,const apollo_overlay_state*m,agc_word loc){
 uint16_t w[PN_WORDS];unsigned i;(void)u;(void)loc;pn_words(m,m->words[0120],w);
 printf("%u %u",number,stage++);for(i=0;i<PN_WORDS;++i)printf(" %u",w[i]);
 if(mixed){for(i=0;i<96;++i)printf(" %u",m->words[0154+i]);for(i=0;i<220;++i)printf(" %u",m->words[0400+i]);printf(" %u %u",m->words[0120],m->words[0121]);printf(" %u %u %u %u %u %u %u",m->words[0116],m->words[023],m->words[0115],m->words[7],m->words[3],m->words[4],m->words[6]);}
 puts("");
}
int main(int argc,char **argv){
 (void)argv;mixed=argc>1;
 static const exec_exclusion guard={enter,leave,0};p66_native_binding b={&image,&runtime,&guard,observe,0,P66_NATIVE_OK,0,0};
 for(number=0;number<(mixed?16u:PN_CASES);++number){unsigned i;stage=0;pn_prepare(&image,&state,&inputs,number);
 if(mixed){unsigned base=image.words[0120]==0401u?0455u:0401u;image.words[base-1u]=0;
 for(i=0;i<43;++i)image.words[base+i]=(agc_word)((i*17u+number*31u)&077777u);
 image.words[0154+12+EXEC_LOC]=03275;image.words[0154+12+EXEC_BANK]=064007;
 image.words[0154+12+EXEC_PUSH]=(agc_word)base;image.words[0154+12+EXEC_PRIORITY]=(agc_word)(04000u+base);
 }
 if(exec_native_context_init(&runtime,&state,p66_native_mappings,P66_NATIVE_MAPPING_COUNT,&b)!=EXEC_OK)return 2;
 runtime.running=1;runtime.current.location=03275;runtime.current.bank=number&1u?064067:064007;
 if(mixed){if(p66_native_step(&b)!=EXEC_OK)return 2;image.words[0067]=12;
 for(i=0;i<6;++i)if(p66_native_step(&b)!=EXEC_OK)return 2;
 }else for(i=0;i<5;++i)if(p66_native_step(&b)!=EXEC_OK)return 2;
 if(p66_native_step(&b)!=EXEC_OUT_OF_SCOPE || b.status!=P66_NATIVE_BOUNDARY || stage!=(mixed?7u:5u))return 2;
 }
 return 0;
}
