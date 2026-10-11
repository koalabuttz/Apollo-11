/* SPDX-License-Identifier: MIT */
#include "standard_lead_routines.h"
static enum exec_status stage(const agc_native_program *p,apollo_overlay_state *s,void *v,enum standard_lead_phase phase){p66_bias_io *io=v;agc_word operand=0,q;
 if(!p||!s||!io)return EXEC_OUT_OF_SCOPE;
 /* The inline CADR sits at the caller's BUF2 (E/BKCALL) or LOC+1 (E/CALL). */
 q=phase==STANDARD_LEAD_BANK_PREPARE?(agc_word)(s->words[0133]&07777u):phase==STANDARD_LEAD_INTERP_PREPARE?(agc_word)((s->words[0164]+1u)&07777u):0;
 if(q>=02000u&&!agc_transfer_operand(p,s,io->superbank,q,&operand))return EXEC_OUT_OF_SCOPE;
 return standard_lead_stage(s,STANDARD_LEAD_LM,phase,operand)?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status bkcall(const agc_native_program *p,apollo_overlay_state *s,void *io){return stage(p,s,io,STANDARD_LEAD_BANK_PREPARE);}
static enum exec_status return0(const agc_native_program *p,apollo_overlay_state *s,void *io){return stage(p,s,io,STANDARD_LEAD_BANK_RETURN0);}
static enum exec_status return1(const agc_native_program *p,apollo_overlay_state *s,void *io){return stage(p,s,io,STANDARD_LEAD_BANK_RETURN1);}
static enum exec_status return2(const agc_native_program *p,apollo_overlay_state *s,void *io){return stage(p,s,io,STANDARD_LEAD_BANK_RETURN2);}
static enum exec_status ecall(const agc_native_program *p,apollo_overlay_state *s,void *io){return stage(p,s,io,STANDARD_LEAD_INTERP_PREPARE);}
static enum exec_status resume(const agc_native_program *p,apollo_overlay_state *s,void *io){return stage(p,s,io,STANDARD_LEAD_INTERP_RESUME);}
static enum exec_status jobwak(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return standard_lead_wake_lm(s,io);}
static enum exec_status control(apollo_overlay_state *s,void *io,enum standard_lead_interp_boundary b){return s&&io&&standard_lead_interp_control(s,io,STANDARD_LEAD_LM,b)?EXEC_OK:EXEC_OUT_OF_SCOPE;}
static enum exec_status intpret(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return control(s,io,STANDARD_LEAD_CALL);}
static enum exec_status intpret2(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return control(s,io,STANDARD_LEAD_FINISH);}
static enum exec_status exit_op(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return control(s,io,STANDARD_LEAD_EXIT);}
enum exec_status lm_interpreter_rvq(apollo_overlay_state *s,p66_bias_io *io){return control(s,io,STANDARD_LEAD_RVQ);}
#define B4(z) (uint16_t)(4u*1024u+((z)-02000u))
#define FF(z) (uint16_t)(2048u+((z)-04000u))
static const agc_routine_entry basic[7]={{B4(02576),bkcall},{B4(02607),return0},{B4(02610),return1},{B4(02611),return2},{B4(02615),ecall},{B4(02627),resume},{B4(02631),jobwak}};
const agc_routine_table standard_lead_routines={basic,7};
static const agc_routine_entry interpreter[2]={{FF(06037),intpret},{FF(06041),intpret2}};
const agc_routine_table lm_interpreter_routines={interpreter,2};
static const agc_routine_entry interpretive[1]={{B4(02626),exit_op}};
const agc_routine_table standard_lead_interpretive_routines={interpretive,1};
/* Comanche055. The interpreter controls take the two latches standard_lead
 * uses; they are copied in and out of the mission's cm_native_io. */
static enum exec_status cm_stage(const agc_native_program *p,apollo_overlay_state *s,void *v,enum standard_lead_phase phase){cm_native_io *io=v;agc_word operand=0,q;
 if(!p||!s||!io)return EXEC_OUT_OF_SCOPE;
 q=phase==STANDARD_LEAD_BANK_PREPARE?(agc_word)(s->words[0133]&07777u):phase==STANDARD_LEAD_INTERP_PREPARE?(agc_word)((s->words[0164]+1u)&07777u):0;
 if(q>=02000u&&!agc_transfer_operand(p,s,io->superbank,q,&operand))return EXEC_OUT_OF_SCOPE;
 return standard_lead_stage(s,STANDARD_LEAD_CM,phase,operand)?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status cm_bkcall(const agc_native_program *p,apollo_overlay_state *s,void *io){return cm_stage(p,s,io,STANDARD_LEAD_BANK_PREPARE);}
static enum exec_status cm_return0(const agc_native_program *p,apollo_overlay_state *s,void *io){return cm_stage(p,s,io,STANDARD_LEAD_BANK_RETURN0);}
static enum exec_status cm_return1(const agc_native_program *p,apollo_overlay_state *s,void *io){return cm_stage(p,s,io,STANDARD_LEAD_BANK_RETURN1);}
static enum exec_status cm_return2(const agc_native_program *p,apollo_overlay_state *s,void *io){return cm_stage(p,s,io,STANDARD_LEAD_BANK_RETURN2);}
static enum exec_status cm_ecall(const agc_native_program *p,apollo_overlay_state *s,void *io){return cm_stage(p,s,io,STANDARD_LEAD_INTERP_PREPARE);}
static enum exec_status cm_resume(const agc_native_program *p,apollo_overlay_state *s,void *io){return cm_stage(p,s,io,STANDARD_LEAD_INTERP_RESUME);}
static enum exec_status cm_jobwak(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return standard_lead_wake_cm(s,io);}
static enum exec_status cm_control(apollo_overlay_state *s,void *v,enum standard_lead_interp_boundary b){cm_native_io *io=v;p66_bias_io latches;bool ok;
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 latches.superbank=io->superbank;latches.interrupt_enabled=io->interrupt_enabled;ok=standard_lead_interp_control(s,&latches,STANDARD_LEAD_CM,b);
 if(ok){io->superbank=latches.superbank;io->interrupt_enabled=latches.interrupt_enabled;}
 return ok?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status cm_intpret(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return cm_control(s,io,STANDARD_LEAD_CALL);}
static enum exec_status cm_intpret2(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return cm_control(s,io,STANDARD_LEAD_FINISH);}
static enum exec_status cm_exit_op(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return cm_control(s,io,STANDARD_LEAD_EXIT);}
enum exec_status cm_interpreter_rvq(apollo_overlay_state *s,cm_native_io *io){return cm_control(s,io,STANDARD_LEAD_RVQ);}
#define B13(z) (uint16_t)(013u*1024u+((z)-02000u))
static const agc_routine_entry cm_basic[7]={{B4(02606),cm_jobwak},{B13(02036),cm_bkcall},{B13(02047),cm_return0},{B13(02050),cm_return1},{B13(02051),cm_return2},{B13(02055),cm_ecall},{B13(02067),cm_resume}};
const agc_routine_table cm_standard_lead_routines={cm_basic,7};
static const agc_routine_entry cm_interpreter[2]={{FF(06006),cm_intpret},{FF(06010),cm_intpret2}};
const agc_routine_table cm_interpreter_routines={cm_interpreter,2};
static const agc_routine_entry cm_interpretive[1]={{B13(02066),cm_exit_op}};
const agc_routine_table cm_standard_lead_interpretive_routines={cm_interpretive,1};
