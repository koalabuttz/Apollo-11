/* SPDX-License-Identifier: MIT */
#include "sp_routines.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#if RK_LM
#include "lm_single_trig.h"
#define SPCOS 05032u
#define TRIG(s,c) (lm_single_trig((s),(c))!=LM_SINGLE_TRIG_OK||lm_single_trig_return(s)!=LM_SINGLE_TRIG_OK)
#define TABLE lm_sp_routines
#else
#include "cm_single_return.h"
#define SPCOS 05074u
#define TRIG(s,c) (cm_single_trig((s),(c))!=CM_TRIG_OK||cm_single_return(s)!=CM_TRIG_OK)
#define TABLE cm_sp_routines
#endif
/* The return check comes first, so a refusal writes nothing. */
static enum exec_status run(apollo_overlay_state *s,uint8_t cosine){
 if(s->words[2]<02000u||s->words[2]>07777u)return EXEC_OUT_OF_SCOPE;
 return TRIG(s,cosine)?EXEC_OUT_OF_SCOPE:EXEC_OK;}
static enum exec_status spcos(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return run(s,1);}
static enum exec_status spsin(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return run(s,0);}
static const agc_routine_entry entries[2]={{FF(SPCOS),spcos},{FF(SPCOS+1u),spsin}};
const agc_routine_table TABLE={entries,2};
