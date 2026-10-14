/* SPDX-License-Identifier: MIT */
/* Interpretive order trace of a yaAGC run (tools/interp_order_rates.py):
 * runs the rope from a fresh start for CS centiseconds with DSKY keys (as
 * tools/flight_host_run.c's reference side: key k at KEY_AT + k * KEY_GAP
 * cs, channel 15 and a KEYRUPT1 request) and prints "cs table offset" each
 * time the interpreter executes a word of its dispatch tables (INDJUMP,
 * MISCJUMP, UNAJUMP; physical addresses on argv), which is once per
 * address, miscellaneous or unary order.
 *
 *   interp-order-trace ROPE.bin CS KEYS PHYS:NAME:COUNT ...
 * FLIGHT_CH30 / FLIGHT_CH31 (octal) set the input channels at start. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "resident_rupt.h"

void BacktraceAdd(agc_t *s, int cause) { (void)s; (void)cause; }
int ChannelInput(agc_t *s) { (void)s; return 0; }
void ChannelOutput(agc_t *s, int c, int v) { if (c == 7) s->InputChannel[7] = s->OutputChannel7 = v & 0160; }
void ChannelRoutine(agc_t *s) { (void)s; }
void RequestRadarData(agc_t *s) { (void)s; }
void ShiftToDeda(agc_t *s, int data) { (void)s; (void)data; }
void UnblockSocket(int n) { (void)n; }

#define KEY_AT 200ul
#define KEY_GAP 50ul
static agc_t m;
static unsigned keycode(char c)
{
    return c == 'V' ? 021u : c == 'N' ? 037u : c == 'E' ? 034u : c == '0' ? 020u : c == '+' ? 032u : c == '-' ? 033u :
           c == 'C' ? 036u : c == 'P' ? 022u : c == 'K' ? 031u : c == 'R' ? 022u : (unsigned)(c - '0');
}
static unsigned word(unsigned a) { return (unsigned)m.Erasable[a / 0400u][a % 0400u] & 077777u; }
static unsigned long now(void) { return ((unsigned long)(word(024) & 037777u) << 14) | (word(025) & 037777u); }
/* Physical fixed word of Z under FB and the superbank (bank*1024+offset). */
static long physical(void)
{
    unsigned z = (unsigned)m.Erasable[0][5] & 07777u, fb = word(4) >> 10, sb = (unsigned)m.OutputChannel7 & 0160u;
    if (z >= 04000u) return 2048L + (long)(z - 04000u);
    if (z < 02000u) return -1;
    if (fb >= 030u && sb >= 0100u) fb += (sb >> 4) * 010u - 030u;
    return (long)fb * 1024L + (long)(z - 02000u);
}
static int quiet(void) { return !m.PendFlag && !m.ExtraDelay && !m.IndexValue && !m.ExtraCode && !m.SubstituteInstruction; }

int main(int argc, char **argv)
{
    static struct { long phys; unsigned count; const char *name; } t[8];
    unsigned long cs, steps;
    const char *keys;
    unsigned nt = 0, pressed = 0, i;
    if (argc < 4 || agc_load_binfile(&m, argv[1])) {
        fprintf(stderr, "usage: interp-order-trace ROPE.bin CS KEYS PHYS:NAME:COUNT ...\n");
        return 2;
    }
    cs = strtoul(argv[2], 0, 0);
    keys = argv[3];
    for (i = 4; i < (unsigned)argc && nt < 8u; ++i) {
        char *p = strchr(argv[i], ':'), *q = p ? strchr(p + 1, ':') : 0;
        if (!q) return 2;
        *p = *q = 0;
        t[nt].phys = strtol(argv[i], 0, 0);
        t[nt].name = p + 1;
        t[nt++].count = (unsigned)strtoul(q + 1, 0, 0);
    }
    /* yaAGC's GOJAM state without its first DOWNRUPT (flight_host_run's cold()). */
    memset(m.Erasable, 0, sizeof m.Erasable);
    for (i = 0; i < NUM_CHANNELS; ++i) m.InputChannel[i] = 0;
    m.InputChannel[030] = (int16_t)(getenv("FLIGHT_CH30") ? strtoul(getenv("FLIGHT_CH30"), 0, 8) & 077777u : 037777u);
    m.InputChannel[031] = (int16_t)(getenv("FLIGHT_CH31") ? strtoul(getenv("FLIGHT_CH31"), 0, 8) & 077777u : 077777u);
    m.InputChannel[032] = m.InputChannel[033] = 077777;
    m.Erasable[0][5] = 04000;
    m.AllowInterrupt = 1;
    InhibitAlarms = 1;
    for (steps = 0; steps < 4000000000ul; ++steps) {
        unsigned long tn = now();
        if (tn > cs) break;
        if (keys[pressed] && tn >= KEY_AT + pressed * KEY_GAP && quiet() && !m.InIsr) {
            m.InputChannel[015] = (int16_t)keycode(keys[pressed++]);
            m.InterruptRequests[RK_RUPT_KEY1] = 1;
        }
        if (quiet()) {
            long p = physical();
            for (i = 0; i < nt; ++i)
                if (p >= t[i].phys && p < t[i].phys + (long)t[i].count) printf("%lu %s %ld\n", tn, t[i].name, p - t[i].phys);
        }
        if (agc_engine(&m)) return 2;
    }
    return 0;
}
