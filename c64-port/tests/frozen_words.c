/* SPDX-License-Identifier: MIT */
#include "frozen_words.h"

void frozen_words_init(frozen_words *r, const uint8_t *bytes, uint16_t length,
                       const uint16_t *dictionary, uint8_t dictionary_count) {
    r->cursor=bytes; r->end=bytes+length; r->dictionary=dictionary;
    r->dictionary_count=dictionary_count; r->repeat=0;
    r->previous=0; r->have_previous=0;
    r->failed=(uint8_t)(dictionary_count>125u);
}

uint8_t frozen_words_next(frozen_words *r, uint16_t *word) {
    uint8_t token;
    if(r->failed) return 0;
    if(r->repeat) {
        --r->repeat; *word=r->previous; return 1;
    }
    if(r->cursor==r->end) return 0;
    token=*r->cursor++;
    if(token<125u) {
        if(token>=r->dictionary_count) goto bad;
        r->previous=r->dictionary[token];
    } else if(token==125u) {
        uint8_t low;
        if(r->cursor==r->end) goto bad;
        low=*r->cursor++;
        if(r->cursor==r->end) goto bad;
        r->previous=(uint16_t)((uint16_t)*r->cursor++ << 8) | low;
    } else if(token==126u) {
        if(!r->have_previous || r->cursor==r->end) goto bad;
        r->repeat=*r->cursor++;
        if(!r->repeat) goto bad;
        --r->repeat; *word=r->previous; return 1;
    } else if(token==127u) goto bad;
    else {
        if(r->cursor==r->end) goto bad;
        r->previous=(uint16_t)((uint16_t)(token & 127u) << 8) | *r->cursor++;
    }
    r->have_previous=1; *word=r->previous; return 1;
bad:
    r->failed=1; return 0;
}

uint8_t frozen_words_finished(const frozen_words *r) {
    return (uint8_t)(!r->failed && !r->repeat && r->cursor==r->end);
}
