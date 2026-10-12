/* SPDX-License-Identifier: GPL-2.0-or-later
 * Lane C oracle: runs the unchanged original program in Virtual AGC from a
 * case's entry until control leaves the code the native routine translates
 * (or reaches a label owned elsewhere), then prints the complete state.
 * Host-only test adapter, never part of the native port. It shares only the
 * case generator with the native side, never a native routine. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "lanec_case.h"
#include "lanec_suites.h"
void BacktraceAdd(agc_t *s, int cause) { (void)s; (void)cause; }
int ChannelInput(agc_t *s) { (void)s; return 0; }
void ChannelOutput(agc_t *s, int c, int v) { if (c == 7) s->InputChannel[7] = s->OutputChannel7 = v & 0160; }
void ChannelRoutine(agc_t *s) { (void)s; }
void RequestRadarData(agc_t *s) { (void)s; }
void ShiftToDeda(agc_t *s, int data) { (void)s; (void)data; }
void UnblockSocket(int n) { (void)n; }

static agc_t rope[2], m;
static apollo_overlay_state s;
static lanec_io io;
static lanec_meta meta;

static unsigned word(unsigned a) { return (unsigned)m.Erasable[a / 0400][a % 0400] & (a <= 2u ? 0177777u : 077777u); }

static int outside(unsigned z)
{
    unsigned fb = ((unsigned)m.Erasable[0][RegFB] >> 10) & 037u, bank, i;
    if (fb >= 030u && (m.OutputChannel7 & 0100)) fb += 010u;
    for (i = 0; i < meta.stops; ++i)
        if (meta.stop[i].address == z && (meta.stop[i].bank == LANEC_ANY_BANK || meta.stop[i].bank == fb)) return 1;
    for (i = 0; i < meta.ranges; ++i) {
        bank = meta.range[i].bank;
        if (z >= meta.range[i].lo && z <= meta.range[i].hi && (bank == LANEC_ANY_BANK || (z >= 02000u && z < 04000u && bank == fb)))
            return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    const lanec_suite *suite = argc == 4 ? lanec_find_suite(argv[1]) : 0;
    uint32_t n;
    unsigned i, steps;
    if (!suite || agc_load_binfile(&rope[0], argv[2]) || agc_load_binfile(&rope[1], argv[3])) {
        fprintf(stderr, "usage: oracle SUITE LM.bin CM.bin\n");
        return 2;
    }
    InhibitAlarms = 1;
    for (n = 0; n < suite->count; ++n) {
        memset(&meta, 0, sizeof meta);
        suite->make(n, &s, &io, &meta);
        m = rope[meta.mission];
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i) m.Erasable[i / 0400][i % 0400] = (int16_t)s.words[i];
        for (i = 0; i < LANEC_CHANNELS; ++i)
            if (i != 1u && i != 2u && i != 7u) m.InputChannel[i] = (int16_t)io.channel[i];
        m.OutputChannel7 = m.InputChannel[7] = (int16_t)io.superbank;
        m.AllowInterrupt = io.interrupt_enabled;
        for (steps = 0; steps < 4000u; ++steps) {
            if (steps && !m.PendFlag && !m.ExtraDelay && outside(word(RegZ))) break;
            m.ScalerCounter = 0;
            if (agc_engine(&m)) return 2;
        }
        if (steps == 4000u) { fprintf(stderr, "%s case %lu: no boundary, Z=%o\n", suite->name, (unsigned long)n, word(RegZ)); return 2; }
        printf("%lu", (unsigned long)n);
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i) printf(" %u", word(i));
        for (i = 0; i < LANEC_CHANNELS; ++i)
            printf(" %u", i == 1u || i == 2u || i == 7u ? 0u : (unsigned)m.InputChannel[i] & 077777u);
        printf(" %u %u 0\n", (unsigned)m.OutputChannel7 & 0160u, (unsigned)m.AllowInterrupt);
    }
    return 0;
}
