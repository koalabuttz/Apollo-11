/* SPDX-License-Identifier: MIT */
/* The work clock's charge for one interpretive word, the same on the C64
 * (the compact interpreter at NEWOPS, src/lm_interp_core.c) and the host
 * (tools/flight_host_run.c, the tables left to the interpreter): yaAGC's
 * MCT for the word, fitted per order and store kind
 * (tools/interp_word_mct.py, include/interp_word_mct_table.h), in units of
 * 4 MCT. A word is charged once, by the code word NEWOPS fetches; resuming
 * a word (DANZIG after an RTB or a job switch) charges nothing. */
#ifndef INTERP_WORD_MCT_H
#define INTERP_WORD_MCT_H
#include <stdint.h>
#include "agc_word.h"
#include "interp_word_mct_table.h"

/* 0: no order; 1 + c: order code c; 129 + k: store kind k; 137: an order
 * pair's base. */
#define INTERP_WORD_STORE_AT 129u
#define INTERP_WORD_BASE_AT  137u
static const uint8_t interp_word_units[138] = INTERP_WORD_CODES;

/* An order pair's units: the base, the left order's (code left) and the
 * right order's (edop: EDOP as NEWOPS's TS EDOP leaves it, the right code
 * plus one, or 0 for none). */
#define INTERP_WORD_PAIR(left, edop)                                                               \
    (uint16_t)(interp_word_units[INTERP_WORD_BASE_AT] + interp_word_units[(left) + 1u] +           \
               interp_word_units[edop])

/* A store code word's units (at least 1): its kind's, DOSTORE's bits
 * 12-14. */
#define INTERP_WORD_STORE_KIND(code) (uint8_t)(INTERP_WORD_STORE_AT + (((code) >> 11) & 7u))
#define INTERP_WORD_STORE(code)      interp_word_units[INTERP_WORD_STORE_KIND(code)]

/* The word's units. NEWOPS's CCS leaves the magnitude less one: its low
 * seven bits are the left code, and TS EDOP keeps bits 8-14. */
static inline uint16_t interp_word_units_of(agc_word code)
{
    agc_word u = (agc_word)(~code - 1u);
    return code & 040000u ? INTERP_WORD_PAIR(u & 0177u, (u >> 7) & 0177u) : INTERP_WORD_STORE(code);
}
#endif
