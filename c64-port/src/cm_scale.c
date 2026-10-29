/* SPDX-License-Identifier: MIT */
#include "cm_scale.h"
enum exec_status cm_scale_output(apollo_overlay_state *s,cm_native_io *io,enum dsky_scale_entry entry){
 static const uint16_t location[3]={02614,02621,02623},original_return[3]={02711,02716,02720},cm_return[3]={02615,02622,02624};
 p66_bias_io primitive;enum p66_bias_status status;
 if((unsigned)entry>DSKY_DP3OUT||s->words[5]!=location[entry])return EXEC_OUT_OF_SCOPE;
 primitive.superbank=io->superbank;primitive.interrupt_enabled=io->interrupt_enabled;
 status=dsky_scale_output(s,&primitive,entry);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;
 /* Literal DPOUT caller in MPAC+6; TPLEFTN may replace it on overflow.
  * READLO caller is2632; DP3's TPLEFTN caller2626 occupies SFTEMP2. */
 if(s->words[0162]==original_return[entry])s->words[0162]=cm_return[entry];
 s->words[0144]=02632;if(entry==DSKY_DP3OUT)s->words[0124]=02626;
 io->superbank=primitive.superbank;io->interrupt_enabled=primitive.interrupt_enabled;return EXEC_OK;
}
