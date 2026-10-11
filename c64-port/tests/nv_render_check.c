/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "nv_render_check.h"
#include "nv_render_vectors.h"
#include "dsky_scale.h"
static apollo_overlay_state s,before;
static void patch(apollo_overlay_state *image,p66_bias_io *io,uint16_t f,uint16_t v){if(f<APOLLO_SHARED_WORDS)image->words[f]=v;else if(f==APOLLO_SHARED_WORDS)io->superbank=v;else io->interrupt_enabled=v;}
static unsigned phase_call(uint8_t phase,p66_bias_io *io){switch(phase){
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==0
case 0:return (unsigned)p66_nv_scale_select(&s,io);
#endif
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==1
case 1:return (unsigned)dsky_scale_output(&s,io,s.words[0117]==2u?DSKY_DP1OUT:DSKY_DP3OUT);
#endif
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==2
case 2:return (unsigned)p66_nv_decimal_row(&s,io);
#endif
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==3
case 3:return (unsigned)p66_nv_next_row(&s,io);
#endif
default:return 1;}}
static unsigned check_stage(uint8_t stage,const uint16_t *numbers,const uint16_t *po,const uint16_t (*pp)[2],const uint16_t *ao,const uint16_t (*ap)[2]){unsigned failures=0;uint8_t j;for(j=0;j<16u;++j){uint16_t i,f,end; p66_bias_io io,old;nv_rows_input(&s,&io,(uint16_t)(numbers[j]*4u));(void)p66_nv_data_prepare(&s,&io);for(i=po[j];i<po[j+1u];++i)patch(&s,&io,pp[i][0],pp[i][1]);before=s;old=io;failures+=phase_call(stage,&io);i=ao[j];end=ao[j+1u];for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){uint16_t expected=f<APOLLO_SHARED_WORDS?before.words[f]:f==APOLLO_SHARED_WORDS?old.superbank:old.interrupt_enabled;uint16_t actual=f<APOLLO_SHARED_WORDS?s.words[f]:f==APOLLO_SHARED_WORDS?io.superbank:io.interrupt_enabled;if(i<end&&ap[i][0]==f)expected=ap[i++][1];if(actual!=expected)++failures;}if(i!=end)++failures;}return failures;}
unsigned nv_render_check(void){unsigned f=0;
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==0
f+=check_stage(0,nv_render0_numbers,nv_render0_pre_offsets,nv_render0_pre_patch,nv_render0_after_offsets,nv_render0_after_patch);
#endif
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==1
f+=check_stage(1,nv_render1_numbers,nv_render1_pre_offsets,nv_render1_pre_patch,nv_render1_after_offsets,nv_render1_after_patch);
#endif
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==2
f+=check_stage(2,nv_render2_numbers,nv_render2_pre_offsets,nv_render2_pre_patch,nv_render2_after_offsets,nv_render2_after_patch);
#endif
#if !defined(NV_RENDER_SELECTED) || NV_RENDER_SELECTED==3
f+=check_stage(3,nv_render3_numbers,nv_render3_pre_offsets,nv_render3_pre_patch,nv_render3_after_offsets,nv_render3_after_patch);
#endif
return f;}
