/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "p66_basic.h"
#include "p66_basic_task_check.h"
#include "p66_basic_task_vectors.h"
static executive state;static exec_native runtime;static waitlist wait;static wl_native tasks;static p66_basic_binding binding;
static uint8_t enter(void *u){(void)u;
#ifdef P66_BASIC_TASK_C64
 uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;
#else
 return 0;
#endif
}
static void leave(void *u,uint8_t p){(void)u;
#ifdef P66_BASIC_TASK_C64
 if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");
#else
 (void)p;
#endif
}
static const exec_exclusion guard={enter,leave,0};
uint16_t p66_basic_task_check(uint8_t sample,apollo_overlay_state *m,apollo_overlay_state *before){uint16_t n=p66_basic_task_numbers[sample],f,patch=p66_basic_task_offsets[sample],end=p66_basic_task_offsets[sample+1u],errors=0;uint8_t token;p66_bias_io input;
 p66_basic_task_prepare(m,n);*before=*m;memset(&binding,0,sizeof binding);state.alarm=0;p66_native_import_executive(&state,m);binding.arithmetic.image=m;binding.arithmetic.runtime=&runtime;binding.arithmetic.producers=&guard;
 if(exec_native_context_init(&runtime,&state,p66_native_mappings,P66_NATIVE_MAPPING_COUNT,&binding)!=EXEC_OK)return 1;
 if(p66_basic_task_init(&binding,&tasks,&wait)!=WL_OK)return 1;
 input.superbank=(uint16_t)((n/432u)%4u*020u);input.interrupt_enabled=(uint16_t)(n%2u);binding.control=input;tasks.current.location=03263;tasks.current.bank=(uint16_t)(m->words[6]|input.superbank);
 token=enter(0);(void)p66_basic_task_mappings[1].call(&tasks,&binding);leave(0,token);
 if(binding.status!=p66_basic_task_results[sample])++errors;
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? input.superbank : input.interrupt_enabled;uint16_t actual=f<APOLLO_SHARED_WORDS ? m->words[f] : f==APOLLO_SHARED_WORDS ? binding.control.superbank : binding.control.interrupt_enabled;if(patch<end && p66_basic_task_patch[patch][0]==f)expected=p66_basic_task_patch[patch++][1];if(actual!=expected)++errors;}if(patch!=end)++errors;return errors;
}
