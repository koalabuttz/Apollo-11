/* SPDX-License-Identifier: MIT */
#include "cm_charin_nv.h"
#include "dsky_charin_nv.h"
enum exec_status cm_charin_nv(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote){dsky_charin_io primitive;enum p66_bias_status status;
 if(wrote==0 || s->words[5]!=02000)return EXEC_OUT_OF_SCOPE;
 primitive.control.superbank=io->superbank;primitive.control.interrupt_enabled=io->interrupt_enabled;primitive.channel11=io->channel11;primitive.wrote_channel11=0;
 status=dsky_charin_nv(s,&primitive);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;
 s->words[1]=agc_extend(agc_correct(s->words[1]));s->words[0]=05217;s->words[2]=02271;s->words[5]=05217;s->words[0136]=05217;
 io->superbank=primitive.control.superbank;io->interrupt_enabled=primitive.control.interrupt_enabled;io->channel11=primitive.channel11;*wrote=primitive.wrote_channel11;return EXEC_OK;
}
