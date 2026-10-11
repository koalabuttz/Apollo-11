/* SPDX-License-Identifier: MIT */
#include "cm_pinball.h"
enum cm_pinball_status cm_pinball_write(apollo_overlay_state *s,cm_native_io *io,enum dsky_pinball_entry entry){
 p66_bias_io primitive;enum p66_bias_status status;
 if((unsigned)entry>(unsigned)DSKY_2BLANK||s->words[5]!=(entry==DSKY_DSPIN?03227u:entry==DSKY_11DSPIN?03311u:02502u))return CM_PINBALL_OUT_OF_SCOPE;
 /* This is a transient external-I/O view, not another flight image. */
 primitive.superbank=io->superbank;primitive.interrupt_enabled=io->interrupt_enabled;
 status=dsky_pinball_write(s,&primitive,entry);if(status!=P66_BIAS_OK)return CM_PINBALL_OUT_OF_SCOPE;
 s->words[1]=agc_extend(agc_correct(s->words[1]));
 io->superbank=primitive.superbank;io->interrupt_enabled=primitive.interrupt_enabled;return CM_PINBALL_OK;
}
