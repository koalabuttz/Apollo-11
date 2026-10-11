/* SPDX-License-Identifier: MIT */
#include "cm_alarm2_image_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_alarm2_image_fixture *f=CM_ALARM2_IMAGE_FIXTURE;enum exec_status status;
 if(selector==0u){if(s->words[5]==03261u&&cm_alarm2_producer(s,CM_ALARM2_IMAGE_IO)!=EXEC_OK)return 1;f->fixed_word=0;f->needs_data=(uint16_t)cm_alarm2_fixed_address(s,CM_ALARM2_IMAGE_IO,&f->fixed_address);return 0;}
 if(selector!=1u)return 1;status=cm_alarm2_image(s,CM_ALARM2_IMAGE_IO,f->fixed_word);f->status=(uint16_t)status;return status==EXEC_OUT_OF_SCOPE;
}
