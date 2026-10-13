/* SPDX-License-Identifier: MIT */
#ifndef SERVICE_ROUTINES_H
#define SERVICE_ROUTINES_H
#include <stdbool.h>
#include "p66_bias.h"
#include "agc_word.h"
enum service_routines_mission {SERVICE_ROUTINES_LM,SERVICE_ROUTINES_CM};
enum service_routines_entry {SERVICE_UPENT2,SERVICE_DOWNENT2,SERVICE_UPFLAG,SERVICE_DOWNFLAG,SERVICE_DELAYJOB,SERVICE_TCGETCAD,SERVICE_WAKER,SERVICE_WAKER_FINISH,SERVICE_GENTRAN,SERVICE_GENTRAN_LOOP,SERVICE_B5OFF,SERVICE_TRFAILOF,SERVICE_TRFAILON};
/* Canonical original source body. External scheduler/alarm bodies are explicit
 * compiled continuations. GENTRAN performs one original descending iteration,
 * ending at its genuine +1 back edge or source return/fallthrough. Caller must
 * keep producers quiesced while composing iterations; no extra poll is added.
 * Ordinary RAM15, raw A/L16, coherent banks/ZERO and ordinary compiled caller
 * Q0100..07775 (any Q for TCGETCAD, WAKER, its finish and B5OFF, which never
 * read it). Named flag descriptors map ordinary erasable words074..01777.
 * GENTRAN source/destination effective logical addresses remain valid original
 * CA/LXCH operand forms; ordinary data (physical>=030) and fixed sources.
 * Unsupported input is unchanged. Fixed inline/source words are immutable
 * source-qualified caller/data inputs, not instructions decoded by C. */
/* Three source-emitted passive words: alarm01104, TCSLEEP, WAKECAD. */
bool service_routines_literal(enum service_routines_mission,uint16_t,agc_word *);
bool service_routines_step(apollo_overlay_state *,p66_bias_io *,enum service_routines_mission,enum service_routines_entry,agc_word fixed_inline0,agc_word fixed_inline1,agc_word fixed_source_word);
/* Passive fixed data resolver, never an instruction fetch. The block driver
 * follows only the compiled GENTRAN back edge, retaining exclusion across C
 * returns. LOOP means the budget ended at that exact source back edge; BAD
 * means an unsupported typed continuation, not a fabricated source return. */
typedef bool (*service_routines_fixed_reader)(void *,uint16_t,agc_word *);
enum service_routines_block_status {SERVICE_BLOCK_DONE,SERVICE_BLOCK_LOOP,SERVICE_BLOCK_BAD};
enum service_routines_block_status service_routines_block(apollo_overlay_state *,p66_bias_io *,enum service_routines_mission,agc_word,agc_word,service_routines_fixed_reader,void *,uint16_t,uint16_t *);
#endif
