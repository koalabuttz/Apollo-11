/* SPDX-License-Identifier: MIT */
#include "noun_reader_cases.h"
#include "noun_reader_vectors.h"

static apollo_overlay_state state, before;

int main(void)
{
    volatile unsigned char *mailbox = (volatile unsigned char *)0xc000;
    apollo_noun_reader_io io, saved;
    uint16_t row, i, changed, expected, n, form, checked = 0;
    mailbox[0] = 0xff;
    for (row = 0; row < NOUN_READER_TARGET_CASES; ++row) {
      for (form = 0; form < 4u; ++form) {
        n = noun_reader_target_ids[row];
        noun_reader_prepare(&state, &io, n);
        io.superbank = (uint16_t)(0100u + form * 020u);
        before = state;
        saved = io;
        if (!apollo_noun_reader(&state, &io, noun_reader_mission(n), noun_reader_entry(n)))
            goto failed;
        changed = 0;
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i) {
            expected = before.words[i];
            if (changed < NOUN_READER_CHANGED_WORDS && noun_reader_changed[changed] == i)
                expected = noun_reader_expected[row][changed++];
            if (state.words[i] != expected) goto failed;
        }
        if (io.superbank != saved.superbank || io.interrupt_enabled != saved.interrupt_enabled)
            goto failed;
        ++checked;
      }
    }
    mailbox[1] = (unsigned char)checked;
    mailbox[2] = (unsigned char)(checked >> 8);
    mailbox[0] = 0xa5;
    for (;;) {}
failed:
    mailbox[1] = (unsigned char)checked;
    mailbox[2] = (unsigned char)(checked >> 8);
    mailbox[0] = 0xee;
    for (;;) {}
}
