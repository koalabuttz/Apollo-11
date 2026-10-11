/* SPDX-License-Identifier: MIT */
#include "dsky_test_noun_check.h"
#include "dsky_test_noun_vectors.h"
uint16_t dsky_test_noun_checkpoint(uint16_t n,apollo_overlay_state *s,apollo_overlay_state *before)
{p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_enter_zero_route route=(enum dsky_enter_zero_route)99;uint16_t f,p,end,fail=0;if(n>=DSKY_TEST_NOUN_FROZEN_COUNT)return 1;dsky_test_noun_input(s,&io,dsky_test_noun_numbers[n],&mission);*before=*s;old=io;if(dsky_test_noun(s,&io,mission,&route)!=P66_BIAS_OK)++fail;p=dsky_test_noun_offsets[n];end=dsky_test_noun_offsets[n+1u];for(f=0;f<2051u;++f){uint16_t want=f<2048u?before->words[f]:f==2048u?old.superbank:f==2049u?old.interrupt_enabled:99,actual=f<2048u?s->words[f]:f==2048u?io.superbank:f==2049u?io.interrupt_enabled:(uint16_t)route;if(p<end&&dsky_test_noun_patch[p][0]==f)want=dsky_test_noun_patch[p++][1];if(actual!=want)++fail;}if(p!=end)++fail;return fail;}
