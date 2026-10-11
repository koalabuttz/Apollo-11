/* SPDX-License-Identifier: MIT */
#include "noun_reader_cases.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static apollo_overlay_state state, before;
static void reject(enum apollo_fixed_mission mission,
                   enum apollo_noun_reader_entry entry,
                   apollo_noun_reader_io *io)
{
    apollo_noun_reader_io saved = *io;
    before = state;
    assert(!apollo_noun_reader(&state, io, mission, entry));
    assert(memcmp(&state, &before, sizeof state) == 0);
    assert(memcmp(io, &saved, sizeof saved) == 0);
}

int main(void)
{
    apollo_noun_reader_io io;
    unsigned mission, entry, fault;
    uint16_t n;
    for (n = 0; n < NOUN_READER_CASE_COUNT; ++n) {
        noun_reader_prepare(&state, &io, n);
        assert(apollo_noun_reader(&state, &io, noun_reader_mission(n), noun_reader_entry(n)));
    }
    for (mission = 0; mission < 2; ++mission) {
        for (entry = 0; entry < 3; ++entry) {
            uint16_t base = (uint16_t)((mission ? NOUN_READER_LM_CASE_COUNT : 0u) +
                (entry == 0 ? 0u : entry == 1 ? 101u * 192u :
                    (101u + (mission ? 21u : 30u)) * 192u));
            for (fault = 0; fault < 16; ++fault) {
                noun_reader_prepare(&state, &io, base);
                switch (fault) {
                case 0: state.words[5]++; break;
                case 1: state.words[4] = 062000; break;
                case 2: state.words[3] |= 1; break;
                case 3: state.words[6] ^= 1; break;
                case 4: state.words[7] = 077777; break;
                case 5: io.superbank = 0; break;
                case 6: io.interrupt_enabled = 2; break;
                case 7: state.words[0] = 010000; break;
                case 8: state.words[0] = 017777; break;
                case 9: state.words[0] = 0177777; break;
                default:
                    if (entry == 0) {
                        static const uint16_t bad[] = {100, 101, 077776, 0100000, 0177777, 0177};
                        state.words[01002] = bad[fault - 10u];
                    } else {
                        static const uint16_t bad[] = {1, 3, 077776, 0100000, 0177777, 0100};
                        state.words[0123] = bad[fault - 10u];
                    }
                }
                reject((enum apollo_fixed_mission)mission,
                       (enum apollo_noun_reader_entry)entry, &io);
            }
        }
    }
    noun_reader_prepare(&state, &io, 0);
    reject((enum apollo_fixed_mission)-1, APOLLO_LODNNTAB, &io);
    reject((enum apollo_fixed_mission)2, APOLLO_LODNNTAB, &io);
    reject(APOLLO_FIXED_LM, (enum apollo_noun_reader_entry)-1, &io);
    reject(APOLLO_FIXED_LM, (enum apollo_noun_reader_entry)3, &io);
    before = state;
    assert(!apollo_noun_reader(&state, NULL, APOLLO_FIXED_LM, APOLLO_LODNNTAB));
    assert(memcmp(&state, &before, sizeof state) == 0);
    assert(!apollo_noun_reader(NULL, &io, APOLLO_FIXED_LM, APOLLO_LODNNTAB));
    puts("PASS: 58368 supported noun-reader contexts; 100 unsupported contexts preserve every word and I/O");
    return 0;
}
