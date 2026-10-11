/* SPDX-License-Identifier: MIT
 * Test-only ABI 2 module bodies (lane $0F); none translates an AGC routine.
 * OV2_MODULE selects 1 (job), 2 (large job) or 3 (task). */
#include "overlay_v2.h"
#include "agc_multiword.h"
#ifndef OV2_MODULE
#error OV2_MODULE required
#endif
static uint16_t transient;   /* .bss: must read zero after every load */
static uint8_t runtime_config(void) { return (uint8_t)(*(volatile uint8_t *)1 & 7u); }
#if OV2_MODULE==2
/* About 14 KB of read-only data, so a cold load moves a near-full slot. */
#define TABLE_BYTES 14000u
static const uint8_t table[TABLE_BYTES]={
#define R4(x) (uint8_t)(x),(uint8_t)((x)*7u),(uint8_t)((x)*13u),(uint8_t)((x)*31u)
#define R16(x) R4(x),R4(x+1u),R4(x+2u),R4(x+3u)
#define R64(x) R16(x),R16(x+4u),R16(x+8u),R16(x+12u)
#define R256(x) R64(x),R64(x+16u),R64(x+32u),R64(x+48u)
    R256(0u),R256(64u),R256(128u),R256(192u),R256(1u),R256(65u),R256(129u),R256(193u),
    R256(2u),R256(66u),R256(130u),R256(194u),R256(3u),R256(67u)
};
#endif
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector) {
    apollo_overlay_state *s=f->state;
    if(runtime_config()!=5u || transient!=0) { f->value=0xdead; return APOLLO_OV2_DONE; }
#if OV2_MODULE==1
    switch(selector) {
    case 0: { /* resident service through the $6F00 jump table: MP by one */
        agc_register_pair p=agc_multiply(s->words[0300],1u);
        if(p.a!=0u) { f->value=0xdead; return APOLLO_OV2_DONE; }
        s->words[0300]=(uint16_t)(p.l+1u); f->value=s->words[0300];
    }
        return APOLLO_OV2_DONE;
    case 1:   /* split routine: call into the task slot, resume at selector 2 */
        f->next.module=APOLLO_OV2_ID(APOLLO_LANE_PROOF,3); f->next.selector=0;
        f->ret.module=APOLLO_OV2_ID(APOLLO_LANE_PROOF,1); f->ret.selector=2;
        return APOLLO_OV2_CALL;
    case 2:
        s->words[0301]=(uint16_t)(s->words[0301]+f->value); f->value=s->words[0301];
        return APOLLO_OV2_DONE;
    default:  /* historical return address resolved by the store */
        f->hist_bank=024000; f->hist_loc=02100; return APOLLO_OV2_JUMP_HISTORICAL;
    }
#elif OV2_MODULE==2
    if(selector==0) {
        uint16_t i,sum=0;
        for(i=0;i<TABLE_BYTES;++i) sum=(uint16_t)(sum+table[i]);
        s->words[0302]=sum;
        f->next.module=APOLLO_OV2_ID(APOLLO_LANE_PROOF,1); f->next.selector=0;
        return 0x21u;   /* a kernel action handed back with its resume point */
    }
    f->value=(uint16_t)(0x5a00u|table[TABLE_BYTES-1u]); return APOLLO_OV2_DONE;
#else
    ++transient;
    s->words[0303]=(uint16_t)(s->words[0303]+1u);
    f->value=0x33;
    return selector ? APOLLO_OV2_DONE : APOLLO_OV2_RET;
#endif
}
