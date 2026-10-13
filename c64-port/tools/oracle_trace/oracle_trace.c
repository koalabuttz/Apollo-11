/* SPDX-License-Identifier: GPL-2.0-or-later; host-only oracle instruction tracer.
 * Linked into a yaAGC reference with -Wl,--wrap=agc_engine (tools/oracle_trace/cc
 * does this). Before each instruction it records the physical fixed address
 * of Z (bank*1024 + offset, banks 030-037 +8 under SUPERBNK bit 7, fixed-fixed
 * 04000-07777 as banks 2-3), and at exit appends the addresses run to the
 * file named by AGC_EXECUTED_WORDS, one decimal address per line. */
#include <stdio.h>
#include <stdlib.h>
#include "agc_engine.h"
int __real_agc_engine(agc_t *);
static unsigned char seen[44u * 1024u];
static int armed;
static void dump(void)
{
    const char *path = getenv("AGC_EXECUTED_WORDS");
    FILE *f;
    unsigned i;
    if (!path || !(f = fopen(path, "a")))
        return;
    for (i = 0; i < sizeof seen; ++i)
        if (seen[i])
            fprintf(f, "%u\n", i);
    fclose(f);
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
        } else if (z >= 04000u) {
            seen[(z >> 10) * 1024u + (z & 01777u)] = 1;
        }
    }
    return __real_agc_engine(m);
}
