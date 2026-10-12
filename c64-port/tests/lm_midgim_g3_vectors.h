/* SPDX-License-Identifier: MIT; native phase steps whose frontiers equal original NEWOPS images (tools/freeze_lm_midgim_phases.py). */
#ifndef LM_MIDGIM_G3_VECTORS_H
#define LM_MIDGIM_G3_VECTORS_H
#define LM_MIDGIM_G3_ENTRY_CHANGES 0u
#define LM_MIDGIM_G3_EXIT_CHANGES 27u
#include "frozen_words.h"
#define LM_MIDGIM_G3_FROZEN_ROWS 8u
#define LM_MIDGIM_G3_FROZEN_WORDS 63u
#define LM_MIDGIM_G3_FROZEN_STORAGE_BYTES 645u
#ifdef LM_MIDGIM_G3_FROZEN_IMPLEMENTATION
const uint16_t lm_midgim_g3_reference_dictionary[50]={
    0,2,1,29696,4,116,16,6,3,32,77,78,79,88,89,108,
    109,110,117,976,1230,1231,1232,1233,1534,1535,1926,4096,7223,9503,12293,14603,
    16384,30598,32757,32765,49152,295,296,62,25,93,112,256,3522,3961,10752,10756,
    12123,29703,
};
const uint16_t lm_midgim_g3_reference_offsets[9]={
    0,66,133,195,263,331,397,462,527,
};
const uint8_t lm_midgim_g3_reference_stream[527]={
    7,8,0,0,0,4,2,0,128,26,0,36,2,27,1,151,
    104,4,3,7,3,6,30,39,151,72,10,32,11,19,12,33,
    13,1,14,9,41,43,15,5,16,31,17,46,5,26,18,3,
    37,34,38,1,20,48,21,45,22,5,23,31,24,34,25,1,
    0,0,128,7,8,0,0,0,4,2,0,128,26,0,36,2,
    27,1,151,104,4,3,7,3,6,30,39,151,72,10,32,11,
    19,12,33,13,1,14,9,41,43,15,5,16,31,17,46,5,
    26,18,3,37,34,38,1,20,48,21,45,22,5,23,31,24,
    34,25,1,0,0,128,14,8,0,0,0,4,2,0,128,24,
    0,36,2,27,1,185,207,4,3,7,3,6,30,10,32,11,
    19,12,33,13,1,14,9,15,29,16,28,17,0,5,26,18,
    3,37,35,38,0,20,47,21,44,22,29,23,28,24,35,25,
    0,126,6,128,22,8,0,0,0,4,2,0,128,27,0,36,
    2,27,1,250,41,4,3,7,3,6,30,39,250,9,10,32,
    11,19,12,33,13,1,14,9,41,43,15,5,16,31,17,46,
    128,115,0,5,26,18,3,37,34,38,1,20,48,21,45,22,
    5,23,31,24,34,25,1,129,70,8,0,0,0,4,2,0,
    40,0,36,2,27,1,144,91,4,3,7,244,2,6,30,10,
    32,11,19,12,33,13,1,14,9,15,29,16,28,17,0,128,
    115,0,5,26,18,244,2,129,83,35,129,84,0,20,47,21,
    44,22,29,23,28,24,35,25,0,126,4,152,6,8,2,6,
    0,4,2,6,40,0,36,2,27,1,239,9,4,3,7,3,
    6,30,10,32,11,19,12,33,13,1,14,9,41,43,15,5,
    16,31,17,46,5,26,18,3,129,215,34,129,216,1,20,48,
    21,45,22,5,23,31,24,34,25,1,0,126,3,125,254,255,
    8,2,42,0,4,2,42,40,0,36,2,27,1,171,42,4,
    3,7,49,6,30,39,171,10,10,32,11,19,12,33,13,1,
    14,9,15,29,16,28,17,0,5,26,18,49,37,35,38,0,
    20,47,21,44,22,29,23,28,24,35,25,0,126,4,125,255,
    255,8,2,42,0,4,2,42,40,0,36,2,27,1,171,42,
    4,3,7,49,6,30,39,171,10,10,32,11,19,12,33,13,
    1,14,9,15,29,16,28,17,0,5,26,18,49,37,35,38,
    0,20,47,21,44,22,29,23,28,24,35,25,0,126,4,
};
#else
extern const uint16_t lm_midgim_g3_reference_dictionary[50];
extern const uint16_t lm_midgim_g3_reference_offsets[9];
extern const uint8_t lm_midgim_g3_reference_stream[527];
#endif
static inline uint8_t lm_midgim_g3_reference_begin(frozen_words *r, unsigned row) {
    if(row>=LM_MIDGIM_G3_FROZEN_ROWS) return 0;
    frozen_words_init(r,lm_midgim_g3_reference_stream+lm_midgim_g3_reference_offsets[row],
        (uint16_t)(lm_midgim_g3_reference_offsets[row+1]-lm_midgim_g3_reference_offsets[row]),
        lm_midgim_g3_reference_dictionary,50);
    return 1;
}
#endif
