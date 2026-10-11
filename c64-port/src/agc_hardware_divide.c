/* SPDX-License-Identifier: MIT */
#include "agc_hardware_divide.h"
/* Independent integer residual algebra. Radix16384 has14 quotient digits;
 * the stored negative residual is reduced modulo the radix before doubling.
 * Trial-add accepts a digit1 exactly when the trial residual remains negative.
 * This bounded arithmetic also defines the historical overflow results.
 * Zero signs are representation bits, separate from numerical magnitudes. */
agc_register_pair agc_hardware_divide(agc_register_pair dividend,agc_register divisor){
    const uint16_t negative_bit=UINT16_C(0100000),radix=UINT16_C(040000);
    uint16_t numerator_negative=(uint16_t)(dividend.a&negative_bit);
    uint16_t residual=numerator_negative?dividend.a:(uint16_t)(dividend.a^UINT16_MAX);
    uint16_t low,low_negative,divisor_negative=(uint16_t)(divisor&negative_bit),magnitude_divisor;
    uint16_t payload,digit_weight,quotient=0;agc_register_pair out;
    dividend.l=agc_extend(agc_correct(dividend.l));
    if(residual==UINT16_MAX)numerator_negative=(uint16_t)(dividend.l&negative_bit);
    low=numerator_negative?(uint16_t)(dividend.l^UINT16_MAX):dividend.l;
    low=agc_add_register(low,radix);
    if(agc_overflow(low)!=1)residual=agc_add_register(residual,1);
    low_negative=(uint16_t)(low&negative_bit);payload=(uint16_t)(low&(radix-1u));
    magnitude_divisor=divisor_negative?(uint16_t)(divisor^UINT16_MAX):divisor;
    for(digit_weight=radix/2u;digit_weight;digit_weight=(uint16_t)(digit_weight/2u)){
        uint16_t negative=(uint16_t)(residual&negative_bit);
        uint16_t magnitude=negative?(uint16_t)(UINT16_MAX-residual):residual;
        uint16_t incoming=(uint16_t)((payload&digit_weight)!=0);
        uint16_t doubled=(uint16_t)(2u*(magnitude%radix)+(negative?incoming:0u));
        uint16_t folded=negative?(uint16_t)(UINT16_MAX-doubled):doubled;
        uint16_t trial=agc_add_register(folded,magnitude_divisor);
        uint16_t accepted=(uint16_t)((trial&negative_bit)!=0);
        quotient=(uint16_t)(2u*quotient+accepted);residual=accepted?trial:folded;
    }
    out.a=(uint16_t)(quotient+(low_negative?UINT16_C(0140000):0u));
    if(numerator_negative!=divisor_negative)out.a=(uint16_t)(out.a^UINT16_MAX);
    out.l=numerator_negative?residual:(uint16_t)(residual^UINT16_MAX);
    out.l=agc_extend(agc_correct(out.l));
    return out;
}
