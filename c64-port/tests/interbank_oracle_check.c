/* SPDX-License-Identifier: MIT */
#include "interbank_oracle_check.h"
#include "interbank_oracle_cases.h"
#include "lm_interbank_routines.h"
#include "cm_interbank_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#if defined(__mos__)
#define image (*(apollo_overlay_state *)0xe000u)
#else
static apollo_overlay_state image;
#endif
/* BANKCALL of mission cm with Q at 07775 (fixed-fixed, after no genuine
 * TC BANKCALL): refused, the image unchanged. */
static uint16_t refused(uint8_t cm){static agc_routine_table t[1];agc_native_program p={0};p66_bias_io lio={0};cm_native_io cio={0};
 uint16_t superbank,inhint,i,steps,count;agc_word before[8];enum exec_status st;
 ibo_input(&image,&superbank,&inhint,0);image.words[2]=07775u;image.words[5]=cm?04662u:04616u;for(i=0;i<8u;++i)before[i]=image.words[i];
 if(cm){t[0]=cm_interbank_routines;cio.superbank=superbank;p.operands=cm_interbank_site_table(&count);}
 else{t[0]=lm_interbank_routines;lio.superbank=superbank;p.operands=lm_interbank_site_table(&count);}
 p.operand_count=count;p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;
 st=agc_transfer_run(&p,&image,cm?(void *)&cio:(void *)&lio,cm?&cio.superbank:&lio.superbank,AGC_TRANSFER_NONE,1,&steps);
 if(st!=EXEC_OUT_OF_SCOPE)return 0;
 for(i=0;i<8u;++i)if(image.words[i]!=before[i])return 0;
 return 1;}
static uint16_t sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i].physical<=t->entries[i-1u].physical)return 0;return 1;}
uint16_t ibo_check(uint16_t n,uint32_t expected,uint16_t *checks){uint16_t f=0;
 ++*checks;if(ibo_digest(&image,n)!=expected)++f;
 if(n<2u){
  ++*checks;if(!refused((uint8_t)n))++f;
  ++*checks;if(!sorted(n?&cm_interbank_routines:&lm_interbank_routines))++f;}
 return f;}
