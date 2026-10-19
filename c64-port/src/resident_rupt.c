/* SPDX-License-Identifier: MIT */
/* The flight resident's AGC timers and interrupt entry (resident_rupt.h). */
#include "resident_rupt.h"

/* Channel 13, the timers' T6 enable (bit 15) and radar activity (bit 4):
 * the program's agc_hardware's, unless the build keeps the flight
 * program's channel 13 in another record. The C64 CM resident
 * (tools/resident_flight_build.py, RK_DEVICE_CH13) runs its CM tables on
 * rk_device (cm_native_io), whose channel 13 T6START and the radar
 * routines write, so the timers read and clear that one. */
#if RK_DEVICE_CH13
#include "cm_native_image.h"
extern cm_native_io rk_device;
#define CH13(hw) rk_device.channel13
#else
#define CH13(hw) (hw)->channel13
#endif

/* yaAGC CounterPINC: +1 in one's complement, 037777 overflows to +0, -0
 * steps to +1. */
static uint8_t pinc(agc_word *w) {
    agc_word v=*w;
    if(v==037777u){*w=0;return 1;}
    v=(agc_word)((v+1u)&AGC_MASK);
    *w=v?v:1u;
    return 0;
}

/* The radar cycle (yaAGC): TIME5's step counts the radar gate while
 * channel 13 bit 4 (radar activity) is set, and after the ninth count the
 * cycle ends before the next TIME5 step (yaAGC: at SCALER1 036, 0.6 ms
 * before it), resetting bit 4 and requesting RADARUPT. Here it ends at the
 * TIME3 step between, the last step before 036: a RADARUPT that sets bit 4
 * again (P20-P25's NOEND/RESAMPLE) has it set for that next TIME5 count, as
 * on yaAGC, so each resample takes nine TIME5 steps, not ten. RNRAD keeps
 * its value (the flight program's radar data comes in through its io). */
static __attribute__((noinline)) void radar(agc_hardware *hw, uint16_t *pending, uint8_t phase)
{
    uint8_t n = hw->radar_gate;
    if (phase == RK_PHASE_T3) {
        if (n != 9u) return;
        n = 0;
        /* Low byte of channel 13, high byte of the requests (RADARUPT is
         * bit 9): both targets are little-endian, and a byte access is half
         * the 6502 code. */
        *(uint8_t *)&CH13(hw) &= (uint8_t)~010u;
        ((uint8_t *)pending)[1] |= (uint8_t)(RK_RUPT_BIT(RK_RUPT_RADAR) >> 8);
    } else if (*(uint8_t *)&CH13(hw) & 010u)
        ++n;
    hw->radar_gate = n;
}

#if RK_DOWNRUPT
/* The DOWNRUPT request below sets its bit in pending's high byte. */
typedef char rk_downrupt_in_high_byte[RK_RUPT_DOWN >= 8 && RK_RUPT_DOWN < 16 ? 1 : -1];
#endif

void rk_timer_phase(apollo_overlay_state *s, agc_hardware *hw, uint16_t *pending, uint8_t phase)
{
    /* Each step's counter and the request its overflow raises. */
    static const uint8_t counter[3] = {027u, 026u, 030u};
    static const uint8_t request[3] = {RK_RUPT_BIT(RK_RUPT_T4), RK_RUPT_BIT(RK_RUPT_T3),
                                       RK_RUPT_BIT(RK_RUPT_T5)};
    uint8_t i, n;
    agc_word *w=s->words;
    agc_word v;
    if (phase != RK_PHASE_T4) radar(hw, pending, phase);
    if (phase == RK_PHASE_T3 && pinc(&w[025])) pinc(&w[024]);
    if (pinc(&w[counter[phase]])) *pending |= request[phase];
    /* The scalers (channels 4 and 3) count AGC time too: SCALER1 8 counts
     * to each of the first two steps, 16 to the last, SCALER2 on its
     * overflow. */
    n = phase == RK_PHASE_T5 ? 16u : 8u;
    v = (agc_word)(hw->channel4 + n);
    if (v & 040000u) hw->channel3 = (agc_word)((hw->channel3 + 1u) & 037777u);
#if RK_DOWNRUPT
    /* DOWNRUPT whenever SCALER1 passes a multiple of 64 counts (20 ms;
     * tools/flight_downlink.h). A step is at most 16 counts, so a pass is
     * exactly a change in bit 6, a byte test. */
    if ((uint8_t)((uint8_t)v ^ (uint8_t)hw->channel4) & 0100u)
        ((uint8_t *)pending)[1] |= (uint8_t)(RK_RUPT_BIT(RK_RUPT_DOWN) >> 8);
#endif
    hw->channel4 = (agc_word)(v & 037777u);
    /* yaAGC CounterDINC on every odd SCALER1 count, half the counts: toward
     * zero by one through the one's complement adder, so +1 and -1 both
     * step to -0; at +0 or -0 the ZOUT. */
    for (i = 0, n >>= 1; i < n && (CH13(hw) & 040000u); ++i) {
        agc_word t = w[031];
        if (t == 0u || t == AGC_MASK) {
            *pending |= RK_RUPT_BIT(RK_RUPT_T6);
            CH13(hw) &= 037777u;
        } else
            w[031] = (agc_word)(t == 1u ? AGC_MASK : (t & AGC_SIGN) ? t + 1u : t - 1u);
    }
}

void rk_timer_tick(apollo_overlay_state *s, agc_hardware *hw, uint16_t *pending)
{
    uint8_t p;
    for (p = RK_PHASE_T4; p <= RK_PHASE_T5; ++p) rk_timer_phase(s, hw, pending, p);
}

static uint8_t word_at(const agc_native_program *p, const apollo_overlay_state *s, agc_word superbank,
                       agc_word z, agc_word *word) {
    if(z>=02000u)
        return p->fixed_data && p->fixed_data(p,agc_fixed_physical(z,s->words[4],superbank),word);
    *word=s->words[z<01400u ? z : (uint16_t)(((s->words[3]>>8)&7u)*0400u+(z&0377u))];
    return 1;
}

uint8_t rk_rupt_take(const agc_native_program *p, apollo_overlay_state *s, agc_word superbank,
                     agc_word interrupt_enabled, uint16_t *pending) {
    agc_word z=(agc_word)(s->words[5]&07777u),word;
    uint8_t i;
    if(!(*pending&03776u)||!p->hardware||p->hardware->in_interrupt||!interrupt_enabled||agc_overflow(s->words[0]))
        return 0;
    if(!word_at(p,s,superbank,z,&word)) return 0;
    word&=AGC_MASK;
    if(word==3u||word==4u||word==6u) return 0;
    for(i=RK_RUPT_T6;!(*pending&RK_RUPT_BIT(i));++i) {}
    *pending&=(uint16_t)~RK_RUPT_BIT(i);
    s->words[015]=(agc_word)(z+1u);
    s->words[017]=word;
    s->words[5]=(agc_word)(04000u+4u*i);
    p->hardware->in_interrupt=1;
    return i;
}
