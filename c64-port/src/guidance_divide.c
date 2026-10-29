/* SPDX-License-Identifier: MIT */
#include "guidance_divide.h"
static agc_register indicator(int sign) { return sign>0 ? 1 : 0177776; }
/* TPAGREE with its third word cleared by BUFZERO. Preserve independent raw
 * word effects, skipped CAF ZERO stores and both forms of zero. */
static void agree(p66_preparation *s) {
    agc_register a; agc_word limit; int sign,overflow; uint8_t i;
    s->mpac[2]=0; s->rate.registers.l=02365; /* Original TPAGREE return in L. */
    sign=agc_to_int(s->mpac[0]);
    if(sign==0) sign=agc_to_int(s->mpac[1]);
    if(sign==0) { s->mpac[0]=s->mpac[1]=0; s->rate.registers.a=0; return; }
    limit=sign>0 ? 037777 : 040000;
    a=agc_add_register(agc_extend(limit),indicator(sign));
    for(i=2;i>0;--i) {
        a=agc_add_register(a,agc_extend(s->mpac[i])); s->mpac[i]=agc_correct(a);
        overflow=agc_overflow(a); a=overflow ? indicator(overflow) : 0;
        a=agc_add_register(a,agc_extend(limit));
    }
    a=agc_add_register(a,agc_extend(s->mpac[0])); s->mpac[0]=agc_correct(a);
    overflow=agc_overflow(a);
    if(overflow) a=indicator(overflow); else s->mpac[1]=agc_correct(a);
    s->rate.registers.a=a;
}
enum p66_divide_result p66_divide_four_seconds(p66_preparation *s,
    p66_scale_scratch *scale,p66_elapsed_state *elapsed,p66_divide_scratch *d) {
    int16_t high,minor; unsigned negative;
    uint16_t magnitude;
    uint32_t numerator,remainder,low;
    if(s->mpac[7]!=0) return P66_DIVIDE_OUT_OF_SCOPE;
    elapsed->buf=0; d->buf_low=400;
    d->dvsign=d->dvnormct=d->maxdvsw=077776;
    agree(s); high=agc_to_int(s->mpac[0]); minor=agc_to_int(s->mpac[1]);
    negative=(unsigned)(high!=0 ? high<0 : minor<0);
    if(negative) d->dvsign=077777;
    if(high!=0) goto overflow;
    /* BUFZERO shifts both pairs by one word. The CCS zero paths supply +0. */
    elapsed->buf=400; d->buf_low=0;
    s->mpac[0]=s->mpac[1]; s->mpac[1]=0;
    if(minor==0) { s->rate.registers.a=0; return P66_DIVIDE_ZERO; }
    magnitude=(uint16_t)(minor<0 ? -minor : minor);
    if(magnitude>=400u) {
        if(magnitude==400u) d->maxdvsw=0;
        goto overflow;
    }
    /* Fixed divisor: five left shifts normalize 400 to 12800. Its low
     * word stays zero, so GENDDV's two DV stages are exact integer ratios.
     * Divide in two 14-bit stages; no 64-bit or floating-point operation. */
    elapsed->buf=12800; d->dvnormct=077771;
    numerator=(uint32_t)magnitude*16384u;
    s->mpac[0]=(agc_word)(numerator/400u); remainder=(numerator%400u)*32u;
    low=remainder*16384u;
    s->mpac[1]=(agc_word)(low/12800u);
    s->rate.registers.l=agc_extend((agc_word)(low%12800u));
    if(negative) {
        s->rate.registers.l=agc_extend(s->mpac[1]);
        s->mpac[0]=agc_negate(s->mpac[0]); s->mpac[1]=agc_negate(s->mpac[1]);
    }
    s->mpac[2]=0; s->rate.registers.a=0;
    return P66_DIVIDE_QUOTIENT;
overflow:
    s->mpac[0]=s->mpac[1]=negative ? 040000 : 037777;
    if(negative) s->rate.registers.l=037777;
    scale->ovfind=1; s->rate.registers.a=1;
    return P66_DIVIDE_OVERFLOW;
}
