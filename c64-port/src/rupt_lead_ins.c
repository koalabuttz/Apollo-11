/* SPDX-License-Identifier: MIT */
#include "rupt_lead_ins.h"
#include "agc_image_ops.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#define W s->words
#define FF(z) (uint16_t)(2048u+((z)-04000u))

/* The two missions' io records share nothing but these two fields. */
typedef struct {agc_word *superbank,*interrupt_enabled;} rupt_io;
static rupt_io lm_io(void *v){p66_bias_io *io=v;rupt_io r={&io->superbank,&io->interrupt_enabled};return r;}
static rupt_io cm_io(void *v){cm_native_io *io=v;rupt_io r={&io->superbank,&io->interrupt_enabled};return r;}

typedef struct {agc_word vector,bbcon,handler;} banked_vector;
/* Fixed constants the vectors read (BBCON words at 04054-04065). */
static const banked_vector lm_banked[7]={{04020u,014106u,02000u},{04024u,010100u,03274u},{04030u,016107u,02332u},
 {04034u,010100u,03317u},{04040u,012100u,03506u},{04044u,052100u,03150u},{04050u,022102u,02275u}};
static const banked_vector cm_banked[7]={{04020u,014062u,02000u},{04024u,016060u,03673u},{04030u,016067u,02104u},
 {04034u,016060u,03716u},{04040u,012060u,03521u},{04044u,056067u,02504u},{04050u,004060u,05336u}};
static int in_rupt(const agc_native_program *p){return p&&p->hardware&&p->hardware->in_interrupt==1u;}
static int fixed(agc_word z){return z>=02000u&&z<=07777u;}
/* DXCH ARUPT; CAF k; XCH BBANK; TCF handler. */
static void banked(apollo_overlay_state *s,const banked_vector *b){ai_dxch(s,010u);ai_caf(s,b->bbcon);ai_xch(s,6u);W[5]=b->handler;}
/* EXTEND; DCA k; DTCB at `at`: Z,BB <- the 2CADR; A = at+1, L = the old BB. */
static void dtcb(apollo_overlay_state *s,unsigned k,agc_word at){agc_word z,bb;
 ai_dca(s,k);z=agc_correct(W[0]);bb=agc_correct(W[1]);
 W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=z;ai_put(s,6u,bb);
}
static const banked_vector *find(const banked_vector *t,agc_word z){unsigned i;for(i=0;i<7u;++i)if(t[i].vector==z)return &t[i];return 0;}
static enum exec_status vector(const agc_native_program *p,apollo_overlay_state *s,void *v,const banked_vector *table){const banked_vector *b;
 if(!s||!v||!in_rupt(p)||!(b=find(table,W[5])))return EXEC_OUT_OF_SCOPE;
 banked(s,b);return EXEC_OK;
}
static enum exec_status lm_vector(const agc_native_program *p,apollo_overlay_state *s,void *v){return vector(p,s,v,lm_banked);}
static enum exec_status cm_vector(const agc_native_program *p,apollo_overlay_state *s,void *v){return vector(p,s,v,cm_banked);}
/* GOPROG lead-in 04000: INHINT; CAF GOBB; XCH BBANK; TCF GOPROG. */
static enum exec_status goprog(apollo_overlay_state *s,rupt_io io,agc_word gobb,agc_word target){
 *io.interrupt_enabled=0;ai_caf(s,gobb);ai_xch(s,6u);W[5]=target;return EXEC_OK;
}
static enum exec_status lm_goprog(const agc_native_program *p,apollo_overlay_state *s,void *v){(void)p;return s&&v?goprog(s,lm_io(v),012103u,02667u):EXEC_OUT_OF_SCOPE;}
static enum exec_status cm_goprog(const agc_native_program *p,apollo_overlay_state *s,void *v){(void)p;return s&&v?goprog(s,cm_io(v),012063u,02526u):EXEC_OUT_OF_SCOPE;}
/* The 2CADR read by DCA at erasable k: its CADR in fixed memory. */
static int pair_ok(const apollo_overlay_state *s,unsigned k){return fixed(W[k]);}
/* LM T5RUPT 04010: DXCH ARUPT; EXTEND; DCA T5ADR; DTCB. */
static enum exec_status lm_t5(const agc_native_program *p,apollo_overlay_state *s,void *v){
 if(!s||!v||!in_rupt(p)||!pair_ok(s,01274u))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,010u);dtcb(s,01274u,04013u);return EXEC_OK;
}
/* CM T6RUPT 04004: DXCH ARUPT; EXTEND; DCA T6LOC; DTCB. */
static enum exec_status cm_t6(const agc_native_program *p,apollo_overlay_state *s,void *v){
 if(!s||!v||!in_rupt(p)||!pair_ok(s,01310u))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,010u);dtcb(s,01310u,04007u);return EXEC_OK;
}
/* CM T5RUPT 04065: EXTEND; BZMF NOQBRSM; EXTEND; DCA T5LOC; DTCB. */
static void cm_t5_body(apollo_overlay_state *s){if(ai_bzmf(s))W[5]=05337u;else dtcb(s,01312u,04071u);}
/* CM T5RUPT vector 04010: DXCH ARUPT; CS TIME5; AD .5SEC; TCF T5RUPT. */
static enum exec_status cm_t5(const agc_native_program *p,apollo_overlay_state *s,void *v){
 if(!s||!v||!in_rupt(p)||!pair_ok(s,01312u))return EXEC_OUT_OF_SCOPE;
 if(W[5]==04010u){ai_dxch(s,010u);ai_cs(s,030u);ai_adf(s,062u);}
 cm_t5_body(s);return EXEC_OK;
}
/* CM RESUME 05333 (EXTEND; QXCH QRUPT), NOQRSM 05335 (CA BANKRUPT), 05336
 * (XCH BBANK), NOQBRSM 05337 (DXCH ARUPT; RELINT; RESUME). */
