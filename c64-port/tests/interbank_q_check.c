/* SPDX-License-Identifier: MIT */
#include "interbank_q_check.h"
#include "interbank_q_cases.h"
#include "interbank_q_vectors.h"
#include "lm_interbank_routines.h"
#include "cm_interbank_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
static apollo_overlay_state image;
/* One step from the entry with Q forced to q: refused means EXEC_OUT_OF_SCOPE and an untouched image. */
static uint16_t refused(uint16_t n,agc_word q){static agc_routine_table t[1];agc_native_program p={0};
 p66_bias_io lio={0};cm_native_io cio={0};void *io;uint16_t superbank,steps;agc_word before[8];uint8_t i;enum exec_status st;
 ibq_input(&image,&superbank,n);image.words[2]=q;for(i=0;i<8u;++i)before[i]=image.words[i];
 if(n%2u){t[0]=cm_interbank_routines;cio.superbank=superbank;io=&cio;}else{t[0]=lm_interbank_routines;lio.superbank=superbank;io=&lio;}
 p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;
 st=agc_transfer_run(&p,&image,io,&superbank,AGC_TRANSFER_NONE,IBQ_MAX_STEPS,&steps);
 if(st!=EXEC_OUT_OF_SCOPE)return 0;
 for(i=0;i<8u;++i)if(image.words[i]!=before[i])return 0;
 return 1;}
uint16_t ibq_check(uint16_t n,uint16_t *checks){uint16_t f=0;
 ++*checks;if(ibq_digest(&image,n)!=ibq_digests[n])++f;
 if(n<4u){
  if(n<2u){++*checks;if(!refused(n,07777u))++f;}
  else{++*checks;if(!refused(n,07776u))++f;++*checks;if(!refused(n,07777u))++f;}}
 return f;}
