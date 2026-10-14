/* SPDX-License-Identifier: MIT */
/* The IMU CDU drive (resident_cdu.h; yaAGC agc_engine.c BurstOutput).
 * Time is in 2.5 ms units: a period adds 4, bursts are 30 apart. A burst is
 * due once more than 30 have passed: yaAGC's burst comes an instruction
 * after the 75 ms mark, so it shows at the end of the period after it. */
#include "resident_cdu.h"

rk_cdu_state rk_cdu;
void rk_cdu_drive(apollo_overlay_state *s, agc_hardware *hw) {
    agc_word *w=s->words, ch14=hw->channel14;
    int16_t *o, v;
    agc_word *c, bit;
    uint8_t burst=!rk_cdu.active;
    if(rk_cdu.elapsed<0xfff0u) rk_cdu.elapsed=(uint16_t)(rk_cdu.elapsed+4u);
    if(!(ch14&070000u)) return;
    if(burst) rk_cdu.elapsed=34u;
    while(rk_cdu.elapsed>30u) {
        rk_cdu.elapsed=(uint16_t)(rk_cdu.elapsed-30u);
        o=rk_cdu.owed; c=&w[050]; bit=040000u; burst=0;
        do {
            v=*o;
            if(ch14&bit) {
                /* + the count, a one's complement word: -0 is 0. */
                v=(int16_t)(v+(int16_t)(*c&040000u ? *c-077777u : *c));
                *c=0;
            }
            /* At most 192 / COARSE_SMOOTH (24) pulses per burst. */
            v=v>24?(int16_t)(v-24):v<-24?(int16_t)(v+24):0;
            *o++=v;
            if(v) burst=1;
            ++c;
        } while((bit>>=1)!=004000u);
        if(!(rk_cdu.active=burst)) break;
    }
}
