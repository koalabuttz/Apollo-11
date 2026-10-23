/* SPDX-License-Identifier: MIT */
/* The IMU CDU drive (resident_cdu.h; yaAGC agc_engine.c BurstOutput).
 * Time is in 2.5 ms units: a period adds 4, bursts are 30 apart. A burst is
 * due once more than 30 have passed: yaAGC's burst comes an instruction
 * after the 75 ms mark, so it shows at the end of the period after it. */
#include "resident_cdu.h"
#include "agc_image_w.h"

rk_cdu_state rk_cdu;
/* The counters driven: RK_CDU_N from 050, their channel 14 bits from 15
 * down (as bits of the channel's high byte, 0100 first). */
#if defined(RK_CM)
#define RK_CDU_N 5u
#elif defined(RK_LM)
#define RK_CDU_N 3u
#else
uint8_t rk_cdu_count = 3u;
#define RK_CDU_N rk_cdu_count
#endif
void rk_cdu_drive(apollo_overlay_state *s, agc_hardware *hw) {
    agc_word w;
    uint8_t hi = (uint8_t)(hw->channel14 >> 8), bit, i, burst = !rk_cdu.active;
    int16_t v;
    uint8_t cap, neg;
    if(rk_cdu.elapsed<0xfff0u) rk_cdu.elapsed=(uint16_t)(rk_cdu.elapsed+4u);
    /* None of the counters' bits: nothing moves. */
    if (!(hi & (uint8_t)(0200u - (0200u >> RK_CDU_N)))) return;
    if(burst) rk_cdu.elapsed=34u;
    while(rk_cdu.elapsed>30u) {
        rk_cdu.elapsed=(uint16_t)(rk_cdu.elapsed-30u);
        burst = 0;
        for (i = 0, bit = 0100u; i < RK_CDU_N; ++i, bit >>= 1) {
            v = rk_cdu.owed[i];
            if (hi & bit) {
                /* + the count, a one's complement word: -0 is 0. */
                w = W[050u + i];
                v = (int16_t)(v + (int16_t)w);
                if (w & 040000u) v = (int16_t)(v - 077777);
                W[050u + i] = 0;
            }
            /* At most 192 / COARSE_SMOOTH (24) IMU pulses per burst, 240
             * optics pulses: the magnitude less the cap, never past 0. */
            neg = v < 0;
            if (neg) v = (int16_t)-v;
            cap = i < 3u ? 24u : 240u;
            v = (uint16_t)v > cap ? (int16_t)(v - cap) : 0;
            if (v) {
                burst = 1;
                if (neg) v = (int16_t)-v;
            }
            rk_cdu.owed[i] = v;
        }
        if(!(rk_cdu.active=burst)) break;
    }
}
