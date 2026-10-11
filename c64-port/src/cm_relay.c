/* SPDX-License-Identifier: MIT */
#include "cm_relay.h"
#include "dsky_relay.h"
enum exec_status cm_relay_drain(apollo_overlay_state *s,cm_native_io *io,cm_relay_output *output){dsky_relay_io primitive;enum p66_bias_status status;uint16_t caller=s->words[2];
 if(output==0||s->words[5]!=02024u||caller<0100u||caller>07776u)return EXEC_OUT_OF_SCOPE;
 primitive.control.superbank=io->superbank;primitive.control.interrupt_enabled=io->interrupt_enabled;primitive.channel10=output->channel10;primitive.wrote_channel10=0;
 /* The qualified fixed relay scan uses no caller data until return. Its
  * known source caller is adapted to the actual CM caller after the scan. */
 s->words[2]=02070;status=dsky_relay_drain(s,&primitive);
 if(status!=P66_BIAS_OK){s->words[2]=caller;return EXEC_OUT_OF_SCOPE;}
 s->words[1]=agc_extend(agc_correct(s->words[1]));
 s->words[5]=(uint16_t)(caller+primitive.wrote_channel10);
 if(primitive.wrote_channel10!=0u)s->words[2]=06710; /* CM Q+1 INDEX Q6706 / TC+1. */
 io->superbank=primitive.control.superbank;io->interrupt_enabled=primitive.control.interrupt_enabled;
 output->wrote_channel10=primitive.wrote_channel10;output->channel10=primitive.channel10;return EXEC_OK;
}
