/* SPDX-License-Identifier: MIT */
#ifndef FROZEN_WORDS_H
#define FROZEN_WORDS_H
#include <stdint.h>

/* Lossless, row-addressable diagnostic fixtures. Tokens 0..124 name dictionary
 * words; 125 precedes a little-endian full uint16_t; 126 repeats the previous
 * word 1..255 times; 127 is invalid; 128..255 encode a 15-bit literal's high
 * seven bits, followed by its low byte. There is no implicit row terminator.
 * Caller-supplied arrays and lengths must describe readable storage. */
typedef struct {
    const uint8_t *cursor, *end;
    const uint16_t *dictionary;
    uint16_t previous;
    uint8_t dictionary_count, repeat, have_previous, failed;
} frozen_words;

void frozen_words_init(frozen_words *reader, const uint8_t *bytes,
                       uint16_t length, const uint16_t *dictionary,
                       uint8_t dictionary_count);
/* Returns one for a decoded word, zero at end or on malformed input. */
uint8_t frozen_words_next(frozen_words *reader, uint16_t *word);
/* Check after consuming the exact expected word count: rejects trailing words,
 * unconsumed runs, truncation, and invalid dictionary references. */
uint8_t frozen_words_finished(const frozen_words *reader);
#endif
