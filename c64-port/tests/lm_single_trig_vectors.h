/* SPDX-License-Identifier: MIT
 * Complete2048-word source-image delta fixtures. Original corpusSHA256:48222b46a09e4d6fe71e08b6cc77891b4f113b69b9e9a42b3e9b508067d99703
 * ConstructorSHA256:df25c7b000d3b4e179b22daeefc92dff1abd8a303eb518e0abe879241db9b50b */
#ifndef LM_SINGLE_TRIG_VECTORS_H
#define LM_SINGLE_TRIG_VECTORS_H
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==0
#include "frozen_words.h"
#define LM_SINGLE_TRIG0_FROZEN_ROWS 32u
#define LM_SINGLE_TRIG0_FROZEN_WORDS 13u
#define LM_SINGLE_TRIG0_FROZEN_STORAGE_BYTES 586u
#ifdef LM_SINGLE_TRIG0_FROZEN_IMPLEMENTATION
const uint16_t lm_single_trig0_reference_dictionary[21]={
    0,5,1,574,575,2614,3,2617,16383,32764,49152,16380,6,4,9,2316,
    16372,32767,49163,63219,65532,
};
const uint16_t lm_single_trig0_reference_offsets[33]={
    0,11,24,39,53,69,85,101,118,135,151,165,181,195,208,222,
    236,250,263,277,290,305,321,334,349,366,381,394,414,430,445,463,
    478,
};
const uint8_t lm_single_trig0_reference_stream[478]={
    0,0,6,1,5,3,0,4,0,126,4,2,2,1,0,6,
    2,15,1,5,3,6,4,0,128,2,0,1,0,12,2,146,
    24,1,5,3,12,4,0,6,2,1,0,14,2,155,36,1,
    5,3,14,4,0,13,0,1,0,128,12,2,164,48,1,5,
    3,128,12,4,0,1,2,1,0,128,15,2,173,60,1,5,
    3,128,15,4,0,12,0,1,0,128,18,2,182,72,1,5,
    3,128,18,4,0,128,7,2,1,0,128,21,2,191,84,1,
    5,3,128,21,4,0,128,8,0,1,0,128,25,2,136,96,
    1,5,3,128,25,4,0,14,2,1,0,128,28,2,145,108,
    1,5,3,128,28,4,0,159,255,2,1,0,8,2,16,1,
    7,3,6,4,11,160,0,0,13,0,8,2,125,0,224,1,
    7,3,0,0,0,160,1,2,1,0,8,2,16,1,7,3,
    6,4,11,8,2,1,0,6,2,15,1,5,3,6,4,0,
    192,0,0,1,0,0,2,0,1,5,3,0,4,0,192,1,
    2,1,0,20,2,19,1,5,3,9,4,0,223,255,2,1,
    0,10,2,18,1,7,3,9,4,11,224,0,0,6,0,10,
    1,7,3,17,0,126,3,224,1,2,1,0,10,2,18,1,
    7,3,9,4,11,17,2,1,0,20,2,19,1,5,3,9,
    4,0,125,0,128,0,1,0,6,2,15,1,5,3,6,4,
    0,125,1,128,2,1,0,12,2,146,24,1,5,3,12,4,
    0,125,255,159,2,6,0,8,1,7,3,0,126,4,125,0,
    160,0,1,0,8,2,16,1,7,3,6,4,11,125,1,160,
    2,1,0,8,2,191,224,1,7,3,6,4,191,248,125,255,
    191,2,1,0,0,2,0,1,5,3,0,4,0,10,0,1,
    0,20,2,19,1,5,3,9,4,0,125,1,192,2,1,0,
    125,249,255,2,125,231,237,1,5,3,255,249,4,0,125,255,
    223,2,13,0,10,2,159,219,1,7,3,17,0,0,125,0,
    224,0,1,0,10,2,18,1,7,3,9,4,11,125,1,224,
    2,1,0,10,2,125,31,192,1,7,3,9,4,191,248,125,
    255,255,2,1,0,0,2,0,1,5,3,0,4,0,
};
#else
extern const uint16_t lm_single_trig0_reference_dictionary[21];
extern const uint16_t lm_single_trig0_reference_offsets[33];
extern const uint8_t lm_single_trig0_reference_stream[478];
#endif
static inline uint8_t lm_single_trig0_reference_begin(frozen_words *r, unsigned row) {
    if(row>=LM_SINGLE_TRIG0_FROZEN_ROWS) return 0;
    frozen_words_init(r,lm_single_trig0_reference_stream+lm_single_trig0_reference_offsets[row],
        (uint16_t)(lm_single_trig0_reference_offsets[row+1]-lm_single_trig0_reference_offsets[row]),
        lm_single_trig0_reference_dictionary,21);
    return 1;
}
#endif
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==1
#include "frozen_words.h"
#define LM_SINGLE_TRIG1_FROZEN_ROWS 32u
#define LM_SINGLE_TRIG1_FROZEN_WORDS 13u
#define LM_SINGLE_TRIG1_FROZEN_STORAGE_BYTES 579u
#ifdef LM_SINGLE_TRIG1_FROZEN_IMPLEMENTATION
const uint16_t lm_single_trig1_reference_dictionary[22]={
    0,5,1,574,575,3,2617,16383,2614,32764,49152,6,16380,4,2316,16352,
    16372,16376,32767,49163,63219,65532,
};
const uint16_t lm_single_trig1_reference_offsets[33]={
    0,11,24,38,52,67,82,97,113,128,144,158,172,186,199,214,
    228,242,256,271,286,301,316,331,346,366,380,393,410,425,440,456,
    469,
};
const uint8_t lm_single_trig1_reference_stream[469]={
    0,0,5,0,7,1,6,3,0,126,4,2,2,1,0,7,
    2,16,1,6,3,5,4,12,128,2,0,1,0,7,2,15,
    1,6,3,5,4,17,5,2,1,0,7,2,191,196,1,6,
    3,5,4,16,13,0,1,0,7,2,191,160,1,6,3,5,
    4,191,240,1,2,1,0,7,2,191,116,1,6,3,5,4,
    191,236,11,0,1,0,7,2,191,64,1,6,3,5,4,191,
    232,128,7,2,1,0,7,2,191,32,1,6,3,2,4,191,
    228,128,8,0,1,0,7,2,190,192,1,6,3,5,4,15,
    128,9,2,1,0,7,2,190,116,1,6,3,5,4,191,220,
    159,255,2,1,0,5,2,14,1,8,3,5,4,0,160,0,
    0,1,0,0,2,0,1,8,3,0,4,0,160,1,2,1,
    0,21,2,20,1,8,3,9,4,0,7,2,1,0,10,2,
    19,1,6,3,9,4,12,192,0,0,13,0,10,2,159,255,
    1,6,3,18,0,0,192,1,2,1,0,10,2,19,1,6,
    3,9,4,12,223,255,2,1,0,21,2,20,1,8,3,9,
    4,0,224,0,0,1,0,5,2,14,1,8,3,5,4,0,
    224,1,2,1,0,11,2,146,24,1,8,3,11,4,0,18,
    2,13,0,7,2,125,218,223,1,6,3,0,0,0,125,0,
    128,0,1,0,7,2,16,1,6,3,5,4,12,125,1,128,
    2,1,0,7,2,15,1,6,3,5,4,17,125,255,159,2,
    1,0,0,2,0,1,8,3,0,4,0,125,0,160,0,1,
    0,21,2,20,1,8,3,9,4,0,125,1,160,2,1,0,
    125,249,255,2,125,231,237,1,8,3,255,249,4,0,125,255,
    191,2,5,0,10,1,6,3,18,0,126,3,10,0,13,2,
    19,1,6,3,9,4,12,0,0,125,1,192,2,1,0,10,
    2,125,31,192,1,6,3,9,4,17,125,255,223,2,1,0,
    0,2,0,1,8,3,0,4,0,125,0,224,0,1,0,5,
    2,14,1,8,3,5,4,0,125,1,224,2,1,0,11,2,
    146,24,1,8,3,11,4,0,125,255,255,2,5,0,7,1,
    6,3,0,126,4,
};
#else
extern const uint16_t lm_single_trig1_reference_dictionary[22];
extern const uint16_t lm_single_trig1_reference_offsets[33];
extern const uint8_t lm_single_trig1_reference_stream[469];
#endif
static inline uint8_t lm_single_trig1_reference_begin(frozen_words *r, unsigned row) {
    if(row>=LM_SINGLE_TRIG1_FROZEN_ROWS) return 0;
    frozen_words_init(r,lm_single_trig1_reference_stream+lm_single_trig1_reference_offsets[row],
        (uint16_t)(lm_single_trig1_reference_offsets[row+1]-lm_single_trig1_reference_offsets[row]),
        lm_single_trig1_reference_dictionary,22);
    return 1;
}
#endif
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==2
#include "frozen_words.h"
#define LM_SINGLE_TRIG2_FROZEN_ROWS 32u
#define LM_SINGLE_TRIG2_FROZEN_WORDS 15u
#define LM_SINGLE_TRIG2_FROZEN_STORAGE_BYTES 673u
#ifdef LM_SINGLE_TRIG2_FROZEN_IMPLEMENTATION
const uint16_t lm_single_trig2_reference_dictionary[26]={
    0,1,3,5,2,6,574,575,16383,32764,49152,16380,4,9,1024,1025,
    2047,2316,3071,3072,3073,16372,32767,49163,63219,65532,
};
const uint16_t lm_single_trig2_reference_offsets[33]={
    0,13,28,45,62,81,100,119,139,159,178,194,212,228,243,260,
    277,294,309,325,340,357,375,390,408,428,446,461,483,501,518,538,
    555,
};
const uint8_t lm_single_trig2_reference_stream[555]={
    0,0,12,4,2,3,14,6,0,7,0,126,4,1,1,5,
    0,2,1,17,4,2,3,15,6,2,7,0,4,0,5,0,
    5,1,146,24,4,2,3,132,2,6,5,7,0,2,1,5,
    0,13,1,155,36,4,2,3,132,3,6,13,7,0,12,0,
    5,0,128,12,1,164,48,4,2,3,132,4,6,128,12,7,
    0,3,1,5,0,128,15,1,173,60,4,2,3,132,5,6,
    128,15,7,0,5,0,5,0,128,18,1,182,72,4,2,3,
    132,6,6,128,18,7,0,128,7,1,5,0,128,21,1,191,
    84,4,2,3,132,7,6,128,21,7,0,128,8,0,5,0,
    128,25,1,136,96,4,2,3,132,8,6,128,25,7,0,13,
    1,5,0,128,28,1,145,108,4,2,3,132,9,6,128,28,
    7,0,159,255,1,5,0,8,1,21,4,2,3,18,6,2,
    7,11,160,0,0,3,0,8,1,125,0,224,4,2,3,19,
    6,0,0,0,160,1,1,5,0,8,1,21,4,2,3,20,
    6,2,7,11,8,1,5,0,2,1,17,4,2,3,16,6,
    2,7,0,192,0,0,5,0,0,1,0,4,2,3,136,0,
    6,0,7,0,192,1,1,5,0,25,1,24,4,2,3,136,
    1,6,9,7,0,223,255,1,5,0,10,1,23,4,2,3,
    143,255,6,9,7,11,224,0,0,12,0,10,4,2,3,14,
    6,22,0,126,3,224,1,1,5,0,10,1,23,4,2,3,
    15,6,9,7,11,22,1,5,0,25,1,24,4,2,3,18,
    6,9,7,0,125,0,128,0,5,0,2,1,17,4,2,3,
    19,6,2,7,0,125,1,128,1,5,0,5,1,146,24,4,
    2,3,20,6,5,7,0,125,255,159,1,12,0,8,4,2,
    3,16,6,0,126,4,125,0,160,0,5,0,8,1,21,4,
    2,3,136,0,6,2,7,11,125,1,160,1,5,0,8,1,
    191,224,4,2,3,136,1,6,2,7,191,248,125,255,191,1,
    5,0,0,1,0,4,2,3,143,255,6,0,7,0,10,0,
    5,0,25,1,24,4,2,3,14,6,9,7,0,125,1,192,
    1,5,0,125,249,255,1,125,231,237,4,2,3,15,6,255,
    249,7,0,125,255,223,1,3,0,10,1,159,219,4,2,3,
    18,6,22,0,0,125,0,224,0,5,0,10,1,23,4,2,
    3,19,6,9,7,11,125,1,224,1,5,0,10,1,125,31,
    192,4,2,3,20,6,9,7,191,248,125,255,255,1,5,0,
    0,1,0,4,2,3,16,6,0,7,0,
};
#else
extern const uint16_t lm_single_trig2_reference_dictionary[26];
extern const uint16_t lm_single_trig2_reference_offsets[33];
extern const uint8_t lm_single_trig2_reference_stream[555];
#endif
static inline uint8_t lm_single_trig2_reference_begin(frozen_words *r, unsigned row) {
    if(row>=LM_SINGLE_TRIG2_FROZEN_ROWS) return 0;
    frozen_words_init(r,lm_single_trig2_reference_stream+lm_single_trig2_reference_offsets[row],
        (uint16_t)(lm_single_trig2_reference_offsets[row+1]-lm_single_trig2_reference_offsets[row]),
        lm_single_trig2_reference_dictionary,26);
    return 1;
}
#endif
#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED==3
#include "frozen_words.h"
#define LM_SINGLE_TRIG3_FROZEN_ROWS 32u
#define LM_SINGLE_TRIG3_FROZEN_WORDS 15u
#define LM_SINGLE_TRIG3_FROZEN_STORAGE_BYTES 666u
#ifdef LM_SINGLE_TRIG3_FROZEN_IMPLEMENTATION
const uint16_t lm_single_trig3_reference_dictionary[27]={
    0,1,3,5,2,574,6,575,16383,32764,49152,16380,4,1024,1025,2047,
    2316,3071,3072,3073,16352,16372,16376,32767,49163,63219,65532,
};
const uint16_t lm_single_trig3_reference_offsets[33]={
    0,13,28,44,61,79,97,115,134,152,171,187,203,219,234,252,
    269,286,302,319,336,353,370,387,405,428,445,460,479,496,513,531,
    546,
};
const uint8_t lm_single_trig3_reference_stream[546]={
    0,0,12,0,8,4,2,3,13,5,0,126,4,1,1,6,
    0,8,1,21,4,2,3,14,5,2,7,11,4,0,6,0,
    8,1,20,4,2,3,132,2,5,2,7,22,2,1,6,0,
    8,1,191,196,4,2,3,132,3,5,2,7,21,12,0,6,
    0,8,1,191,160,4,2,3,132,4,5,2,7,191,240,3,
    1,6,0,8,1,191,116,4,2,3,132,5,5,2,7,191,
    236,6,0,6,0,8,1,191,64,4,2,3,132,6,5,2,
    7,191,232,128,7,1,6,0,8,1,191,32,4,2,3,132,
    7,5,1,7,191,228,128,8,0,6,0,8,1,190,192,4,
    2,3,132,8,5,2,7,20,128,9,1,6,0,8,1,190,
    116,4,2,3,132,9,5,2,7,191,220,159,255,1,6,0,
    2,1,16,4,2,3,17,5,2,7,0,160,0,0,6,0,
    0,1,0,4,2,3,18,5,0,7,0,160,1,1,6,0,
    26,1,25,4,2,3,19,5,9,7,0,8,1,6,0,10,
    1,24,4,2,3,15,5,9,7,11,192,0,0,3,0,10,
    1,159,255,4,2,3,136,0,5,23,0,0,192,1,1,6,
    0,10,1,24,4,2,3,136,1,5,9,7,11,223,255,1,
    6,0,26,1,25,4,2,3,143,255,5,9,7,0,224,0,
    0,6,0,2,1,16,4,2,3,13,5,2,7,0,224,1,
    1,6,0,6,1,146,24,4,2,3,14,5,6,7,0,23,
    1,3,0,8,1,125,218,223,4,2,3,17,5,0,0,0,
    125,0,128,0,6,0,8,1,21,4,2,3,18,5,2,7,
    11,125,1,128,1,6,0,8,1,20,4,2,3,19,5,2,
    7,22,125,255,159,1,6,0,0,1,0,4,2,3,15,5,
    0,7,0,125,0,160,0,6,0,26,1,25,4,2,3,136,
    0,5,9,7,0,125,1,160,1,6,0,125,249,255,1,125,
    231,237,4,2,3,136,1,5,255,249,7,0,125,255,191,1,
    12,0,10,4,2,3,143,255,5,23,0,126,3,10,0,3,
    1,24,4,2,3,13,5,9,7,11,0,0,125,1,192,1,
    6,0,10,1,125,31,192,4,2,3,14,5,9,7,22,125,
    255,223,1,6,0,0,1,0,4,2,3,17,5,0,7,0,
    125,0,224,0,6,0,2,1,16,4,2,3,18,5,2,7,
    0,125,1,224,1,6,0,6,1,146,24,4,2,3,19,5,
    6,7,0,125,255,255,1,12,0,8,4,2,3,15,5,0,
    126,4,
};
#else
extern const uint16_t lm_single_trig3_reference_dictionary[27];
extern const uint16_t lm_single_trig3_reference_offsets[33];
extern const uint8_t lm_single_trig3_reference_stream[546];
#endif
static inline uint8_t lm_single_trig3_reference_begin(frozen_words *r, unsigned row) {
    if(row>=LM_SINGLE_TRIG3_FROZEN_ROWS) return 0;
    frozen_words_init(r,lm_single_trig3_reference_stream+lm_single_trig3_reference_offsets[row],
        (uint16_t)(lm_single_trig3_reference_offsets[row+1]-lm_single_trig3_reference_offsets[row]),
        lm_single_trig3_reference_dictionary,27);
    return 1;
}
#endif
#endif
