/* SPDX-License-Identifier: MIT */
#include "cm_abort_recovery_cases.h"
#include "cm_enema_image_cases.h"
void cm_abort_recovery_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){cm_enema_image_input(s,io,n);s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));s->words[5]=05732;s->words[2]=05731;io->interrupt_enabled=1;s->words[0103]=(agc_word)((s->words[0103]&077737u)|((n&1u)?040u:0));}
