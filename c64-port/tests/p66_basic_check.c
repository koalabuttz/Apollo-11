/* SPDX-License-Identifier: MIT */
#include "p66_basic_check.h"
#include "p66_basic.h"
#ifdef P66_BASIC_C64_SCREEN
#include "p66_basic_vectors.h"
#else
#include "p66_mode_display_vectors.h"
#define P66_BASIC_SOURCE_CASES 64u
#endif
#include <string.h>
#ifdef P66_BASIC_C64_SCREEN
#include "c64_dsky_display.h"
#endif
static apollo_overlay_state image;
static uint8_t expected_case;
static uint16_t expected_wdret;
static p66_bias_io expected_control;
#ifdef P66_BASIC_C64_SCREEN
/* Same reserved transient Executive lease as the native disk driver;
 * software stack owns C800..CFFF, not this source-state view. */
typedef char executive_view_fits[(sizeof(executive)<=0x300u)?1:-1];
#define state (*(executive *)(uintptr_t)0xc100)
#else
static executive state;
#endif
static exec_native runtime;
static wl_native task_runtime;
static waitlist task_view;
static dsky_display_state display;
static p66_basic_binding binding;
static exec_context_mapping combined[P66_NATIVE_MAPPING_COUNT+P66_BASIC_MAPPING_COUNT];
static uint16_t errors;
/* At most64 immutable fixture callbacks; one byte is sufficient. */
static uint8_t observed;
static uint8_t excluded;
static uint8_t enter(void *user){uint8_t previous;(void)user;
#ifdef P66_BASIC_C64_SCREEN
 __asm__ volatile("php\nsei\npla" : "=a"(previous) : : "memory");
#else
 previous=excluded;
#endif
 excluded=1;return previous;}
static void leave(void *user,uint8_t previous){(void)user;
#ifdef P66_BASIC_C64_SCREEN
 if((previous&4u)==0)__asm__ volatile("cli" : : : "memory");else __asm__ volatile("sei" : : : "memory");
 excluded=(uint8_t)((previous&4u)!=0);
#else
 excluded=previous;
#endif
}
static const exec_exclusion guard={enter,leave,0};
/* Replay only immutable original input data, then apply frozen source deltas.
 * This checks every word while avoiding a second4KB image on the C64. */
