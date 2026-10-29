/* SPDX-License-Identifier: MIT */
/* SPSIN/SPCOS on the transfer ABI (src/sp_routines.c): every frozen yaAGC
 * full-return state of lm-single-trig (selectors 2, 3) or cm-single-return,
 * entered through the routine table at the original address, must end in
 * the original's final image; a return address in erasable is refused
 * with the image unchanged. Build with -DRK_LM=1 or -DRK_CM=1. */
#include <stdio.h>
#include <string.h>
#include "sp_routines.h"
#if RK_LM
#include "lm_single_trig_cases.h"
#define LM_SINGLE_TRIG2_FROZEN_IMPLEMENTATION
#define LM_SINGLE_TRIG3_FROZEN_IMPLEMENTATION
#include "lm_single_trig_vectors.h"
#define TABLE lm_sp_routines
#define MISSION "lm"
#define FIRST 2u
#define LAST 3u
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){return (uint8_t)(selector==2u?lm_single_trig2_reference_begin(r,row):lm_single_trig3_reference_begin(r,row));}
static void input(apollo_overlay_state *s,agc_word *irq,uint16_t n,uint8_t selector){lm_single_trig_input(s,irq,n,(uint8_t)(selector%2u));}
#else
#include "cm_single_return_cases.h"
#define CM_SINGLE_RETURN0_FROZEN_IMPLEMENTATION
#define CM_SINGLE_RETURN1_FROZEN_IMPLEMENTATION
#include "cm_single_return_vectors.h"
#define TABLE cm_sp_routines
#define MISSION "cm"
#define FIRST 0u
#define LAST 1u
static uint8_t begin(frozen_words *r,uint8_t selector,unsigned row){return (uint8_t)(selector==0u?cm_single_return0_reference_begin(r,row):cm_single_return1_reference_begin(r,row));}
static void input(apollo_overlay_state *s,agc_word *irq,uint16_t n,uint8_t selector){(void)selector;cm_single_return_input(s,irq,n);}
#endif
static apollo_overlay_state image,expected,before;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static uint8_t finish(frozen_words *r){uint16_t pad;while(frozen_words_next(r,&pad))if(pad)return 0;return frozen_words_finished(r);}
int main(void){agc_native_program p={&TABLE,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint8_t selector;unsigned row,checks=0,fail=0;
 for(selector=FIRST;selector<=LAST;++selector)for(row=0;row<32u;++row){frozen_words reader;uint16_t n,expected_irq;agc_word irq;agc_routine call;
  checks+=2u;
  if(!begin(&reader,selector,row)||!frozen_words_next(&reader,&n)){fail+=2u;continue;}
  input(&image,&irq,n,selector);expected=image;
#if !RK_LM
  if(!patch(&reader,&expected)){fail+=2u;continue;}
#endif
  if(!frozen_words_next(&reader,&expected_irq)||!patch(&reader,&expected)||!finish(&reader)){fail+=2u;continue;}
  before=image;
  call=agc_transfer_resolve(&p,&image,0);
  if(!call||call(&p,&image,0)!=EXEC_OK||memcmp(&image,&expected,sizeof image))++fail;
  /* The same entry returning into erasable: refused, nothing written. */
  image=before;image.words[2]=01777u;before=image;
  if(!call||call(&p,&image,0)!=EXEC_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image))++fail;
 }
#if !RK_LM
 /* The CM entry pack's ride copy holds the same two entries. */
 {
     const agc_routine_table *h = &cm_sp_hot_routines, *f = &cm_sp_routines;
     uint16_t i;
     if (h->count != f->count) ++fail;
     for (i = 0; i < h->count && i < f->count; ++i)
         if (h->entries[i].physical != f->entries[i].physical || h->entries[i].call != f->entries[i].call) ++fail;
 }
#endif
 printf("%s: sp-routines %s: %u full-return states and %u refusals; %u checks, %u failures\n",fail?"FAIL":"PASS",MISSION,checks/2u,checks/2u,checks,fail);
 return fail!=0u;
}