static enum exec_status cm_resume(const agc_native_program *p,apollo_overlay_state *s,void *v){cm_native_io *io=v;agc_word z;
 if(!s||!io||!in_rupt(p)||W[015]==0u||W[015]>010000u)return EXEC_OUT_OF_SCOPE;
 z=W[5];
 if(z==05335u||z==05333u){if(W[016]&(agc_word)~076167u)return EXEC_OUT_OF_SCOPE;}
 if(z==05333u)ai_qxch(s,012u);
 if(z==05333u||z==05335u)ai_ca(s,016u);
 if(z!=05337u)ai_xch(s,6u);
 ai_dxch(s,010u);io->interrupt_enabled=1;
 W[5]=(agc_word)(W[015]-1u);W[7]=0;p->hardware->in_interrupt=0;return EXEC_OK;
}
static const agc_routine_entry lm_entries[9]={
 {FF(04000u),lm_goprog},{FF(04010u),lm_t5},{FF(04020u),lm_vector},{FF(04024u),lm_vector},{FF(04030u),lm_vector},
 {FF(04034u),lm_vector},{FF(04040u),lm_vector},{FF(04044u),lm_vector},{FF(04050u),lm_vector}};
const agc_routine_table lm_lead_in_routines={lm_entries,9};
static const agc_routine_entry cm_entries[15]={
 {FF(04000u),cm_goprog},{FF(04004u),cm_t6},{FF(04010u),cm_t5},{FF(04020u),cm_vector},{FF(04024u),cm_vector},{FF(04030u),cm_vector},
 {FF(04034u),cm_vector},{FF(04040u),cm_vector},{FF(04044u),cm_vector},{FF(04050u),cm_vector},{FF(04065u),cm_t5},
 {FF(05333u),cm_resume},{FF(05335u),cm_resume},{FF(05336u),cm_resume},{FF(05337u),cm_resume}};
const agc_routine_table cm_lead_in_routines={cm_entries,15};
