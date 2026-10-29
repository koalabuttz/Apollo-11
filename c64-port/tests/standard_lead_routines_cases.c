/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "standard_lead_routines_cases.h"
#include "standard_lead_cases.h"
#include "standard_lead_fixed_inputs.h"
static agc_site_operand stage_ops[64],trip_ops[64];
static enum exec_status basic_callee(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;s->words[0]=05151;s->words[5]=s->words[2];return EXEC_OK;}
static enum exec_status interp_callee(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;s->words[0154]=06161;return lm_interpreter_rvq(s,io);}
/* Callees sit at LM_SLR_CALLEE_BANK,2000+0100+k%64 for all k. */
static agc_routine_entry basic_callees[64],interp_callees[64];
static agc_routine_table basic_tables[3],interp_tables[2];
static agc_native_program stage_program,trip_program;
static int ready;
static void setup(void){uint16_t i;
 for(i=0;i<64u;++i){stage_ops[i].physical=(uint16_t)(2048u+i);stage_ops[i].word=standard_lead_fixed_inputs[STANDARD_LEAD_LM][i];
  trip_ops[i].physical=(uint16_t)(2048u+i);trip_ops[i].word=(agc_word)((LM_SLR_CALLEE_BANK<<10)|(0100u+i));
  basic_callees[i].physical=interp_callees[i].physical=(uint16_t)(LM_SLR_CALLEE_BANK*1024u+0100u+i);basic_callees[i].call=basic_callee;interp_callees[i].call=interp_callee;}
 basic_tables[0]=standard_lead_routines;basic_tables[1]=lm_interbank_routines;basic_tables[2]=lm_interpreter_routines;
 interp_tables[0]=standard_lead_interpretive_routines;interp_tables[1].entries=interp_callees;interp_tables[1].count=64;
 stage_program.basic=basic_tables;stage_program.basic_count=3;stage_program.interpretive=interp_tables;stage_program.interpretive_count=1;
 stage_program.operands=stage_ops;stage_program.operand_count=64;stage_program.interpreter_fetch=LM_INTERPRETER_FETCH;stage_program.loc=0164;stage_program.bankset=0165;
 trip_program=stage_program;trip_program.operands=trip_ops;trip_program.interpretive_count=2;
 ready=1;
}
static agc_routine_table trip_basic[4];
enum lm_slr_kind lm_slr_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,const agc_native_program **program,agc_word *fixed){enum standard_lead_phase phase;uint16_t k=(uint16_t)(n%512u),address,physical;agc_word cadr;
 if(!ready)setup();
 memset(io,0,sizeof *io);
 if(n<3072u){standard_lead_input(s,STANDARD_LEAD_LM,n,&phase,fixed);*program=&stage_program;io->interrupt_enabled=(uint16_t)((n/7u)&1u);return LM_SLR_STAGE;}
 standard_lead_input(s,STANDARD_LEAD_LM,(uint16_t)((n<3584u?0u:4u)*512u+k),&phase,fixed);
 /* Basic callees join the trip program's tables. */
 trip_basic[0]=standard_lead_routines;trip_basic[1]=lm_interbank_routines;trip_basic[2]=lm_interpreter_routines;trip_basic[3].entries=basic_callees;trip_basic[3].count=64;
 trip_program.basic=trip_basic;trip_program.basic_count=4;*program=&trip_program;
 cadr=(agc_word)((LM_SLR_CALLEE_BANK<<10)|(0100u+k%64u));
 address=(uint16_t)((n<3584u?s->words[0133]:(agc_word)(s->words[0164]+1u))&07777u);
 if(address<01400u)s->words[address]=cadr;else if(address<02000u){physical=(uint16_t)(s->words[3]+(address&0377u));s->words[physical]=cadr;}
 s->words[0120]=(agc_word)(0401u+054u*(k%5u));s->words[0]&=077777u;
 *fixed=0;return n<3584u?LM_SLR_BKCALL:LM_SLR_ECALL;
}
enum exec_status lm_slr_run(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,uint16_t stop,uint16_t max_steps,uint16_t *steps){const agc_native_program *p;agc_word fixed;
 (void)lm_slr_input(s,io,n,&p,&fixed);
 return agc_transfer_run(p,s,io,&io->superbank,stop,max_steps,steps);
}
uint32_t lm_slr_digest(enum exec_status status,uint16_t steps,const apollo_overlay_state *s,const p66_bias_io *io){uint16_t a=0,b=0,i;
#define MIX(w) do{a=(uint16_t)(a+(w));b=(uint16_t)(b+a);}while(0)
 MIX((uint16_t)status);MIX(steps);for(i=0;i<APOLLO_SHARED_WORDS;++i)MIX(s->words[i]);MIX(io->superbank);MIX(io->interrupt_enabled);
 return (uint32_t)b<<16|a;
}
