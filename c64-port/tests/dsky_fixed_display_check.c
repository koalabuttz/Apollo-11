/* SPDX-License-Identifier: MIT */
#include "dsky_fixed_display_check.h"
#include "dsky_fixed_display_vectors.h"
uint16_t dsky_fixed_display_checkpoint(uint16_t n,apollo_overlay_state *s,apollo_overlay_state *before)
{p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_fixed_display_route route=(enum dsky_fixed_display_route)99;uint16_t f,p,end,fail=0;if(n>=DSKY_FIXED_DISPLAY_FROZEN_COUNT)return 1;dsky_fixed_display_input(s,&io,dsky_fixed_display_numbers[n],&mission);
 for(p=dsky_fixed_display_input_offsets[n];p<dsky_fixed_display_input_offsets[n+1u];++p){f=dsky_fixed_display_input_patch[p][0];if(f<2048u)s->words[f]=dsky_fixed_display_input_patch[p][1];else if(f==2048u)io.superbank=dsky_fixed_display_input_patch[p][1];else if(f==2049u)io.interrupt_enabled=dsky_fixed_display_input_patch[p][1];}
 *before=*s;old=io;
 if(dsky_fixed_display_stages[n]==1u){uint16_t address=65535;if(!dsky_fixed_display_address(s,&io,mission,&address)||address!=dsky_fixed_display_numbers[n]%40960u)++fail;}
 if(dsky_fixed_display(s,&io,mission,(enum dsky_fixed_display_stage)dsky_fixed_display_stages[n],dsky_fixed_display_values[n],&route)!=P66_BIAS_OK)++fail;
 p=dsky_fixed_display_output_offsets[n];end=dsky_fixed_display_output_offsets[n+1u];for(f=0;f<2051u;++f){uint16_t want=f<2048u?before->words[f]:f==2048u?old.superbank:f==2049u?old.interrupt_enabled:99,actual=f<2048u?s->words[f]:f==2048u?io.superbank:f==2049u?io.interrupt_enabled:(uint16_t)route;if(p<end&&dsky_fixed_display_output_patch[p][0]==f)want=dsky_fixed_display_output_patch[p++][1];if(actual!=want)++fail;}if(p!=end)++fail;return fail;}
