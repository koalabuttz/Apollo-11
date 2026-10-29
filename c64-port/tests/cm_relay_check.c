/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_relay_check.h"
#include "cm_relay_vectors.h"
#include "dsky_display.h"
uint16_t cm_relay_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t f,patch,end,failures=0,number,old[4],values[4];cm_native_io io;cm_relay_output out;
 if(n>=CM_RELAY_TARGET_COUNT)return 1;
 number=cm_relay_numbers[n];cm_relay_input(s,&io,&out,number);memcpy(before,s,sizeof *s);old[0]=io.superbank;old[1]=io.interrupt_enabled;old[2]=out.wrote_channel10;old[3]=out.channel10;
 if(cm_relay_drain(s,&io,&out)!=cm_relay_results[n])++failures;
 values[0]=io.superbank;values[1]=io.interrupt_enabled;values[2]=out.wrote_channel10;values[3]=out.channel10;
 patch=cm_relay_offsets[n];end=cm_relay_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+4u;++f){uint16_t expected=f<2048u?before->words[f]:old[f-2048u];uint16_t actual=f<2048u?s->words[f]:values[f-2048u];
  if(patch<end&&cm_relay_patch[patch][0]==f)expected=cm_relay_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
uint16_t cm_relay_display_check(apollo_overlay_state *s,volatile uint8_t *screen,volatile uint8_t *color){cm_native_io io;cm_relay_output out;dsky_display_state display;uint16_t failures=0;
 /* Case0's real source fixture drains dirty MODE89. Only the source-emitted
  * channel word reaches the hardware latches, then actual screen/color RAM. */
 cm_relay_input(s,&io,&out,0);if(cm_relay_drain(s,&io,&out)!=EXEC_OK||out.wrote_channel10!=1u)return 1;
 dsky_display_reset(&display);dsky_display_output(&display,010,(uint16_t)out.channel10);dsky_display_render(&display,screen,color,1);
 if(screen[143]!=56u||screen[144]!=57u||(color[143]&15u)!=5u||(color[144]&15u)!=5u)++failures;
 if(s->words[01035]!=(uint16_t)(04000u|(035u<<5)|037u)||s->words[5]!=02071u||s->words[2]!=06710u)++failures;
 return failures;
}
