/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_guidance_live_cases.h"
#include "p66_guidance_invoke.h"
typedef struct{unsigned phase,stage;uint16_t number;} observation;
static uint8_t enter(void *u){(void)u;return 0;}
static void leave(void *u,uint8_t p){(void)u;(void)p;}
static uint16_t call(void *u,apollo_overlay_state *m,uint16_t selector){(void)u;return selector<=5?p66_native_entry(m,selector):p66_native_guidance_entry(m,selector);}
static void observe(void *u,const apollo_overlay_state *m,agc_word loc){observation *o=u;unsigned i,higher=o->stage>o->phase;uint16_t words[PGL_FULL_WORDS];if(loc!=(higher?03353:pgl_boundaries[o->stage])){fprintf(stderr,"invalid pending observer phase%u stage%u LOC%o\n",o->phase,o->stage,loc);return;}if(o->stage>=o->phase){pgl_words(m,words,higher?0:o->phase);printf("%u %u %u",o->number,o->phase,higher);for(i=0;i<PGL_FULL_WORDS;++i)printf(" %u",words[i]);puts("");}++o->stage;}
int main(void){uint16_t n;unsigned phase;static const exec_exclusion guard={enter,leave,0};for(n=0;n<16;++n)for(phase=1;phase<PGL_STAGES;++phase){apollo_overlay_state image;executive state;exec_native runtime;unsigned step;observation o={phase,0,n};p66_guidance_modules modules={call,0};p66_native_binding binding={&image,&runtime,&guard,observe,&o,P66_NATIVE_OK,p66_guidance_invoke,&modules};pgl_prepare(&image,&state,n);pgl_prepare_high(&image,n);p66_native_import_executive(&state,&image);if(p66_native_guidance_init(&runtime,&state,&binding)!=EXEC_OK)return 2;runtime.running=1;runtime.current.location=03275;runtime.current.bank=n&1u?064067:064007;for(step=0;step<=phase+1;++step){if(step==phase)image.words[0067]=12;if(p66_native_step(&binding)!=EXEC_OK)return 3;if(step==phase&&(runtime.current_kind!=EXEC_CONTEXT_BASIC||runtime.current.location!=03275))return 4;}if(o.stage!=phase+2)return 5;}return 0;}
