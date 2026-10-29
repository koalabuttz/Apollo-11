/* SPDX-License-Identifier: MIT */
#include "noun_reader_cases.h"
#include <stdio.h>
#include <string.h>
static void emit(uint16_t word)
{
    putchar((int)(word & 255u)); putchar((int)(word >> 8));
}
int main(int argc, char **argv)
{
    static apollo_overlay_state state;
    apollo_noun_reader_io io;
    uint16_t n, i;
    bool execute = argc == 1;
    if (!execute && (argc != 2 || strcmp(argv[1], "--input") != 0)) return 2;
    for (n = 0; n < NOUN_READER_CASE_COUNT; ++n) {
        noun_reader_prepare(&state, &io, n);
        emit(n);
        emit(!execute || apollo_noun_reader(&state, &io, noun_reader_mission(n),
                                          noun_reader_entry(n)) ? 1u : 0u);
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i) emit(state.words[i]);
        emit(io.superbank); emit(io.interrupt_enabled);
    }
    return ferror(stdout) ? 2 : 0;
}
