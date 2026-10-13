/* SPDX-License-Identifier: MIT */
#include "lm_restart_routines.h"
#define LM_RESTART_INITDSP_ONLY
#include "lm_restart_table_data.inc"
#define B1(z) (uint16_t)(1024u+((z)-02000u))
#define E(a) (uint16_t)(AGC_TRANSFER_ERASABLE_BASE+(a))
#define W s->words

static enum exec_status selected(enum lm_restart_select_exit e){return e==LM_RS_SELECT_INVALID?EXEC_OUT_OF_SCOPE:EXEC_OK;}
static enum exec_status restarts(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return selected(lm_restart_select(s,LM_RS_SELECT_GROUP));}
static enum exec_status getpart2(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return selected(lm_restart_select(s,LM_RS_SELECT_TYPEB_TABLE));}
static enum exec_status phspart2(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return selected(lm_restart_select(s,LM_RS_SELECT_PART2));}
/* 01,3550 TC RTRNCADR; 01,3565 TC SWRETURN. */
static enum exec_status display_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 W[2]=(agc_word)(LM_RS_RTRNCADR+1u);W[5]=LM_RS_SWRETURN;return EXEC_OK;
}
/* TC GOLOC-1: the service address RESTARTS stored. */
static enum exec_status goloc_call(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_word w=W[0704];(void)p;(void)io;
 if(w!=LM_RS_FINDVAC&&w!=LM_RS_NOVAC&&w!=LM_RS_WAITLIST&&w!=LM_RS_LONGCALL)return EXEC_OUT_OF_SCOPE;
 W[2]=0705;W[5]=w;return EXEC_OK;
}
/* The service returns to GOLOC+2: TC SWRETURN, PHSPART2 or GETPART2. */
static enum exec_status goloc_return(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_word w=W[0707];(void)p;(void)io;
 if(w!=LM_RS_SWRETURN&&((w!=LM_RS_PHSPART2&&w!=LM_RS_GETPART2)||W[4]!=02000u))return EXEC_OUT_OF_SCOPE;
 W[2]=0710;W[5]=w;return EXEC_OK;
}
static const agc_routine_entry entries[6]={
 {B1(LM_RS_RESTARTS),restarts},{B1(LM_RS_GETPART2),getpart2},{B1(LM_RS_DISPLAY_RETURN),display_return},
 {B1(LM_RS_PHSPART2),phspart2},{E(0704u),goloc_call},{E(0707u),goloc_return}};
const agc_routine_table lm_restart_routines={entries,6};
const agc_site_operand lm_restart_operands[2]={{LM_RESTART_INITDSP_SITE,LM_RESTART_INITDSP_0},{LM_RESTART_INITDSP_SITE+1u,LM_RESTART_INITDSP_1}};
