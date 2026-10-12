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

/* LANEC_TRACE=FILE: also write every fixed-memory instruction address the
 * original program started executing inside the suite's ranges, as
 * "MISSION bank,address" (bank 0 for fixed-fixed), one per line, deduplicated. */
static unsigned char traced[2][044][02000];

static unsigned word(unsigned a) { return (unsigned)m.Erasable[a / 0400][a % 0400] & (a <= 2u ? 0177777u : 077777u); }

static int outside(unsigned z)
{
    unsigned fb = ((unsigned)m.Erasable[0][RegFB] >> 10) & 037u, bank, i;
    if (fb >= 030u && (m.OutputChannel7 & 0100)) fb += 010u;
    if (meta.newops && z == meta.newops) {
        unsigned loc = word(0164u), inside = 0;
        for (i = 0; i < meta.locs; ++i)
            if (meta.loc[i].bank == fb && loc >= meta.loc[i].lo && loc <= meta.loc[i].hi) inside = 1;
        if (!inside) return 1;
    }
    for (i = 0; i < meta.stops; ++i)
        if (meta.stop[i].address == z && (meta.stop[i].bank == LANEC_ANY_BANK || meta.stop[i].bank == fb)) return 1;
    for (i = 0; i < meta.ranges; ++i) {
        bank = meta.range[i].bank;
        if (z >= meta.range[i].lo && z <= meta.range[i].hi && (bank == LANEC_ANY_BANK || (z >= 02000u && z < 04000u && bank == fb)))
            return 0;
    }
    return 1;
}

/* Virtual AGC drives the coarse-align CDUs from channel 14 bits 15-13
 * between instructions, emptying CDUXCMD..CDUZCMD on a schedule kept in
 * engine statics that carry over from case to case. Like the timers, that
 * drive is hardware the port leaves outside its scope, so the bits are kept
 * out of the engine's view except during an instruction that does channel 14
 * I/O itself. */
static unsigned fetch(unsigned z)
{
    unsigned fb = ((unsigned)m.Erasable[0][RegFB] >> 10) & 037u;
    if (fb >= 030u && (m.OutputChannel7 & 0100)) fb += 010u;
    if (z >= 04000u) return (unsigned)m.Fixed[2u + (z - 04000u) / 02000u][(z - 04000u) % 02000u] & 077777u;
    if (z >= 02000u) return (unsigned)m.Fixed[fb][z - 02000u] & 077777u;
    return word(z);
}

static int channel14_io(void)
{
    unsigned k = (fetch(word(RegZ) & 07777u) + (unsigned)m.IndexValue) & 077777u;    /* INDEX applies first */
    return m.ExtraCode && (k & 070000u) == 0 && (k & 0777u) == 014u;
}

static void trace(unsigned z)
{
    unsigned fb = ((unsigned)m.Erasable[0][RegFB] >> 10) & 037u;
    if (fb >= 030u && (m.OutputChannel7 & 0100)) fb += 010u;
    unsigned char (*t)[02000] = traced[meta.mission == LANEC_CM];
    if (z >= 04000u) t[0][(z - 04000u) % 02000u] |= (unsigned char)(1u << ((z - 04000u) / 02000u));
    else if (z >= 02000u && fb >= 1u && fb < 044u) t[fb][z - 02000u] |= 1u;
}

static void write_trace(const char *path)
{
    FILE *f = fopen(path, "w");
    unsigned b, a, k, n;
    if (!f) return;
    for (n = 0; n < 2u; ++n) {
        for (k = 0; k < 2u; ++k)
            for (a = 0; a < 02000u; ++a)
                if (traced[n][0][a] & (1u << k)) fprintf(f, "%s 0,%04o\n", n ? "CM" : "LM", 04000u + k * 02000u + a);
        for (b = 1; b < 044u; ++b)
            for (a = 0; a < 02000u; ++a)
                if (traced[n][b][a]) fprintf(f, "%s %o,%04o\n", n ? "CM" : "LM", b, 02000u + a);
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    const lanec_suite *suite = argc == 4 ? lanec_find_suite(argv[1]) : 0;
    uint32_t n;
    unsigned i, steps, drive;
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
        for (steps = 0; steps < 40000u; ++steps) {
            if (steps && !m.PendFlag && !m.ExtraDelay && outside(word(RegZ))) break;
            if (!m.PendFlag && !m.ExtraDelay) trace(word(RegZ));
            m.ScalerCounter = 0;
            drive = channel14_io() ? 0 : (unsigned)m.InputChannel[014] & 070000u;
            m.InputChannel[014] &= (int16_t)~drive;
            if (agc_engine(&m)) return 2;
            m.InputChannel[014] |= (int16_t)drive;
        }
        if (steps == 40000u) { fprintf(stderr, "%s case %lu: no boundary, Z=%o\n", suite->name, (unsigned long)n, word(RegZ)); return 2; }
        printf("%lu", (unsigned long)n);
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i) printf(" %u", word(i));
        for (i = 0; i < LANEC_CHANNELS; ++i)
            printf(" %u", i == 1u || i == 2u || i == 7u ? 0u : (unsigned)m.InputChannel[i] & 077777u);
        printf(" %u %u 0\n", (unsigned)m.OutputChannel7 & 0160u, (unsigned)m.AllowInterrupt);
    }
    if (getenv("LANEC_TRACE")) write_trace(getenv("LANEC_TRACE"));
    return 0;
}
