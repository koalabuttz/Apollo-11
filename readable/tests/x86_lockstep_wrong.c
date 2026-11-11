/* SPDX-License-Identifier: MIT
 * A wrong instruction, for the negative of tests/x86_lockstep.py (--wrong).
 *
 * That check flies the readable program against Virtual AGC and must fail
 * a program that computes a wrong word. This file is linked into the LM
 * program with the linker's --wrap=ts, which sends every call of ts()
 * from another file (src/agc/agc.c, the TS instruction) here. The first
 * TS BB after a thousand calls stores its word plus one, which changes
 * the erasable bank: one wrong word, once, in ten seconds of P00. The
 * check must report that routine and no other.
 *
 * Why BB: a wrong word the routine itself stores over, or a bit the
 * register does not have (FB plus one), is no difference at the routine's
 * end, and the check rightly passes it. */
#include <stdbool.h>

bool __real_ts(unsigned address);
bool __wrap_ts(unsigned address);
void incr(unsigned address);

enum { BB = 6 };

bool __wrap_ts(unsigned address)
{
    static unsigned long calls;
    static bool done;
    bool overflow = __real_ts(address);
    if (++calls > 1000ul && !done && address == BB) {
        done = true;
        incr(BB);
    }
    return overflow;
}
