/* SPDX-License-Identifier: MIT */
#include "cm_abort_snapshot_cases.h"
#include "cm_vac5_store_cases.h"
agc_word cm_abort_snapshot_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){static const agc_word pc[3]={07755,05721,05747};unsigned c=n/3u;cm_vac5_store_input(s,io,c);s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));s->words[5]=pc[n%3u];s->words[2]=c&1u?02724:02642;return c&1u?01202:01201;}
