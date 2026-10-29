/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_keyboard_number.h"
#include "dsky_keycom.h"
#include "dsky_keyboard_number_flow_cases.h"
static apollo_overlay_state image;static executive state;static exec_native runtime;static dsky_display_state display;static dsky_keyboard_binding binding;static uint16_t number;
static uint8_t enter(void *u){(void)u;return 0;}static void leave(void *u,uint8_t t){(void)u;(void)t;}static const exec_exclusion guard={enter,leave,0};
static void snapshot(uint16_t phase){uint16_t i;printf("%u %u 0",number,phase);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",image.words[i]);printf(" %u %u %u\n",binding.basic.control.superbank,binding.basic.control.interrupt_enabled,display.channel11);}
static void observe(void *u,const apollo_overlay_state *s,agc_word pc){(void)u;(void)s;if(pc==05155)snapshot(2);}
int main(void){for(number=0;number<DSKY_KEYBOARD_NUMBER_FLOW_CASE_COUNT;++number){p66_basic_binding *b=&binding.basic;dsky_keyboard_number_flow_prepare(&image,number);memset(&binding,0,sizeof binding);dsky_display_reset(&display);display.channel11=dsky_keyboard_number_flow_channel11(number);b->arithmetic.image=&image;b->arithmetic.runtime=&runtime;b->arithmetic.producers=&guard;b->display=&display;b->control.superbank=(uint16_t)((number/128u)%8u*020u);b->control.interrupt_enabled=1;
 state.alarm=0;p66_native_import_executive(&state,&image);if(exec_native_context_init(&runtime,&state,dsky_keyboard_number_mappings,DSKY_KEYBOARD_MAPPING_COUNT,&binding)!=EXEC_OK)return 2;
 if(dsky_keyboard_interrupt(&binding,dsky_keyboard_number_flow_key(number),010067)!=EXEC_OK)return 2;
 snapshot(0);
 if(dsky_keyboard_idle(&binding)!=EXEC_OK)return 2;
 snapshot(1);b->arithmetic.observe=observe;
 if(p66_basic_step(b)!=EXEC_IDLE || b->status!=0 || runtime.running!=0 || image.words[0167]!=077777 || image.words[067]!=077777)return 2;
 }return 0;}
