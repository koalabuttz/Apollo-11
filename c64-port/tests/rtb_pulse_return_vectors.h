/* SPDX-License-Identifier: MIT
 * Frozen solely from original assembled software. */
#ifndef RTB_PULSE_RETURN_VECTORS_H
#define RTB_PULSE_RETURN_VECTORS_H
#if RTBP_MISSION == 0
#include "frozen_words.h"
#define RTB_PULSE_RETURN_FROZEN_ROWS 32u
#define RTB_PULSE_RETURN_FROZEN_WORDS 10u
#define RTB_PULSE_RETURN_FROZEN_STORAGE_BYTES 436u
#ifdef RTB_PULSE_RETURN_FROZEN_IMPLEMENTATION
const uint16_t rtb_pulse_return_reference_dictionary[7]={
    0,1,5,3121,2,112,16,
};
const uint16_t rtb_pulse_return_reference_offsets[33]={
    0,8,18,27,38,49,58,69,79,90,100,111,121,132,142,153,
    164,175,187,199,211,223,234,247,259,270,282,295,307,320,332,344,
    356,
};
const uint8_t rtb_pulse_return_reference_stream[356]={
    0,126,4,1,2,3,0,0,1,0,1,0,0,1,2,3,
    0,0,4,0,126,3,1,2,3,0,0,128,3,0,1,0,
    0,1,2,3,0,0,128,15,0,1,0,0,1,2,3,0,
    0,6,0,126,3,1,2,3,0,0,128,31,0,1,0,0,
    1,2,3,0,0,128,32,0,126,3,1,2,3,0,0,128,
    47,0,1,0,0,1,2,3,0,0,128,48,0,126,3,1,
    2,3,0,0,128,63,0,1,0,0,1,2,3,0,0,128,
    64,0,126,3,1,2,3,0,0,128,95,0,1,0,0,1,
    2,3,0,0,128,96,0,126,3,1,2,3,0,0,128,127,
    0,1,0,0,1,2,3,0,0,128,128,0,0,6,0,1,
    2,3,0,0,128,255,0,1,6,0,1,2,3,0,0,129,
    0,0,0,128,32,0,1,2,3,0,0,129,255,0,1,128,
    48,0,1,2,3,0,0,130,0,0,0,128,64,0,1,2,
    3,0,0,131,255,0,1,5,0,4,1,139,237,2,3,132,
    0,0,126,3,4,1,140,0,2,3,135,255,0,1,5,0,
    4,1,125,237,215,2,3,136,0,0,126,3,4,1,125,0,
    216,2,3,143,255,0,1,5,0,1,2,3,0,0,159,255,
    0,1,5,0,4,1,159,237,2,3,191,255,0,1,5,0,
    4,1,125,237,255,2,3,255,255,0,1,5,0,4,1,191,
    237,2,3,125,0,128,0,126,3,4,1,125,0,192,2,3,
    125,255,191,0,1,5,0,1,2,3,0,0,125,254,255,0,
    0,5,0,1,2,3,0,0,125,255,255,0,1,5,0,1,
    2,3,0,0,
};
#else
extern const uint16_t rtb_pulse_return_reference_dictionary[7];
extern const uint16_t rtb_pulse_return_reference_offsets[33];
extern const uint8_t rtb_pulse_return_reference_stream[356];
#endif
static inline uint8_t rtb_pulse_return_reference_begin(frozen_words *r, unsigned row) {
    if(row>=RTB_PULSE_RETURN_FROZEN_ROWS) return 0;
    frozen_words_init(r,rtb_pulse_return_reference_stream+rtb_pulse_return_reference_offsets[row],
        (uint16_t)(rtb_pulse_return_reference_offsets[row+1]-rtb_pulse_return_reference_offsets[row]),
        rtb_pulse_return_reference_dictionary,7);
    return 1;
}
#endif
#if RTBP_MISSION == 1
#include "frozen_words.h"
#define RTB_PULSE_RETURN_FROZEN_ROWS 32u
#define RTB_PULSE_RETURN_FROZEN_WORDS 10u
#define RTB_PULSE_RETURN_FROZEN_STORAGE_BYTES 436u
#ifdef RTB_PULSE_RETURN_FROZEN_IMPLEMENTATION
const uint16_t rtb_pulse_return_reference_dictionary[7]={
    0,1,5,3096,2,112,16,
};
const uint16_t rtb_pulse_return_reference_offsets[33]={
    0,8,18,27,38,49,58,69,79,90,100,111,121,132,142,153,
    164,175,187,199,211,223,234,247,259,270,282,295,307,320,332,344,
    356,
};
const uint8_t rtb_pulse_return_reference_stream[356]={
    0,126,4,1,2,3,0,0,1,0,1,0,0,1,2,3,
    0,0,4,0,126,3,1,2,3,0,0,128,3,0,1,0,
    0,1,2,3,0,0,128,15,0,1,0,0,1,2,3,0,
    0,6,0,126,3,1,2,3,0,0,128,31,0,1,0,0,
    1,2,3,0,0,128,32,0,126,3,1,2,3,0,0,128,
    47,0,1,0,0,1,2,3,0,0,128,48,0,126,3,1,
    2,3,0,0,128,63,0,1,0,0,1,2,3,0,0,128,
    64,0,126,3,1,2,3,0,0,128,95,0,1,0,0,1,
    2,3,0,0,128,96,0,126,3,1,2,3,0,0,128,127,
    0,1,0,0,1,2,3,0,0,128,128,0,0,6,0,1,
    2,3,0,0,128,255,0,1,6,0,1,2,3,0,0,129,
    0,0,0,128,32,0,1,2,3,0,0,129,255,0,1,128,
    48,0,1,2,3,0,0,130,0,0,0,128,64,0,1,2,
    3,0,0,131,255,0,1,5,0,4,1,139,237,2,3,132,
    0,0,126,3,4,1,140,0,2,3,135,255,0,1,5,0,
    4,1,125,237,215,2,3,136,0,0,126,3,4,1,125,0,
    216,2,3,143,255,0,1,5,0,1,2,3,0,0,159,255,
    0,1,5,0,4,1,159,237,2,3,191,255,0,1,5,0,
    4,1,125,237,255,2,3,255,255,0,1,5,0,4,1,191,
    237,2,3,125,0,128,0,126,3,4,1,125,0,192,2,3,
    125,255,191,0,1,5,0,1,2,3,0,0,125,254,255,0,
    0,5,0,1,2,3,0,0,125,255,255,0,1,5,0,1,
    2,3,0,0,
};
#else
extern const uint16_t rtb_pulse_return_reference_dictionary[7];
extern const uint16_t rtb_pulse_return_reference_offsets[33];
extern const uint8_t rtb_pulse_return_reference_stream[356];
#endif
static inline uint8_t rtb_pulse_return_reference_begin(frozen_words *r, unsigned row) {
    if(row>=RTB_PULSE_RETURN_FROZEN_ROWS) return 0;
    frozen_words_init(r,rtb_pulse_return_reference_stream+rtb_pulse_return_reference_offsets[row],
        (uint16_t)(rtb_pulse_return_reference_offsets[row+1]-rtb_pulse_return_reference_offsets[row]),
        rtb_pulse_return_reference_dictionary,7);
    return 1;
}
#endif
#endif
