/* SPDX-License-Identifier: MIT
 * Wrong instructions, for the negatives of tests/x86_lockstep.py (--wrong).
 *
 * That check flies the readable program against Virtual AGC and must fail
 * a program that is wrong. This file is linked into the LM program with
 * the linker's --wrap, which sends every call of one instruction's
 * function from another file (src/agc/agc.c) here. Built three ways, each
 * wrong once, in one place the check compares:
 *
 *   WRONG_TS       an erasable word. --wrap=ts: the first TS BB after a
 *                  thousand TSs stores its word plus one, which changes
 *                  the erasable bank.
 *   WRONG_CHANNEL  a channel word. --wrap=wor_channel: the first WOR
 *                  CHAN13 leaves bit 5 of the channel changed (a bit the
 *                  program does not use), and A as it should be.
 *   WRONG_TRAP     a HANDRUPT trap. The same WOR leaves bit 13 of the
 *                  channel set, which arms trap 31B. The check compares
 *                  channel 13 without its bits 12 to 14, so only its
 *                  comparison of the traps can see this one.
 *
 * Each time the check must report that routine and no other, and report
 * it for the word that is wrong.
 *
 * Why BB for the first: a wrong word the routine itself stores over, or
 * a bit the register does not have (FB plus one), is no difference at the
 * routine's end, and the check rightly passes it. */
#include <stdbool.h>
#include "../src/agc/agc.h"

#if defined(WRONG_TS)
bool __real_ts(unsigned address);
bool __wrap_ts(unsigned address);
void incr(unsigned address);

bool __wrap_ts(unsigned address)
{
    static unsigned long calls;
    static bool done;
    bool overflow = __real_ts(address);
    if (++calls > 1000ul && !done && address == BBANK_ADDRESS) {
        done = true;
        incr(BBANK_ADDRESS);
    }
    return overflow;
}
#else
#if defined(WRONG_CHANNEL)
#define WRONG_BIT 000020u /* bit 5 */
#elif defined(WRONG_TRAP)
#define WRONG_BIT 010000u /* bit 13 */
#endif
enum { CHAN13 = 013 };

void __real_wor_channel(unsigned channel);
void __wrap_wor_channel(unsigned channel);

void __wrap_wor_channel(unsigned channel)
{
    static bool done;

    __real_wor_channel(channel);
    if (!done && channel == CHAN13) {
        done = true;
        agc.channel_out(CHAN13, (word)(agc.channel_in(CHAN13) ^ WRONG_BIT));
    }
}
#endif
