/* SPDX-License-Identifier: MIT */
#include "cm_abort_word_binding.h"
#include "cm_alarm_rope_probe.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_abort_word_fixture *f=CM_ABORT_WORD_FIXTURE;enum exec_status status;
 if(selector==0u){f->fixed_word=0;f->needs_data=(uint16_t)cm_abort_word_fixed_address(s,CM_ABORT_WORD_IO,&f->fixed_address);return 0;}
 if(selector!=1u)return 1;status=cm_abort_word(s,CM_ABORT_WORD_IO,f->fixed_word);f->status=(uint16_t)status;return status==EXEC_OUT_OF_SCOPE;
}
