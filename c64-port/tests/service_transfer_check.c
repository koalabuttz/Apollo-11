/* SPDX-License-Identifier: MIT */
#include "service_transfer_check.h"
#include "service_transfer_cases.h"
#include "service_transfer.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#if defined(__mos__)
#define image (*(apollo_overlay_state *)0xe000u)
#else
static apollo_overlay_state image;
#endif
/* One step at the mission's UPFLAG (n even) or DOWNFLAG with Q forced to q: refused leaves the image unchanged. */
static uint16_t refused(uint16_t n,agc_word q){static agc_routine_table t[1];agc_native_program p={0};
 p66_bias_io lio={0};cm_native_io cio={0};void *io;uint16_t superbank,inhint,steps,count,i;agc_word before[8];enum exec_status st;uint8_t cm=(uint8_t)((n/2u)%2u);
 svt_input(&image,&superbank,&inhint,0);image.words[2]=q;image.words[5]=cm?(n%2u?05560u:05546u):(n%2u?05516u:05504u);
 for(i=0;i<8u;++i)before[i]=image.words[i];
 if(cm){t[0]=cm_service_routines;cio.superbank=superbank;io=&cio;p.operands=cm_service_site_table(&count);}
 else{t[0]=lm_service_routines;lio.superbank=superbank;io=&lio;p.operands=lm_service_site_table(&count);}
 p.operand_count=count;p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;
 st=agc_transfer_run(&p,&image,io,&superbank,AGC_TRANSFER_NONE,1,&steps);
 if(st!=EXEC_OUT_OF_SCOPE)return 0;
 for(i=0;i<8u;++i)if(image.words[i]!=before[i])return 0;
 return 1;}
static uint16_t sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i].physical<=t->entries[i-1u].physical)return 0;return 1;}
uint16_t svt_check(uint16_t n,uint32_t expected,uint16_t *checks){uint16_t f=0;
 ++*checks;if(svt_digest(&image,n)!=expected)++f;
 if(n<4u){
  ++*checks;if(!refused(n,077u))++f;
  /* 07775 is fixed-fixed but follows no genuine TC UPFLAG/DOWNFLAG. */
  ++*checks;if(!refused(n,07775u))++f;
  ++*checks;if(!sorted(n%2u?&cm_service_routines:&lm_service_routines))++f;}
 return f;}
