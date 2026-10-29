/* SPDX-License-Identifier: MIT */
#include "cm_entry_tasks.h"
enum cm_wl_status cm_setjtag_image(apollo_overlay_state *s,cm_native_io *io){
 cm_phase_state phase;static const agc_word word=1;
 if(s==0||io==0||s->words[6]!=032006u||s->words[3]!=03000u||s->words[4]!=032000u||io->interrupt_enabled>1u||(io->superbank&077617u)!=0)return CM_WL_OUT_OF_SCOPE;
 s->words[03312]=agc_negate(s->words[025]);s->words[03311]=1;
 cm_native_gather_phase(s,io,&phase);phase.q=03214;
 if(cm_phase_change(&phase,&word,1,0)!=CM_PHASE_OK)return CM_WL_CALLBACK_FAULT;
 cm_native_commit_phase(s,io,&phase);s->words[2]=03216;s->words[5]=05324;return CM_WL_OK;
}
enum cm_wl_status cm_setjtag_callback(cm_wl_native *r,void *user){
 cm_entry_task_binding *b=(cm_entry_task_binding *)user;
 if(r==0||b==0||r->current.location!=03207u||r->current.bank!=032066u)return CM_WL_CALLBACK_FAULT;
 return cm_setjtag_image(b->image,b->io);
}
