/* SPDX-License-Identifier: MIT */
#include "cm_abort_whimper.h"
enum exec_status cm_abort_whimper(apollo_overlay_state *s,cm_native_io *io,uint8_t *in_isr){agc_word *w;
 if(!s||!io||!in_isr||*in_isr>1u)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if((w[5]!=07766u&&w[5]!=07767u)||w[7]||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u||w[015]>07777u)return EXEC_OUT_OF_SCOPE;
 if(w[5]==07766u)io->interrupt_enabled=0;
 w[0]=07773;w[017]=07773;*in_isr=0;w[2]=07774;w[5]=04701;return cm_interbank_extended(s,io,CM_IB_POSTJUMP,012651,0);
}
