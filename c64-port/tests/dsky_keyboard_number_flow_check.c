/* SPDX-License-Identifier: MIT
 * Actual queued native keyboard jobs against complete original source images. */
#include <string.h>
#include "dsky_keyboard_number.h"
#include "dsky_keycom.h"
#include "dsky_keyboard_number_flow_check.h"
#include "dsky_keyboard_number_flow_vectors.h"
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
#include "c64_dsky_keyboard.h"
#endif
static apollo_overlay_state image;
#ifndef DSKY_KEYBOARD_NUMBER_FLOW_C64
static apollo_overlay_state before;
#endif
static executive state;
static exec_native runtime;
static dsky_display_state display;
static dsky_keyboard_binding binding;
static uint8_t fixture,excluded,observed;
static uint16_t failures;
static uint8_t enter(void *u){uint8_t previous;(void)u;
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
 __asm__ volatile("php\nsei\npla" : "=a"(previous) : : "memory");
#else
 previous=excluded;
#endif
 excluded=1;return previous;}
static void leave(void *u,uint8_t previous){(void)u;
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
 if((previous&4u)==0)__asm__ volatile("cli" : : : "memory");else __asm__ volatile("sei" : : : "memory");excluded=(uint8_t)((previous&4u)!=0);
#else
 excluded=previous;
#endif
}
static const exec_exclusion guard={enter,leave,0};
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
/* Original deterministic input data, never source-kernel behavior. This
 * regenerates the exact oracle input without a second4KB target image. */
static uint16_t input_word(uint16_t n,uint16_t address,uint16_t random_word){
 static const uint16_t regs[]={0,077777,037777,040000,0100000,0177777,0140000,1};
 static const uint16_t corners[]={0,077777,037777,040000};
 static const uint16_t cadrs[]={0,077777,1,040000};
 uint16_t eb=(uint16_t)((n/2u)%8u),base=(uint16_t)(0401u+(n%5u)*054u),relative=(uint16_t)(n%2u ? 8 : 9);
 if(address<8){switch(address){case 0:return 2;case 1:return regs[(n/2u)%8u];case 2:return regs[(n/16u)%8u];case 3:return (uint16_t)(eb*0400u);case 4:return 02000;case 5:return 03214;case 6:return (uint16_t)(02000u|eb);default:return 0;}}
 if(address==0777){uint16_t count=(uint16_t)(n%19u);return count>=15u ? (uint16_t)(count+1u) : count;}
 if(address==01000)return (uint16_t)(1u+(n/19u)%2u);
 if(address>=01001 && address<=01005)return (uint16_t)((n*41u+address)%(address<01003?100u:10000u));
 if(address>=01023 && address<=01035 && random_word==0)return 073777;
 if(address==067)return 077777;
 if(address==0120)return 0110;
 if(address==0121)return 0;
 if(address==0166)return base;
 if(address>=0154 && address<=0313 && (address-0154u)%12u==11)return 077777;
 if(address==base-1u)return 0;
 if(address>=01452 && address<=01457 && n<64){uint16_t k=(uint16_t)(address-01452u);return corners[(((uint32_t)n>>(2u*(k%3u)))+k)%4u];}
 if(address==01023+relative){switch((n/16u)%4u){case 0:return 073777;case 1:return 077777;default:break;}}
 if(address==01042)return cadrs[n%4u];
 return random_word;
}
#endif
static void checkpoint(uint8_t phase){uint16_t f,index=(uint16_t)(fixture*3u+phase),patch=dsky_keyboard_number_flow_offsets[index],end=dsky_keyboard_number_flow_offsets[index+1u],n=dsky_keyboard_number_flow_numbers[fixture];
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
 uint32_t seed=n+394u;
#endif
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){uint16_t expected=
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
 f<APOLLO_SHARED_WORDS ? (seed=seed*1664525u+1013904223u,input_word(n,f,(uint16_t)((seed>>16)&077777u))) :
#else
 f<APOLLO_SHARED_WORDS ? before.words[f] :
#endif
 f==APOLLO_SHARED_WORDS ? (uint16_t)((n/128u)%8u*020u) : f==APOLLO_SHARED_WORDS+1u ? 1 : dsky_keyboard_number_flow_channel11(n);uint16_t actual=f<APOLLO_SHARED_WORDS ? image.words[f] : f==APOLLO_SHARED_WORDS ? binding.basic.control.superbank : f==APOLLO_SHARED_WORDS+1u ? binding.basic.control.interrupt_enabled : display.channel11;
 if(patch<end && dsky_keyboard_number_flow_patch[patch][0]==f)expected=dsky_keyboard_number_flow_patch[patch++][1];
 if(actual!=expected)++failures;
 }if(patch!=end)++failures;
}
static void observe(void *u,const apollo_overlay_state *s,agc_word pc){(void)u;(void)s;if(excluded!=1 || pc!=05155)++failures;checkpoint(2);++observed;}
uint16_t dsky_keyboard_number_flow_check(uint8_t n){uint16_t number,channel15;
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
 uint8_t token;
#endif
 p66_basic_binding *b=&binding.basic;
 if(n>=DSKY_KEYBOARD_NUMBER_FLOW_TARGET_COUNT)return 1;
 fixture=n;number=dsky_keyboard_number_flow_numbers[n];channel15=dsky_keyboard_number_flow_key(number);failures=0;observed=0;excluded=0;dsky_keyboard_number_flow_prepare(&image,number);
#ifndef DSKY_KEYBOARD_NUMBER_FLOW_C64
 before=image;
#endif
 memset(&binding,0,sizeof binding);state.alarm=0;dsky_display_reset(&display);display.channel11=dsky_keyboard_number_flow_channel11(number);b->arithmetic.image=&image;b->arithmetic.runtime=&runtime;b->arithmetic.producers=&guard;b->display=&display;b->control.superbank=(uint16_t)((number/128u)%8u*020u);b->control.interrupt_enabled=1;
#ifdef DSKY_KEYBOARD_NUMBER_FLOW_C64
 {uint16_t code=077777;uint8_t raw=0;token=enter(0);*(volatile uint8_t *)0x00c6=1;*(volatile uint8_t *)0x0277=(uint8_t)('0'+number%10u);if(c64_dsky_keyboard_poll(&code,&raw)!=C64_DSKY_KEY_READY || code!=(dsky_keyboard_number_flow_key(number)) || raw!=((uint8_t)('0'+number%10u)))++failures;channel15=code;leave(0,token);}
#endif
 p66_native_import_executive(&state,&image);if(exec_native_context_init(&runtime,&state,dsky_keyboard_number_mappings,DSKY_KEYBOARD_MAPPING_COUNT,&binding)!=EXEC_OK)++failures;
 if(dsky_keyboard_interrupt(&binding,channel15,010067)!=EXEC_OK)++failures;
 checkpoint(0);
 if(dsky_keyboard_idle(&binding)!=EXEC_OK)++failures;
 checkpoint(1);b->arithmetic.observe=observe;
 if(p66_basic_step(b)!=EXEC_IDLE || b->status!=0 || runtime.running!=0 || image.words[0167]!=077777 || image.words[067]!=077777 || excluded!=0 || observed!=1)++failures;
 return failures;
}
