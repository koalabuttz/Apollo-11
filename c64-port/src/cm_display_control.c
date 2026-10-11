/* SPDX-License-Identifier: MIT */
#include "cm_display_control.h"
enum cm_display_control_status cm_display_control_enter(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words;agc_word flags,cadr;
 if(w[5]!=02707u&&w[5]!=02715u&&w[5]!=02720u)return CM_DISPLAY_CONTROL_UNSUPPORTED;
 if(w[4]!=020000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(020000u|(w[3]>>8))||w[7]!=0||w[0]!=agc_extend(agc_correct(w[0]))||w[0133]>07777u||w[0134]>076000u||(w[0134]&01777u)||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return CM_DISPLAY_CONTROL_UNSUPPORTED;
 flags=(agc_word)(w[5]==02707u?020000u:w[5]==02715u?024030u:010u);w[0155]=agc_correct(w[0]);w[0160]=(agc_word)((flags&074377u)+w[3]);cadr=agc_add_word((agc_word)(w[0133]&01777u),w[0134]);w[0157]=cadr;w[0]=agc_add_register(agc_extend(cadr),agc_extend(w[072]));w[1]=02724;w[2]=2;w[5]=02770;io->interrupt_enabled=1;return CM_DISPLAY_CONTROL_OK;}
