/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_dyndisp_check.h"
#include "cm_dyndisp_vectors.h"
static const uint8_t control[27]={0,1,2,3,4,5,6,7,255,8,9,254,254,254,254,10,253,11,252,252,252,252,12,13,14,15,16};
static void patch(apollo_overlay_state *s,cm_native_io *io,uint16_t field,uint16_t value){if(field<2048u)s->words[field]=value;else if(field==2048u)io->superbank=value;else io->interrupt_enabled=value;}
uint8_t cm_dyndisp_checkpoint_input(uint8_t n,apollo_overlay_state *s,cm_native_io *io){uint16_t i;if(n>=68u)return 255;cm_dyndisp_input(s,io,(uint16_t)(cm_dyndisp_numbers[n]/27u));for(i=cm_dyndisp_before_offsets[n];i<cm_dyndisp_before_offsets[n+1u];++i)patch(s,io,cm_dyndisp_before_patch[i][0],cm_dyndisp_before_patch[i][1]);return control[cm_dyndisp_numbers[n]%27u];}
uint16_t cm_dyndisp_checkpoint_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){cm_native_io io,old;uint16_t failures=0,f,p,end;uint8_t phase=cm_dyndisp_checkpoint_input(n,s,&io);if(phase==255u)return 1;memcpy(before,s,sizeof *s);old=io;if(!cm_dyndisp_stage(s,&io,phase))++failures;p=cm_dyndisp_after_offsets[n];end=cm_dyndisp_after_offsets[n+1u];for(f=0;f<2050u;++f){uint16_t expected=f<2048u?before->words[f]:f==2048u?old.superbank:old.interrupt_enabled;uint16_t actual=f<2048u?s->words[f]:f==2048u?io.superbank:io.interrupt_enabled;if(p<end&&cm_dyndisp_after_patch[p][0]==f)expected=cm_dyndisp_after_patch[p++][1];if(actual!=expected)++failures;}if(p!=end)++failures;return failures;}
