/* SPDX-License-Identifier: GPL-2.0-or-later
 * Independent host-only original-source oracle. Input-only constructors are
 * linked; fixed_data.c, noun_reader.c and all native numerical bodies are not. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "noun_reader_cases.h"
static void emit_original(unsigned word)
{
    putchar((int)(word & 255u)); putchar((int)((word >> 8) & 255u));
}
int main(int argc, char **argv)
{
    static agc_t pristine;
    static apollo_overlay_state state;
    apollo_noun_reader_io io;
    unsigned n, i, cycles, caller_pc, caller_bank;
    if (argc != 2 || agc_load_binfile(&pristine, argv[1])) return 2;
    for (n = 0; n < NOUN_READER_CASE_COUNT; ++n) {
        if ((unsigned)noun_reader_mission((uint16_t)n) !=
            (strstr(argv[1], "Comanche055") != NULL ? 1u : 0u)) continue;
        memcpy(&machine, &pristine, sizeof machine);
        noun_reader_prepare(&state, &io, (uint16_t)n);
        InhibitAlarms = 1;
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i)
            *memory(i) = (int16_t)state.words[i];
        caller_pc = (unsigned)OverflowCorrected(state.words[0]);
        caller_bank = (unsigned)OverflowCorrected(state.words[1]) & 076007u;
        machine.OutputChannel7 = machine.InputChannel[7] = io.superbank;
        machine.AllowInterrupt = io.interrupt_enabled;
        for (cycles = 0; cycles < 1000; ++cycles) {
            if (!machine.PendFlag && !machine.ExtraDelay && get(RegZ) == caller_pc &&
                get(RegBB) == caller_bank) break;
            machine.ScalerCounter = 0;
            if (agc_engine(&machine)) return 2;
        }
        if (cycles == 1000) {
            fprintf(stderr, "noun reader %u timeout PC%05o FB%05o\n", n, get(RegZ), get(RegFB));
            return 2;
        }
        emit_original(n); emit_original(1);
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i)
            emit_original((unsigned)*memory(i) & (i <= 2 ? 0177777u : 077777u));
        emit_original((unsigned)machine.OutputChannel7);
        emit_original((unsigned)machine.AllowInterrupt);
    }
    return ferror(stdout) ? 2 : 0;
}
