/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_ROPE_PROBE_H
#define CM_ALARM_ROPE_PROBE_H
/* Test-only. The legacy ALARM/ALARM2/abort-word harnesses sweep every caller Q
 * in 0100..7775, including fixed addresses that are no genuine call site, and
 * fetch the word at Q from the original rope to compare against the oracle.
 * The flight code takes inline operands from cm_alarm_sites instead
 * (docs/NATIVE_TRANSFER_ABI.md); these resolvers live with the harnesses. */
#include <stdbool.h>
#include "cm_native_image.h"
static inline bool cm_alarm_probe_base(const apollo_overlay_state *s,const cm_native_io *io){return s&&io&&s->words[7]==0&&s->words[3]<=03400u&&!(s->words[3]&0377u)&&!(s->words[4]&~076000u)&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&!(io->superbank&~0160u)&&io->interrupt_enabled<=1u;}
static inline bool cm_alarm_probe_physical(const apollo_overlay_state *s,const cm_native_io *io,agc_word q,uint16_t *out){agc_word bank;if(q<02000u)return false;if(q>=06000u)bank=3;else if(q>=04000u)bank=2;else{bank=(agc_word)(s->words[4]>>10);if(bank>=030u&&(io->superbank&0100u))bank=(agc_word)(bank+8u);}*out=(uint16_t)(bank*1024u+(q&01777u));return true;}
static inline bool cm_alarm_fixed_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){if(!cm_alarm_probe_base(s,io)||s->words[2]<0100u||s->words[2]>07775u||!out||s->words[5]!=05650u)return false;return cm_alarm_probe_physical(s,io,s->words[2],out);}
static inline bool cm_alarm2_fixed_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){if(!cm_alarm_probe_base(s,io)||s->words[2]<0100u||s->words[2]>07775u||!out||s->words[5]!=05652u)return false;return cm_alarm_probe_physical(s,io,s->words[2],out);}
static inline bool cm_abort_word_fixed_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){if(!cm_alarm_probe_base(s,io)||(s->words[5]!=07762u&&s->words[5]!=05726u)||s->words[01363]<0100u||s->words[01363]>07775u||!out)return false;return cm_alarm_probe_physical(s,io,s->words[01363],out);}
#endif
