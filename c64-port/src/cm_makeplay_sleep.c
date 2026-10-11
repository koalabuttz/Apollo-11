/* SPDX-License-Identifier: MIT */
#include "cm_makeplay_sleep.h"
static uint8_t ready(const uint16_t *w,const cm_native_io *io,uint16_t pc){return(uint8_t)(w[5]==pc&&w[3]<=03400u&&(w[3]&0377u)==0&&w[4]==020000u&&w[6]==(uint16_t)(020000u|(w[3]>>8))&&w[7]==0&&io->superbank<=0160u&&(io->superbank&017u)==0&&io->interrupt_enabled<=1u);}
enum cm_display_sleep_boundary cm_makeplay_sleep_enter(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words;if(!ready(w,io,03176)||w[0164]!=2u)return CM_DISPLAY_SLEEP_UNSUPPORTED;
 /* Indexed normal PRIOOCT11254 & WAITMASK3004 passes1004 to UPENT2.
  * Its low3bits choose FLAGWRD4; it retains the1000mask in ITEMP1. */
 w[0100]|=01000;w[061]=01000;w[0154]=1;w[0]=021037;w[1]=4;w[2]=03212;w[5]=05201;io->interrupt_enabled=0;return CM_DISPLAY_SLEEP_WAKE;}
enum cm_display_sleep_boundary cm_makeplay_sleep_return(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words,offset=w[064];if(!ready(w,io,03212)||w[0154]!=1u||w[0164]!=2u||(offset!=077776u&&(offset>72u||offset%12u)))return CM_DISPLAY_SLEEP_UNSUPPORTED;
 if(offset==0||offset==077776u)w[064]=05217;else w[0164u+offset]=05217;
 w[0]=021037;w[1]=agc_extend(agc_correct(w[1]));w[2]=3;w[5]=05175;io->interrupt_enabled=1;return CM_DISPLAY_SLEEP_EXECUTIVE;}
enum cm_display_sleep_boundary cm_makeplay_sleep_resume(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words;if(!ready(w,io,03037))return CM_DISPLAY_SLEEP_UNSUPPORTED;w[0]=2;w[1]=agc_extend(agc_correct(w[1]));w[0164]=2;w[5]=03312;return CM_DISPLAY_SLEEP_PLAY;}
