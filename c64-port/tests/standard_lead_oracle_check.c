/* SPDX-License-Identifier: MIT */
#include "standard_lead_oracle_check.h"
#include "standard_lead_oracle_cases.h"
#include "standard_lead_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#if defined(__mos__)
#define image (*(apollo_overlay_state *)0xe000u)
#else
static apollo_overlay_state image;
#endif
/* The CALL (kind 10) or EXIT (kind 9) word of mission cm with FB moved off
 * BANKSET: refused, the image unchanged. */
static uint16_t refused(uint8_t cm,uint8_t kind){uint16_t superbank,inhint,i,n=(uint16_t)(2u*kind+cm);agc_word before[8];enum exec_status st;
 p66_bias_io lio={0};cm_native_io cio={0};
 slo_input(&image,&superbank,&inhint,n);image.words[4]=(agc_word)(image.words[4]^04000u);image.words[6]=(agc_word)(image.words[6]^04000u);
 for(i=0;i<8u;++i)before[i]=image.words[i];
 st=cm?(cm_standard_lead_interpretive_routines.entries[kind==10u?0:1].call)(0,&image,&cio):(standard_lead_interpretive_routines.entries[kind==10u?0:1].call)(0,&image,&lio);
 if(st!=EXEC_OUT_OF_SCOPE)return 0;
 for(i=0;i<8u;++i)if(image.words[i]!=before[i])return 0;
 return 1;}
static uint16_t sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i].physical<=t->entries[i-1u].physical)return 0;return 1;}
uint16_t slo_check(uint16_t n,uint32_t expected,uint16_t *checks){uint16_t f=0;
 ++*checks;if(slo_digest(&image,n)!=expected)++f;
 if(n<2u){
  ++*checks;if(!refused((uint8_t)n,9u))++f;
  ++*checks;if(!refused((uint8_t)n,10u))++f;
  ++*checks;if(!sorted(n?&cm_standard_lead_interpretive_routines:&standard_lead_interpretive_routines))++f;}
 return f;}
