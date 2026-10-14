/* SPDX-License-Identifier: GPL-2.0-or-later; host-only oracle instruction tracer.
 * Linked into a yaAGC reference with -Wl,--wrap=agc_engine (tools/oracle_trace/cc
 * does this). Before each instruction it records the physical fixed address
 * of Z (bank*1024 + offset, banks 030-037 +8 under SUPERBNK bit 7, fixed-fixed
 * 04000-07777 as banks 2-3), and at exit appends the addresses run to the
 * file named by AGC_EXECUTED_WORDS, one decimal address per line.
 * With AGC_READ_WORDS set it also records the fixed words that CA, CS, AD,
 * MASK, DCA, DCS and MP read (after INDEX), which is how the interpreter
 * fetches interpretive code, and appends them to that file the same way. */
#include <stdio.h>
#include <stdlib.h>
#include "agc_engine.h"
int __real_agc_engine(agc_t *);
static unsigned char seen[44u * 1024u], read[44u * 1024u];
static int armed;
static void dump_set(const char *path, const unsigned char *set)
{
    FILE *f;
    unsigned i;
    if (!path || !(f = fopen(path, "a")))
        return;
    for (i = 0; i < sizeof seen; ++i)
        if (set[i])
            fprintf(f, "%u\n", i);
    fclose(f);
}
static void dump(void)
{
    dump_set(getenv("AGC_EXECUTED_WORDS"), seen);
    dump_set(getenv("AGC_READ_WORDS"), read);
}
/* Physical address of fixed K (02000-07777) under the current banks. */
static unsigned fixed_phys(const agc_t *m, unsigned k)
{
    unsigned bank;
    if (k >= 04000u)
        return (k >> 10) * 1024u + (k & 01777u);
    bank = ((unsigned)m->Erasable[0][RegFB] & 076000u) >> 10;
    if (bank >= 030u && (m->OutputChannel7 & 0100))
        bank += 8u;
    return bank * 1024u + (k & 01777u);
}
/* The fixed words the instruction at physical p reads, as yaAGC forms it
 * (the word plus the pending INDEX value). */
static void note_reads(const agc_t *m, unsigned p)
{
    int ins = m->Fixed[p / 1024u][p % 1024u] & 077777, ix = m->IndexValue & 0177777;
    unsigned op, k;
    if (ix & 0100000)
        ix = -(~ix & 077777);
    ins += ix;
    if (ins > 077777)
        ins = (ins & 077777) + 1;
    if (ins < 0)
        ins += 077777;
    op = (unsigned)ins >> 12;
    k = (unsigned)ins & 07777u;
    if (k < 02000u)
        return;
    if (m->ExtraCode ? op == 3u || op == 4u : 0) {
        read[fixed_phys(m, k - 1u)] = 1;
        read[fixed_phys(m, k)] = 1;
    } else if (m->ExtraCode ? op == 7u : op == 3u || op == 4u || op == 6u || op == 7u) {
        read[fixed_phys(m, k)] = 1;
    }
}
int __wrap_agc_engine(agc_t *m)
{
    if (!armed) {
        armed = 1;
        atexit(dump);
    }
    if (!m->PendFlag && !m->ExtraDelay) {
        unsigned z = (unsigned)m->Erasable[0][RegZ] & 07777u, bank;
        if (z >= 02000u && z < 04000u) {
            bank = ((unsigned)m->Erasable[0][RegFB] & 076000u) >> 10;
            if (bank >= 030u && (m->OutputChannel7 & 0100))
                bank += 8u;
            seen[bank * 1024u + (z & 01777u)] = 1;
            note_reads(m, bank * 1024u + (z & 01777u));
        } else if (z >= 04000u) {
            seen[(z >> 10) * 1024u + (z & 01777u)] = 1;
            note_reads(m, (z >> 10) * 1024u + (z & 01777u));
        }
    }
    return __real_agc_engine(m);
}
