/* SPDX-License-Identifier: MIT */
#include <string.h>
#define AGC_SITES_CALLEES 1
#include "cm_interbank_routines_cases.h"
#include "../src/cm_interbank_sites.inc"
uint32_t cm_ibr_cases(void){return (uint32_t)CM_INTERBANK_SITES_COUNT*CM_IBR_CONTEXTS;}
uint16_t cm_ibr_site(uint32_t n){return cm_interbank_sites[n%CM_INTERBANK_SITES_COUNT].physical;}
enum cm_interbank_complete_entry cm_ibr_entry(agc_word callee){return callee==04662u?CM_IC_BANKCALL:callee==04740u?CM_IC_IBNKCALL:callee==04701u?CM_IC_POSTJUMP:CM_IC_USPRCADR;}
const agc_native_program cm_ibr_program={&cm_interbank_routines,1,0,0,cm_interbank_sites,CM_INTERBANK_SITES_COUNT,AGC_TRANSFER_NONE,0164,0165,0,0};
agc_word cm_ibr_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){static const agc_word a[CM_IBR_CONTEXTS]={012345,0177777,040000,0100001};
 uint16_t site=(uint16_t)(n%CM_INTERBANK_SITES_COUNT),ctx=(uint16_t)(n/CM_INTERBANK_SITES_COUNT),p=cm_interbank_sites[site].physical,bank=(uint16_t)(p/1024u),offset=(uint16_t)(p%1024u),i;uint32_t x=(uint32_t)(0x9e3779b9ul*(n+1u));
 memset(s,0,sizeof *s);memset(io,0,sizeof *io);
 for(i=010;i<APOLLO_SHARED_WORDS;++i){x=(uint32_t)(x*1103515245ul+12345ul);s->words[i]=(agc_word)((x>>9)&077777u);}
 s->words[0]=a[ctx%CM_IBR_CONTEXTS];s->words[1]=(agc_word)(x&0177777u);s->words[3]=(agc_word)((ctx*3u%8u)*0400u);io->interrupt_enabled=(uint16_t)(ctx&1u);
 if(bank==2u||bank==3u){s->words[4]=(agc_word)(((ctx*5u)%030u)<<10);s->words[2]=(agc_word)(bank*02000u+offset);}
 else if(bank>=040u){s->words[4]=(agc_word)((bank-8u)<<10);io->superbank=0100u;s->words[2]=(agc_word)(02000u+offset);}
 else{s->words[4]=(agc_word)(bank<<10);s->words[2]=(agc_word)(02000u+offset);}
 s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=cm_interbank_sites_callee[site];
 return s->words[5];
}
static enum exec_status test_callee(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;s->words[0]=07070;s->words[5]=s->words[2];return EXEC_OK;}
static apollo_overlay_state run_image;
static uint16_t sum_a,sum_b;
static void mix(uint16_t w){sum_a=(uint16_t)(sum_a+w);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_ibr_run(uint32_t n){cm_native_io io;agc_word callee=cm_ibr_input(&run_image,&io,n),cadr=0;enum cm_interbank_complete_entry entry=cm_ibr_entry(callee);enum exec_status status;uint16_t steps=0,i;
 agc_routine_entry callee_entry[1];agc_routine_table tables[2];agc_native_program program=cm_ibr_program;
 if(entry==CM_IC_BANKCALL||entry==CM_IC_IBNKCALL)(void)agc_transfer_operand(&cm_ibr_program,&run_image,io.superbank,run_image.words[2],&cadr);
 callee_entry[0].physical=agc_fixed_physical((agc_word)(02000u+(cadr&01777u)),cadr,io.superbank);callee_entry[0].call=test_callee;
 tables[0]=cm_interbank_routines;tables[1].entries=callee_entry;tables[1].count=1;program.basic=tables;program.basic_count=(entry==CM_IC_BANKCALL||entry==CM_IC_IBNKCALL)?2u:1u;
 status=agc_transfer_run(&program,&run_image,&io,&io.superbank,(entry==CM_IC_BANKCALL||entry==CM_IC_IBNKCALL)?(uint16_t)(cm_ibr_site(n)+1u):(uint16_t)(AGC_TRANSFER_NONE-1u),16,&steps);
 sum_a=0;sum_b=0;mix((uint16_t)status);mix(steps);for(i=0;i<APOLLO_SHARED_WORDS;++i)mix(run_image.words[i]);mix(io.superbank);mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
