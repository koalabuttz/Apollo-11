/* SPDX-License-Identifier: MIT */
#include "agc_multiword.h"
agc_register_pair agc_multiply(agc_register accumulator, agc_word multiplier) {
    agc_word operand = agc_correct(accumulator);
    agc_register_pair result;
    uint32_t product;
    agc_word high, low;
    unsigned negative = (unsigned)((operand ^ multiplier) & AGC_SIGN);
    /* A zero memory operand always produces +0, even with negative A. */
    if (multiplier == AGC_POS_ZERO || multiplier == AGC_NEG_ZERO) {
        result.a = result.l = 0;
        return result;
    }
    product = (uint32_t)((operand & AGC_SIGN) ? agc_negate(operand) : operand)
            * (uint32_t)((multiplier & AGC_SIGN) ? agc_negate(multiplier) : multiplier);
    high = (agc_word)(product >> 14);
    low = (agc_word)(product & UINT32_C(037777));
    if (negative != 0) { high = agc_negate(high); low = agc_negate(low); }
    result.a = agc_extend(high); result.l = agc_extend(low);
    return result;
}
void agc_double_add(agc_register_pair *registers, agc_dp *destination) {
    agc_register low = agc_add_register(registers->l, agc_extend(destination->low));
    agc_register high = agc_add_register(registers->a, agc_extend(destination->high));
    int carry = agc_overflow(low);
    int overflow;
    if (carry > 0) high = agc_add_register(high, 1);
    else if (carry < 0) high = agc_add_register(high, agc_extend(agc_from_int(-1)));
    overflow = agc_overflow(high);
    destination->high = agc_correct(high); destination->low = agc_correct(low);
    registers->a = overflow > 0 ? 1 : overflow < 0 ? agc_extend(agc_from_int(-1)) : 0;
    registers->l = 0;
}
