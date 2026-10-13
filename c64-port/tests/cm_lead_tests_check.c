/* SPDX-License-Identifier: MIT */
#include "cm_lead_tests_check.h"
#include "cm_lead_tests_cases.h"
#include "cm_lead_tests_vectors.h"
#include "standard_lead_routines.h"
#include "cm_native_image.h"
static apollo_overlay_state image;
uint16_t clt_check(uint16_t n,uint16_t *checks){uint16_t f=0,superbank,steps;agc_routine_table t[1];agc_native_program p={0};cm_native_io io={0};
 ++*checks;if(clt_digest(&image,n)!=clt_digests[n])++f;
 if(n<3u){
  t[0]=cm_standard_lead_routines;p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;
  clt_input(&image,&superbank,n);image.words[7]=1;io.superbank=superbank;
  ++*checks;if(agc_transfer_run(&p,&image,&io,&superbank,AGC_TRANSFER_NONE,1,&steps)!=EXEC_OUT_OF_SCOPE||image.words[5]!=02000u+n)++f;
  clt_input(&image,&superbank,n);superbank=0100u;
  ++*checks;if(agc_transfer_resolve(&p,&image,superbank)!=0)++f;}
 return f;}
