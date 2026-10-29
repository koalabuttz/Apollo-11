/* SPDX-License-Identifier: MIT */
#include "agc_word.h"
agc_word agc_negate(agc_word value) { return (agc_word)(value ^ AGC_MASK); }
int16_t agc_to_int(agc_word value) {
    value &= AGC_MASK;
    if ((value & AGC_SIGN) != 0) return (int16_t)-(int16_t)agc_negate(value);
    return (int16_t)value;
}
agc_word agc_from_int(int16_t value) {
    if (value < 0) return agc_negate((agc_word)-value);
    return (agc_word)value;
}
int agc_compare(agc_word a, agc_word b) {
    int16_t x = agc_to_int(a), y = agc_to_int(b);
    return (x > y) - (x < y);
}
agc_word agc_add_word(agc_word a, agc_word b) {
    uint32_t sum = (uint32_t)a + b;
    return (agc_word)((sum & AGC_MASK) + (sum >> 15));
}
agc_register agc_extend(agc_word value) {
    return (agc_register)(value | ((uint32_t)(value & AGC_SIGN) << 1));
}
agc_word agc_correct(agc_register value) {
    return (agc_word)((value & UINT16_C(037777)) | ((value >> 1) & AGC_SIGN));
}
agc_register agc_add_register(agc_register a, agc_register b) {
    uint32_t sum = (uint32_t)a + b;
    return (agc_register)((sum & UINT32_C(0177777)) + (sum >> 16));
}
int agc_overflow(agc_register value) {
    uint16_t signs = (uint16_t)(value & UINT16_C(0140000));
    return signs == UINT16_C(040000) ? 1 : signs == UINT16_C(0100000) ? -1 : 0;
}
enum agc_ccs_class agc_classify(agc_word value) {
    if (value == AGC_POS_ZERO) return AGC_PLUS_ZERO;
    if (value == AGC_NEG_ZERO) return AGC_MINUS_ZERO;
    return (value & AGC_SIGN) != 0 ? AGC_NEGATIVE : AGC_POSITIVE;
}
agc_word agc_diminished_abs(agc_word value) {
    agc_word magnitude = (value & AGC_SIGN) != 0 ? agc_negate(value) : value;
    return magnitude == 0 ? 0 : (agc_word)(magnitude - 1u);
}
