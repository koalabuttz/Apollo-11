/* SPDX-License-Identifier: MIT */
#include "sp_routines.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
/* The mission's table under RK_LM or RK_CM (the resident); both when
 * neither is set (the host table index and tests). The return check comes
 * first, so a refusal writes nothing. */
#define SP_TABLE(table,spcos_z,trig) \
 static enum exec_status table##_run(apollo_overlay_state *s,uint8_t cosine){ \
  if(s->words[2]<02000u||s->words[2]>07777u)return EXEC_OUT_OF_SCOPE; \
  return trig(s,cosine)?EXEC_OUT_OF_SCOPE:EXEC_OK;} \
 static enum exec_status table##_cos(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return table##_run(s,1);} \
 static enum exec_status table##_sin(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return table##_run(s,0);} \
 static const agc_routine_entry table##_entries[2]={{FF(spcos_z),table##_cos},{FF((spcos_z)+1u),table##_sin}}; \
 const agc_routine_table table={table##_entries,2};
#if RK_LM || !RK_CM
#include "lm_single_trig.h"
#define LM_TRIG(s,c) (lm_single_trig((s),(c))!=LM_SINGLE_TRIG_OK||lm_single_trig_return(s)!=LM_SINGLE_TRIG_OK)
SP_TABLE(lm_sp_routines,05032u,LM_TRIG)
#endif
#if RK_CM || !RK_LM
#include "cm_single_return.h"
#define CM_TRIG(s,c) (cm_single_trig((s),(c))!=CM_TRIG_OK||cm_single_return(s)!=CM_TRIG_OK)
SP_TABLE(cm_sp_routines,05074u,CM_TRIG)
#endif
