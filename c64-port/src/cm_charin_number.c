/* SPDX-License-Identifier: MIT */
#include "cm_charin_number.h"
#include "dsky_charin_number.h"
enum exec_status cm_charin_number(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote){
 dsky_charin_io primitive;enum p66_bias_status status;uint16_t count=s->words[0777],key=s->words[0154],address=s->words[0116];
 if(wrote==0 || s->words[5]!=02000)return EXEC_OUT_OF_SCOPE;
 primitive.control.superbank=io->superbank;primitive.control.interrupt_enabled=io->interrupt_enabled;primitive.channel11=io->channel11;primitive.wrote_channel11=0;
 status=dsky_charin_number(s,&primitive);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;
 if((count&040000u)==0){s->words[0114]=02117;s->words[2]=02216;
  /* DECON is an actual CM address, but unrelated preexisting ADDRWD data
   * must remain unchanged when the source fraction call did not execute. */
  if(s->words[0116]==02322 && (count<15u) && (s->words[01000]&3u)!=0 && (s->words[0777]&040000u)!=0)s->words[0116]=02223;else s->words[0116]=address;
 }else s->words[2]=(key==8 || key==9) ? (count==077777 ? 02067u : 02066u) : 02103u;
 s->words[5]=05217;io->superbank=primitive.control.superbank;io->interrupt_enabled=primitive.control.interrupt_enabled;io->channel11=primitive.channel11;*wrote=primitive.wrote_channel11;return EXEC_OK;
}
