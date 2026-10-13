/* SPDX-License-Identifier: MIT */
#include "waitlist_entries_check.h"
#include "waitlist_entries_cases.h"
#include "lm_exec_routines.h"
#include "cm_exec_routines.h"
#include "lm_t6rupt_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#if defined(__mos__)
/* The C64 diagnostic banks BASIC and KERNAL ROM out ($01=$35), so the state lives in RAM under them. */
#define image (*(apollo_overlay_state *)0xe000u)
#else
static apollo_overlay_state image;
#endif
static uint32_t sum(void){uint16_t i,a=0,b=0;for(i=0;i<APOLLO_SHARED_WORDS;++i){a=(uint16_t)(a+image.words[i]);b=(uint16_t)(b+a);}return (uint32_t)b<<16|a;}
#if !defined(WLE_ONLY)
#define WLE_TABLE(k) ((k)==0u||(k)==2u?lm_exec_task_routines:(k)==4u?lm_t6rupt_routines:cm_exec_routines)
#elif WLE_ONLY==0||WLE_ONLY==2
#define WLE_TABLE(k) lm_exec_task_routines
#elif WLE_ONLY==4
#define WLE_TABLE(k) lm_t6rupt_routines
#elif defined(CM_EXEC_DLY2_ONLY)
#define WLE_TABLE(k) cm_exec_dly2_routines
#else
#define WLE_TABLE(k) cm_exec_routines
#endif
static uint16_t refuses(uint16_t n){static wle_io_storage io;agc_hardware hw;agc_routine_table t[1];agc_native_program p={0};uint16_t steps;
 uint8_t kind=wle_kind(n);agc_word z;enum exec_status st;
 wle_input(&image,&io,&hw,kind,wle_case_base_of(n));
 if(kind==0u||kind==1u)image.words[7]=1;
 else if(kind==2u)((p66_bias_io *)&io)->interrupt_enabled=1;
 else if(kind==3u)((cm_native_io *)&io)->interrupt_enabled=1;
 else hw.in_interrupt=0;
 /* DLY2 -1 itself, not DELAYJOB. */
 if(kind==2u||kind==3u)image.words[5]=kind==2u?LM_EXEC_DLY2_M1:05255u;
#ifdef WLE_ONLY
 if(kind!=WLE_ONLY)return 0;
#endif
 t[0]=WLE_TABLE(kind);
 p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;p.hardware=&hw;
#if !defined(WLE_ONLY)||WLE_ONLY==1||WLE_ONLY==3
 if(kind==1u||kind==3u)p.operands=cm_exec_site_table(&p.operand_count);
#endif
 uint32_t was=sum();z=image.words[5];
 st=agc_transfer_run(&p,&image,&io,(kind==1u||kind==3u)?(const agc_word *)&((cm_native_io *)&io)->superbank:&((p66_bias_io *)&io)->superbank,AGC_TRANSFER_NONE,1,&steps);
 return st==EXEC_OUT_OF_SCOPE&&image.words[5]==z&&sum()==was;}
uint16_t wle_check(uint16_t n,uint16_t *checks){uint16_t f=0;
 ++*checks;if(wle_digest(&image,n)!=wle_frozen_digest(n))++f;
 if(!n||wle_kind(n)!=wle_kind((uint16_t)(n-1u))){++*checks;if(!refuses(n))++f;}
 return f;}
