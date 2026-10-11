/* SPDX-License-Identifier: MIT */
#include "cm_keyboard_runtime.h"
enum exec_status cm_keyboard_runtime_entry(apollo_overlay_state *s,cm_native_io *io,uint16_t *isr,uint16_t key,uint16_t selector,exec_transfer *next,uint16_t *wrote){enum exec_status status;
 if(next==0 || wrote==0)return EXEC_OUT_OF_SCOPE;
 switch(selector){
 case CM_KEYBOARD_IRQ:status=cm_keyboard_interrupt(s,io,isr,key);if(status==EXEC_OK){next->address=(exec_address){s->words[5],(agc_word)(s->words[6]|io->superbank)};next->interpretive=0;*wrote=0;}return status;
 case CM_KEYBOARD_SELECT:status=cm_idle_dispatch(s,io,next);if(status==EXEC_OK)*wrote=1;else if(status==EXEC_IDLE)*wrote=0;return status;
 case CM_KEYBOARD_NV:status=cm_charin_nv(s,io,wrote);break;
 case CM_KEYBOARD_NUMBER:status=cm_charin_number(s,io,wrote);break;
 case CM_KEYBOARD_FINISH:status=cm_basic_finish(s,io,next);if(status==EXEC_IDLE)*wrote=1;else if(status==EXEC_OK)*wrote=0;return status;
 default:return EXEC_OUT_OF_SCOPE;
 }
 if(status==EXEC_OK){next->address=(exec_address){s->words[5],(agc_word)(s->words[6]|io->superbank)};next->interpretive=0;}return status;
}
