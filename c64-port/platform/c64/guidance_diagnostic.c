/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "guidance_cases.h"
#include "guidance_vectors.h"
static guidance_case c;
static uint8_t enter(void) {
    uint8_t p;
    __asm__ volatile("php\nsei\npla" : "=a"(p) : : "memory"); return p;
}
static void leave(uint8_t p) {
    if ((p & 4u) == 0) __asm__ volatile("cli" : : : "memory");
    else __asm__ volatile("sei" : : : "memory");
}
int main(void) {
    volatile uint8_t *mail = (volatile uint8_t *)0xc080;
    /* Positive/negative multiply, low carry, high overflow, zero/full ROD. */
    static const uint16_t samples[6] = {130,131,280,178,400,543};
    unsigned failures = 0;
    uint8_t i, token;
    mail[0] = 0;
    puts("APOLLO NATIVE P66 RATE UPDATE");
    failures += gn_check(&c);
    for (i = 0; i < 6; ++i) {
        uint16_t words[6]; unsigned j;
        uint8_t p;
        gn_prepare(&c,samples[i]); token = enter();
        mail[8] = (uint8_t)(0x10u + i);
        gn_apply(&c);
        mail[8] = (uint8_t)(0x20u + i);
        leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if ((p & 4u) != (token & 4u) || (i == 0 && (p & 4u) != 0)) ++failures;
        gn_words(&c,words);
        for (j = 0; j < 6; ++j) if (words[j] != guidance_reference_words[samples[i]][j]) ++failures;
    }
    /* The caller's already-masked state survives an owned rate update. */
    token = enter();
    {
        uint8_t nested = enter(), p;
        p66_rate_update(&c.state); leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if ((p & 4u) == 0) ++failures;
    }
    leave(token);
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)GN_TARGET_ROWS; mail[4] = (uint8_t)(GN_TARGET_ROWS >> 8);
    mail[5] = 3; mail[6] = (uint8_t)sizeof(p66_rate_state); mail[7] = (uint8_t)sizeof(guidance_case);
    puts(failures ? "GUIDANCE TESTS FAILED" : "568 COMPLETE ASSEMBLY STATES PASS");
    puts("THREE REPEATED RATE HISTORIES PASS");
    mail[0] = 0xa5;
    return failures;
}