static uint16_t input_word(uint16_t n,uint16_t address,uint16_t random_word){
 static const uint16_t corners[]={0,077777,037777,040000};
 static const uint16_t modes[]={077777,077776,040000,040001};
 static const uint16_t codes[]={025,003,031,033,017,036,034,023,035,037};
 uint16_t base=(uint16_t)(0401u+(n%5u)*054u),mode=(uint16_t)(n%104u),same=(uint16_t)(04000u+((uint32_t)codes[(mode%100u)/10u]<<5)+codes[mode%10u]);
 if(address<8){static const uint16_t regs[]={0,0,01030,01000,060000,03534,060002,0};return regs[address];}
 if(address==067)return 0;
 if(address==0120 || address==0166)return base;
 if(address>=0154 && address<=0313 && (address-0154u)%12u==11)return address==0167 ? 030110 : 077777;
 if(address==base-1u)return 0;
 if(address>=01452 && address<=01457 && n<64){uint16_t k=(uint16_t)(address-01452u);return corners[(((uint32_t)n>>(2u*(k%3u)))+k)%4u];}
 if(address==01011)return mode<100 ? mode : modes[mode-100u];
 if(address==01035){switch((n/104u)%8u){case 0:return same;case 1:return (uint16_t)(same^077777u);case 2:return 04321;case 3:return 073456;case 4:return 077777;case 5:return 04000;case 6:return 040000;default:return 037777;}}
 return random_word;
}
static void observe(void *user,const apollo_overlay_state *s,agc_word pc){uint16_t i,number=p66_mode_display_numbers[expected_case],patch=p66_mode_display_offsets[expected_case],end=p66_mode_display_offsets[expected_case+1u];uint32_t seed=number+394u;(void)user;
 if(excluded!=1 || pc!=05155){++errors;}
 for(i=0;i<APOLLO_SHARED_WORDS;++i){uint16_t expected;seed=seed*1664525u+1013904223u;expected=input_word(number,i,(uint16_t)((seed>>16)&077777u));if(patch<end && p66_mode_display_patch[patch][0]==i)expected=p66_mode_display_patch[patch++][1];if(s->words[i]!=expected){++errors;}if(i==0115)expected_wdret=expected;}
 if(binding.control.superbank!=expected_control.superbank || binding.control.interrupt_enabled!=expected_control.interrupt_enabled){++errors;}
 ++observed;
}
static void fresh(void){uint8_t i;memset(&image,0,sizeof image);exec_init(&state);p66_native_export_executive(&image,&state);wl_init(&task_view);image.words[026]=task_view.time3;image.words[0734]=task_view.ruptagn;for(i=0;i<8;++i)image.words[01400+i]=task_view.interval[i];for(i=0;i<9;++i){image.words[01410+2u*i]=task_view.task[i].location;image.words[01411+2u*i]=task_view.task[i].bank;}}
static void bind(void){uint8_t i;memset(&binding,0,sizeof binding);binding.arithmetic.image=&image;binding.arithmetic.runtime=&runtime;binding.arithmetic.producers=&guard;binding.tasks=&task_runtime;binding.display=&display;
 for(i=0;i<P66_NATIVE_MAPPING_COUNT;++i)combined[i]=p66_native_mappings[i];
 for(i=0;i<P66_BASIC_MAPPING_COUNT;++i)combined[P66_NATIVE_MAPPING_COUNT+i]=p66_basic_mappings[i];
 state.alarm=0;p66_native_import_executive(&state,&image);if(exec_native_context_init(&runtime,&state,combined,P66_NATIVE_MAPPING_COUNT+P66_BASIC_MAPPING_COUNT,&binding)!=EXEC_OK)++errors;if(p66_basic_task_init(&binding,&task_runtime,&task_view)!=WL_OK)++errors;dsky_display_reset(&display);
}
uint16_t p66_basic_check(void){uint8_t n,i;uint16_t patch;exec_address task={03263,064067};enum wl_status ws;
 errors=0;observed=0;excluded=0;
 for(n=0;n<P66_BASIC_SOURCE_CASES;++n){
  p66_mode_display_prepare(&image,p66_mode_display_numbers[n]);
  /* Source-unowned Executive allocation words use genuine valid idle slots;
   * frozen source deltas still compare every physical word before ENDOFJOB. */
  for(i=0;i<8;++i)image.words[0154+i*12u+11u]=077777;
  image.words[0167]=030110;image.words[067]=0;expected_case=n;expected_control.superbank=0100;expected_control.interrupt_enabled=(uint16_t)(p66_mode_display_numbers[n]%2u);
  for(patch=p66_mode_display_offsets[n];patch<p66_mode_display_offsets[n+1u];++patch){uint16_t f=p66_mode_display_patch[patch][0];if(f==APOLLO_SHARED_WORDS)expected_control.superbank=p66_mode_display_patch[patch][1];else if(f==APOLLO_SHARED_WORDS+1u)expected_control.interrupt_enabled=p66_mode_display_patch[patch][1];}
  bind();image.words[3]=03400;image.words[4]=064000;image.words[6]=064007;binding.control.superbank=060;binding.control.interrupt_enabled=(uint16_t)(p66_mode_display_numbers[n]%2u);binding.arithmetic.observe=observe;runtime.current.location=03534;runtime.current.bank=060102;runtime.running=1;
  {enum exec_status st=p66_basic_step(&binding);if(st!=EXEC_IDLE || runtime.running!=0 || image.words[0167]!=077777 || image.words[067]!=077777 || excluded!=0){++errors;}}
  /* WDRET shares interpreter INTBIT15; basic completion must preserve it. */
  if(image.words[0115]!=expected_wdret){++errors;}
 }
 if(observed!=P66_BASIC_SOURCE_CASES)++errors;
 /* Source-mode66 -> real dirty relay -> actual source channel010 -> C64 glyph. */
 fresh();image.words[3]=01000;image.words[4]=060000;image.words[6]=060002;image.words[5]=03534;image.words[0167]=030110;image.words[067]=0;image.words[01011]=66;image.words[01035]=04000;
 for(i=0;i<11;++i)image.words[01023+i]=04000;
 bind();binding.control.superbank=0100;runtime.current.location=03534;runtime.current.bank=060102;runtime.running=1;
 if(p66_basic_step(&binding)!=EXEC_IDLE || image.words[01016]!=1)++errors;
 image.words[0]=0;image.words[2]=02070;image.words[3]=01000;image.words[4]=014000;image.words[6]=014002;image.words[5]=02024;image.words[0776]=10;
 if(p66_basic_relay(&binding)!=P66_BIAS_OK || display.relay[10]!=(055634u&03777u))++errors;
 #ifdef P66_BASIC_C64_SCREEN
 c64_dsky_display_render(&display,1);
 if(((volatile uint8_t *)0x0400)[143]!=54 || ((volatile uint8_t *)0x0400)[144]!=54 || (((volatile uint8_t *)0xd800)[143]&15u)!=5)++errors;
#endif
 /* Authoritativeimage wins if a stale transient waitlist view is poisoned. */
 fresh();image.words[026]=030000;bind();task_view.time3=01234;
 if(p66_basic_schedule(&binding,100,task)!=WL_OK || image.words[01410]!=03263 || image.words[01411]!=064067 || image.words[026]!=037634)++errors;
 if(p66_basic_advance(&binding,99)!=WL_OK || task_runtime.pending!=0)++errors;
 if(p66_basic_advance(&binding,1)!=WL_OK || task_runtime.pending!=1)++errors;
 ws=p66_basic_task_step(&binding);
 if((ws!=WL_IDLE && ws!=WL_OK) || binding.status!=EXEC_OK || image.words[0167]!=022401 || image.words[0164]!=03275 || image.words[0165]!=064067 || image.words[0400]!=0 || image.words[5]!=05261 || excluded!=0){++errors;}
 p66_native_import_executive(&state,&image);
 if(exec_native_start(&runtime)!=EXEC_OK || runtime.current.location!=03275 || runtime.current.bank!=064067)++errors;
 /* The next callback is the genuine translated RODCOMP, not a test stub.
  * It is dispatched by the complete arithmetic overlay driver separately. */
 return errors;
}
