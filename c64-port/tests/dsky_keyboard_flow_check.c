/* SPDX-License-Identifier: MIT
 * Actual queued native keyboard jobs against complete original source images. */
#include <string.h>
#include "dsky_keyboard.h"
#include "dsky_keycom.h"
#include "dsky_keyboard_flow_check.h"
#include "dsky_keyboard_flow_vectors.h"
#ifdef DSKY_KEYBOARD_FLOW_C64
#include "c64_dsky_keyboard.h"
#endif
static apollo_overlay_state image,before;
static executive state;
static exec_native runtime;
static dsky_display_state display;
static dsky_keyboard_binding binding;
static uint8_t fixture,excluded,observed;
static uint16_t failures;
static uint8_t enter(void *u){uint8_t previous;(void)u;
#ifdef DSKY_KEYBOARD_FLOW_C64
 __asm__ volatile("php\nsei\npla" : "=a"(previous) : : "memory");
#else
 previous=excluded;
#endif
 excluded=1;return previous;}
static void leave(void *u,uint8_t previous){(void)u;
#ifdef DSKY_KEYBOARD_FLOW_C64
 if((previous&4u)==0)__asm__ volatile("cli" : : : "memory");else __asm__ volatile("sei" : : : "memory");excluded=(uint8_t)((previous&4u)!=0);
#else
 excluded=previous;
#endif
}
static const exec_exclusion guard={enter,leave,0};
static void checkpoint(uint8_t phase){uint16_t f,index=(uint16_t)(fixture*3u+phase),patch=dsky_keyboard_flow_offsets[index],end=dsky_keyboard_flow_offsets[index+1u],n=dsky_keyboard_flow_numbers[fixture];
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){uint16_t expected=f<APOLLO_SHARED_WORDS ? before.words[f] : f==APOLLO_SHARED_WORDS ? (uint16_t)((n/128u)%8u*020u) : f==APOLLO_SHARED_WORDS+1u ? 0 : dsky_keyboard_flow_channel11(n);uint16_t actual=f<APOLLO_SHARED_WORDS ? image.words[f] : f==APOLLO_SHARED_WORDS ? binding.basic.control.superbank : f==APOLLO_SHARED_WORDS+1u ? binding.basic.control.interrupt_enabled : display.channel11;
 if(patch<end && dsky_keyboard_flow_patch[patch][0]==f)expected=dsky_keyboard_flow_patch[patch++][1];
 if(actual!=expected)++failures;
 }if(patch!=end)++failures;
}
static void observe(void *u,const apollo_overlay_state *s,agc_word pc){(void)u;(void)s;if(excluded!=1 || pc!=05155)++failures;checkpoint(2);++observed;}
uint16_t dsky_keyboard_flow_check(uint8_t n){uint16_t number;uint8_t token;p66_basic_binding *b=&binding.basic;
 if(n>=DSKY_KEYBOARD_FLOW_TARGET_COUNT)return 1;
 fixture=n;number=dsky_keyboard_flow_numbers[n];failures=0;observed=0;excluded=0;dsky_keyboard_flow_prepare(&image,number);before=image;memset(&binding,0,sizeof binding);state.alarm=0;dsky_display_reset(&display);display.channel11=dsky_keyboard_flow_channel11(number);b->arithmetic.image=&image;b->arithmetic.runtime=&runtime;b->arithmetic.producers=&guard;b->display=&display;b->control.superbank=(uint16_t)((number/128u)%8u*020u);b->control.interrupt_enabled=0;
#ifdef DSKY_KEYBOARD_FLOW_C64
 {uint16_t code=077777;uint8_t raw=0;token=enter(0);*(volatile uint8_t *)0x00c6=1;*(volatile uint8_t *)0x0277=number%2u ? 'N' : 'V';if(c64_dsky_keyboard_poll(&code,&raw)!=C64_DSKY_KEY_READY || code!=image.words[0] || raw!=(number%2u ? 'N' : 'V'))++failures;image.words[0]=code;leave(0,token);}
#endif
 token=enter(0);if(dsky_keycom(&image,&b->control)!=EXEC_OK)++failures;checkpoint(0);leave(0,token);
 p66_native_import_executive(&state,&image);if(exec_native_context_init(&runtime,&state,dsky_keyboard_mappings,DSKY_KEYBOARD_MAPPING_COUNT,&binding)!=EXEC_OK)++failures;
 /* Explicit original idle NUDIRECT semantic entry, following the proven
  * KEYCOM RESUME-entry checkpoint; does not fake interrupt restoration. */
 image.words[0]=2;image.words[1]=0;image.words[4]=02000;image.words[6]=(uint16_t)(02000u|(image.words[3]>>8));image.words[5]=03225;b->control.interrupt_enabled=1;
 if(dsky_keyboard_start(&binding)!=EXEC_OK)++failures;
 checkpoint(1);b->arithmetic.observe=observe;
 if(p66_basic_step(b)!=EXEC_IDLE || b->status!=0 || runtime.running!=0 || image.words[0167]!=077777 || image.words[067]!=077777 || excluded!=0 || observed!=1)++failures;
 return failures;
}
