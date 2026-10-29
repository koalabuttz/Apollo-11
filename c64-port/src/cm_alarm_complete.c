/* SPDX-License-Identifier: MIT */
#include "cm_alarm_complete.h"
enum exec_status cm_alarm_complete(apollo_overlay_state *s,cm_native_io *io,enum cm_alarm_record_entry entry,agc_word word){enum exec_status status=cm_alarm_image(s,io,entry,word);if(status==EXEC_OK&&entry==CM_RECORD_BORTENT&&s->words[5]==05721u&&s->words[0]!=1u&&s->words[0]!=0177776u)io->interrupt_enabled=1;return status;}
