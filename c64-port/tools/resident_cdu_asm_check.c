/* SPDX-License-Identifier: MIT */
/* The C64 resident's output drives (platform/c64/resident_cdu.S,
 * rk_cdu_drive6502) in llvm-mos's simulator (mos-sim, whose clock() counts
 * cycles), built once for each mission (RK_LM, RK_CM):
 *  - yaAGC's IMU CDU timeline (tests/resident_cdu_cases.h, 65,536 periods,
 *    the frozen tests/resident_cdu_vectors.h), as tests/test_resident_cdu.c
 *    runs the C;
 *  - against src/resident_cdu.c's rk_cdu_drive, the reference, period by
 *    period on seeded inputs: any channel 14 high byte, writes to every
 *    counter the mission drives (edges, +0 and -0, large counts), long idle
 *    stretches and elapsed counts near the C's 0xfff0 stop. Each period
 *    compares 050-054 and the whole drive state;
 *  - in the CM, the optics counters' check from tests/test_resident_cdu.c.
 * Prints the periods, the mean cycles a period and the differences; exits
 * 1 on any difference.
 *
 * Build: mos-sim-clang -Oz -DRK_LM (or -DRK_CM) -Iinclude -Itests
 * tools/resident_cdu_asm_check.c tests/resident_cdu_cases.c
 * platform/c64/resident_cdu.S (tools/mk/resident-cdu-asm.mk). */
#include <stdio.h>
#include <time.h>
#include "resident_cdu_cases.h"
#include "resident_cdu_vectors.h"
/* The reference: the C drive over its own image and state. */
#define rk_cdu rk_cdu_c
#define rk_cdu_drive rk_cdu_drive_c
#include "../src/resident_cdu.c"
#undef rk_cdu
#undef rk_cdu_drive
rk_cdu_state rk_cdu;                   /* the .S's state */
void rk_cdu_drive6502(uint8_t hi);
#define IMG ((volatile agc_word *)APOLLO_SHARED_ADDRESS)  /* the .S's image */
#if defined(RK_CM)
#define MISSION "CM"
#else
#define MISSION "LM"
#endif

static apollo_overlay_state ref;
extern char __heap_start;              /* the end of the check's own memory */
static unsigned long seed = 1;
static unsigned rnd(void) {
    seed = seed * 1103515245ul + 12345ul;
    return (unsigned)(seed >> 16);
}
/* Cycles a period (with the call), apart for periods with none of the
 * drive's channel 14 bits (idle, most of flight) and with some. */
static long cycles[2], calls[2], overhead;
static void drive(uint8_t hi) {
    uint8_t on = (hi & (uint8_t)(0200u - (0200u >> RK_CDU_N))) != 0;
    long t0 = clock();
    rk_cdu_drive6502(hi);
    cycles[on] += clock() - t0 - overhead;
    ++calls[on];
}
static void clear(void) {
    uint8_t k;
    for (k = 0; k < 5u; ++k) IMG[050u + k] = ref.words[050u + k] = 0;
    rk_cdu = rk_cdu_c = (rk_cdu_state){{0}, 0, 0};
}

static unsigned timeline(void) {
    uint32_t n, block = RC_BASIS;
    unsigned failures = 0, k;
    agc_word now[3];
    clear();
    for (n = 0; n < RC_PERIODS; ++n) {
        agc_word ch14, write[3];
        uint8_t set[3];
        rc_input(n, &ch14, set, write);
        for (k = 0; k < 3u; ++k) if (set[k]) IMG[050u + k] = write[k];
        drive((uint8_t)(ch14 >> 8));
        for (k = 0; k < 3u; ++k) now[k] = IMG[050u + k];
        block = rc_fold(block, rc_digest(now));
        if (n % RC_BLOCK == RC_BLOCK - 1u) {
            if (block != rc_vectors[n / RC_BLOCK]) {
                if (++failures <= 5u)
                    printf("yaAGC timeline: periods %lu-%lu differ\n", (unsigned long)(n + 1u - RC_BLOCK), (unsigned long)n);
            }
            block = RC_BASIS;
        }
    }
    return failures;
}

