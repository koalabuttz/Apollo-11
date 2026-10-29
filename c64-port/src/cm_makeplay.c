/* SPDX-License-Identifier: MIT */
#include "cm_makeplay.h"
static uint8_t ready(const uint16_t *w,const cm_native_io *io,agc_word pc){return(uint8_t)(w[5]==pc&&w[3]<=03400u&&(w[3]&0377u)==0&&w[4]==020000u&&w[6]==(agc_word)(020000u|(w[3]>>8))&&w[7]==0&&io->superbank<=0160u&&(io->superbank&017u)==0&&io->interrupt_enabled<=1u);}
enum cm_makeplay_boundary cm_makeplay_priority(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words;if(!ready(w,io,02770)||w[0167]>AGC_MASK)return CM_MAKEPLAY_UNSUPPORTED;w[0163]=(agc_word)(w[0167]&037000u);w[0]=033000;w[1]=agc_extend(agc_correct(w[1]));w[2]=02775;w[5]=05210;return CM_MAKEPLAY_PRIORITY;}
enum cm_makeplay_boundary cm_makeplay_select(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words;agc_word flags=w[0160],pm,address;uint8_t which;
 if(!ready(w,io,02775)||(flags&040100u)||flags>AGC_MASK||w[0163]>037000u||(w[0163]&0777u)||w[01012]>7u||io->channel11>AGC_MASK)return CM_MAKEPLAY_UNSUPPORTED;
 w[0164]=2;w[1]=agc_extend(agc_correct(w[1]));
 if((flags&020000u)&&agc_compare(w[0157],w[0374])==0&&w[01012]!=0){w[0]=(agc_word)(w[01012]-1u);w[2]=03632;w[5]=05217;return CM_MAKEPLAY_END;}
 if((flags&020000u)==0&&(w[01071]&010u)&&(w[0100]&011210u)){w[0]=(agc_word)(w[0100]&011210u);w[2]=03020;w[5]=05721;return CM_MAKEPLAY_ALARM;}
 w[01071]=flags;if(flags&050u)w[0374]=w[0157];w[0371]=w[0155];w[0366]=(agc_word)(w[0163]|io->superbank);io->interrupt_enabled=1;pm=(agc_word)(w[0100]&066521u);
 if(pm!=0){w[0]=agc_diminished_abs(pm);w[2]=3;w[5]=03176;return CM_MAKEPLAY_SLEEP;}
 io->channel11=(agc_word)(io->channel11&077757u);which=(uint8_t)((w[0100]&0700u)!=0);address=(agc_word)(01042u+which);w[0]=agc_extend(w[address]);w[address]=0;w[1]=which;w[2]=03036;w[5]=05201;io->interrupt_enabled=0;return CM_MAKEPLAY_WAKE;
}
enum cm_makeplay_boundary cm_makeplay_wake_return(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words,offset=w[064];if(!ready(w,io,03036)||w[0164]!=2u||(offset!=077776u&&(offset>72u||offset%12u)))return CM_MAKEPLAY_UNSUPPORTED;if(offset==0||offset==077776u)w[064]=05217;else w[0164u+offset]=05217;w[0]=2;w[1]=agc_extend(agc_correct(w[1]));w[2]=3;w[0164]=2;w[5]=03312;io->interrupt_enabled=1;return CM_MAKEPLAY_PLAY;}
