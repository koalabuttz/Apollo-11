/* SPDX-License-Identifier: MIT */
#ifndef LANEC_CASE_H
#define LANEC_CASE_H
/* Test-only case description shared by the native runner, the Virtual AGC
 * oracle and the C64 diagnostic. Never linked into flight code. */
#include "lanec.h"
#define LANEC_MAX_RANGES 8u
#define LANEC_MAX_STOPS 4u
typedef struct {
    enum lanec_mission mission;
    uint8_t ranges, stops;
    lanec_range range[LANEC_MAX_RANGES]; /* code the native routine translates */
    lanec_point stop[LANEC_MAX_STOPS];   /* labels inside those ranges owned elsewhere */
    const lanec_point *stop_list;        /* more such labels (NULL: none) */
    uint16_t stop_count;
    const lanec_range *range_list;       /* more translated code (NULL: none) */
    uint16_t range_count;
    /* Interpretive code: when newops is set, the oracle also stops at that
     * address (the interpreter's NEWOPS) once LOC, in the bank FB selects,
     * leaves the interpretive words listed in loc[]. */
    uint16_t newops;
    uint8_t locs;
    lanec_range loc[LANEC_MAX_RANGES];
} lanec_meta;
typedef void (*lanec_make)(uint32_t n, apollo_overlay_state *, lanec_io *, lanec_meta *);
typedef enum lanec_status (*lanec_run)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
typedef struct { const char *name; uint32_t count; lanec_make make; lanec_run run; } lanec_suite;

uint32_t lanec_rand(uint32_t *state);
/* Random but well-formed image and I/O: every erasable word, A/L/Q in 16-bit
 * form, consistent EB/FB/BB, ZERO = 0, random channels, IRQ and superbank
 * bits. The caller then sets the entry bank, Z and the inputs it varies. */
void lanec_fill(apollo_overlay_state *, lanec_io *, uint32_t seed);
void lanec_enter(apollo_overlay_state *, lanec_io *, uint16_t bank, uint16_t z);
/* CRC-32 (zlib) over a result, little-endian 16-bit fields: every word, channels, superbank, IRQ, status. */
uint32_t lanec_hash(const apollo_overlay_state *, const lanec_io *, enum lanec_status);
#endif
