/* SPDX-License-Identifier: MIT */
#include "cm_abort_recovery.h"
#include "cm_interbank_extended.h"
#include "cm_phase_cleanup.h"
#include "service_routines.h"
static enum exec_status down(apollo_overlay_state *s,cm_native_io *io,agc_word descriptor){p66_bias_io hw;bool ok;hw.superbank=io->superbank;hw.interrupt_enabled=io->interrupt_enabled;ok=service_routines_step(s,&hw,SERVICE_ROUTINES_CM,SERVICE_DOWNFLAG,descriptor,0,0);if(!ok)return EXEC_OUT_OF_SCOPE;io->superbank=hw.superbank;io->interrupt_enabled=hw.interrupt_enabled;return EXEC_OK;}
enum exec_status cm_abort_recovery_stage(apollo_overlay_state *s,cm_native_io *io,uint8_t phase){agc_word *w;
 if(!s||!io||phase>6u)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if(w[7]||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(phase==0u){if(w[5]!=05732u)return EXEC_OUT_OF_SCOPE;w[0]=agc_extend((agc_word)(w[0103]&040u));w[1]=agc_extend(agc_correct(w[1]));if(w[0]){w[0]=037;w[2]=05736;w[5]=07766;}else{w[2]=05737;w[5]=05560;}return EXEC_OK;}
 if(phase==1u){if(w[5]!=05560u||w[2]!=05737u)return EXEC_OUT_OF_SCOPE;return down(s,io,067);}
 if(phase==2u||phase==3u){if(w[5]!=(phase==2u?05740u:05742u))return EXEC_OUT_OF_SCOPE;w[2]=(agc_word)(w[5]+1u);w[5]=05560;return down(s,io,phase==2u?0236:054);}
 if(phase==4u){if(w[5]!=05744u)return EXEC_OUT_OF_SCOPE;w[2]=05745;w[5]=04662;return cm_interbank_extended(s,io,CM_IB_BANKCALL,012501,0);}
 if(phase==5u){if(w[5]!=02501u||w[2]!=04675u)return EXEC_OUT_OF_SCOPE;return cm_phase_cleanup(s,io);}
 if(w[5]!=04675u||w[0133]!=05746u)return EXEC_OUT_OF_SCOPE;
 if(cm_interbank_extended(s,io,CM_IB_SWRETURN,0,0)!=EXEC_OK)return EXEC_OUT_OF_SCOPE;
 w[2]=05747;w[5]=07767;return EXEC_OK;
}