static agc_word count(void) {
    static const agc_word edge[8] = {0, 077777, 1, 077776, 037777, 040000, 240, 077777 - 240};
    unsigned r = rnd();
    agc_word v;
    switch (r % 8u) {
    case 0: return edge[rnd() % 8u];
    case 1: v = (agc_word)(rnd() % 4000u); break;
    case 2: v = (agc_word)(rnd() % 300u); break;
    default: v = (agc_word)(rnd() % 50u); break;
    }
    return rnd() & 1u && v ? (agc_word)(v ^ 077777u) : v;
}
static unsigned against_c(uint32_t periods) {
    agc_hardware hw = {0};
    uint32_t n;
    unsigned k, failures = 0, idle = 0;
    uint8_t hi = 0;
    clear();
    for (n = 0; n < periods; ++n) {
        unsigned r = rnd();
        if (idle) { --idle; hi = (uint8_t)(hi & ~0x7cu); }
        else if (r % 2000u == 0u) idle = 2000u + rnd() % 17000u;   /* past the 0xfff0 stop */
        else if (r % 16u == 0u) hi = (uint8_t)rnd();
        if (r % 997u == 0u) rk_cdu.elapsed = rk_cdu_c.elapsed = (uint16_t)(0xffe0u + rnd() % 32u);
        for (k = 0; k < 5u; ++k)
            if (rnd() % 12u == 0u) IMG[050u + k] = ref.words[050u + k] = count();
        hw.channel14 = (uint16_t)((uint16_t)hi << 8 | (rnd() & 0xffu));
        rk_cdu_drive_c(&ref, &hw);
        drive(hi);
        for (k = 0; k < 5u; ++k)
            if (IMG[050u + k] != ref.words[050u + k] || rk_cdu.owed[k] != rk_cdu_c.owed[k]) break;
        if (k < 5u || rk_cdu.elapsed != rk_cdu_c.elapsed || rk_cdu.active != rk_cdu_c.active) {
            if (++failures <= 5u)
                printf("period %lu, ch14 %02x: counter %u %05o/%05o owed %d/%d, elapsed %u/%u, active %u/%u (.S/C)\n",
                       (unsigned long)n, hi, k, k < 5u ? IMG[050u + k] : 0, k < 5u ? ref.words[050u + k] : 0,
                       k < 5u ? rk_cdu.owed[k] : 0, k < 5u ? rk_cdu_c.owed[k] : 0,
                       rk_cdu.elapsed, rk_cdu_c.elapsed, rk_cdu.active, rk_cdu_c.active);
            for (k = 0; k < 5u; ++k) IMG[050u + k] = ref.words[050u + k];
            rk_cdu = rk_cdu_c;
        }
    }
    return failures;
}

#if defined(RK_CM)
static unsigned optics(void) {
    unsigned fail = 0, n;
    clear();
    IMG[053] = 300u;
    IMG[054] = (agc_word)(077777u - 5u);
    IMG[050] = 7u;
    drive(06000u >> 8);
    if (IMG[053] || IMG[054] || rk_cdu.owed[3] != 60 || rk_cdu.owed[4] || IMG[050] != 7u || !rk_cdu.active) ++fail;
    for (n = 0; n < 7u; ++n) drive(06000u >> 8);
    if (rk_cdu.owed[3] || rk_cdu.active) ++fail;
    IMG[053] = 9u;
    drive(0);
    if (IMG[053] != 9u) ++fail;
    return fail;
}
#endif

int main(void) {
    unsigned yaagc, c, opt = 0;
    if ((uintptr_t)&__heap_start > APOLLO_SHARED_ADDRESS) {
        printf("FAIL: the check reaches %p, past the image at %04x\n", (void *)&__heap_start, APOLLO_SHARED_ADDRESS);
        return 1;
    }
    {   /* clock()'s own cost, taken out of each period's */
        long t0 = clock();
        overhead = clock() - t0;
    }
    yaagc = timeline();
    c = against_c(UINT32_C(65536));
#if defined(RK_CM)
    opt = optics();
#endif
    printf("%s: resident-cdu-asm %s: yaAGC timeline %lu periods, %u blocks differ; against the C 65536 periods, %u differ; optics %u fail; cycles a period with the call: idle %ld, driving %ld (mean)\n",
           yaagc || c || opt ? "FAIL" : "PASS", MISSION, (unsigned long)RC_PERIODS, yaagc, c, opt,
           calls[0] ? cycles[0] / calls[0] : 0, calls[1] ? cycles[1] / calls[1] : 0);
    return yaagc || c || opt ? 1 : 0;
}
