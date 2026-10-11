/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_paste_check.h"
#include "dsky_monitor_paste_vectors.h"
uint16_t dsky_monitor_paste_checkpoint(uint16_t n,apollo_overlay_state *s,apollo_overlay_state *before)
{p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_monitor_paste_stage stage;uint16_t f,p,end,fail=0;if(n>=96u)return 1;dsky_monitor_paste_input(s,&io,dsky_monitor_paste_numbers[n],&mission,&stage);*before=*s;old=io;if(dsky_monitor_paste(s,&io,mission,stage)!=P66_BIAS_OK)++fail;p=dsky_monitor_paste_offsets[n];end=dsky_monitor_paste_offsets[n+1u];for(f=0;f<2050u;++f){uint16_t want=f<2048u?before->words[f]:f==2048u?old.superbank:old.interrupt_enabled,actual=f<2048u?s->words[f]:f==2048u?io.superbank:io.interrupt_enabled;if(p<end&&dsky_monitor_paste_patch[p][0]==f)want=dsky_monitor_paste_patch[p++][1];if(actual!=want)++fail;}if(p!=end)++fail;return fail;}
