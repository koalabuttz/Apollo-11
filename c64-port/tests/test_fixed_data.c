/* SPDX-License-Identifier: MIT */
#include "fixed_data.h"
#include <assert.h>
#include <stdio.h>

static void mission(enum apollo_fixed_mission which, uint16_t type60,
                    uint16_t scale_words, uint16_t expected_words)
{
    apollo_fixed_table table;
    uint16_t word = 0xaaaa;
    unsigned index;
    uint32_t address;
    uint16_t covered = 0;
    assert(apollo_noun_table_read(which, APOLLO_NNADTAB, 60, &word));
    assert(word == 024074);
    assert(apollo_noun_table_read(which, APOLLO_NNTYPTAB, 60, &word));
    assert(word == type60);
    assert(apollo_noun_table_info(which, APOLLO_SFINTAB, &table));
    assert(table.word_count == scale_words);
    for (index = 0; index < APOLLO_NOUN_TABLE_COUNT; ++index) {
        assert(apollo_noun_table_info(which, (enum apollo_noun_table)index, &table));
        assert(apollo_noun_table_read(which, (enum apollo_noun_table)index,
                                     (uint16_t)(table.word_count - 1), &word));
        word = 0xa55a;
        assert(!apollo_noun_table_read(which, (enum apollo_noun_table)index,
                                      table.word_count, &word));
        assert(word == 0xa55a);
    }
    for (address = 0; address < 65536UL; ++address) {
        word = 0xa55a;
        if (apollo_fixed_data_read(which, (uint16_t)address, &word)) {
            assert(word <= 077777);
            ++covered;
        } else {
            assert(word == 0xa55a);
        }
    }
    assert(covered == expected_words);
}

int main(void)
{
    uint16_t word = 0xa55a;
    apollo_fixed_table table = {123, 456};
#if !defined(APOLLO_FIXED_CM_ONLY)
    mission(APOLLO_FIXED_LM, 060512, 58, 1524);
#endif
#if !defined(APOLLO_FIXED_LM_ONLY)
    mission(APOLLO_FIXED_CM, 010440, 40, 1347);
#endif
    assert(!apollo_fixed_data_read((enum apollo_fixed_mission)-1, 0, &word));
    assert(!apollo_fixed_data_read(APOLLO_FIXED_LM, 65535, &word));
    assert(word == 0xa55a);
    assert(!apollo_fixed_data_read(APOLLO_FIXED_LM, 0, NULL));
    assert(!apollo_noun_table_read(APOLLO_FIXED_LM, APOLLO_NNADTAB, 0, NULL));
    assert(!apollo_noun_table_info(APOLLO_FIXED_LM, (enum apollo_noun_table)-1, &table));
    assert(!apollo_noun_table_info(APOLLO_FIXED_LM, APOLLO_NOUN_TABLE_COUNT, &table));
    assert(table.physical_fixed_word == 123 && table.word_count == 456);
    assert(!apollo_noun_table_info(APOLLO_FIXED_LM, APOLLO_NNADTAB, NULL));
#if defined(APOLLO_FIXED_LM_ONLY)
    assert(!apollo_noun_table_read(APOLLO_FIXED_CM, APOLLO_NNADTAB, 0, &word));
#elif defined(APOLLO_FIXED_CM_ONLY)
    assert(!apollo_noun_table_read(APOLLO_FIXED_LM, APOLLO_NNADTAB, 0, &word));
#endif
    puts("PASS: independent noun60 source values, both missions, table bounds and unchanged failure outputs");
    return 0;
}
