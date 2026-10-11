/* SPDX-License-Identifier: MIT */
#include "cm_delay_image.h"
enum cm_wl_status cm_delay_image(apollo_overlay_state *s,cm_native_io *io,agc_word literal,uint8_t fixed){agc_register olda;agc_word delay;exec_address continuation;enum cm_wl_status status;
 if(s==0||io==0||fixed>1u||io->interrupt_enabled!=0||s->words[2]>07775u)return CM_WL_OUT_OF_SCOPE;
 delay=fixed?literal:s->words[0];if(delay==0||delay>CM_WL_MAX_DELAY)return CM_WL_OUT_OF_SCOPE;
 continuation.location=(agc_word)(s->words[2]+fixed);continuation.bank=(agc_word)(s->words[6]|io->superbank);
 olda=s->words[0];s->words[0]=delay;status=cm_waitlist_image(s,io,continuation);
 if(status!=CM_WL_OK){s->words[0]=olda;return status;}
 s->words[5]=05324;return CM_WL_OK;
}
