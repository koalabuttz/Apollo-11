/* SPDX-License-Identifier: MIT */
/* The words of fixed memory after a caller's TC, for the bindings whose
 * routines read them (binding.h): from the program's operand set, or,
 * where the set names the address alone, from the rope image. */
#include <stddef.h>
#include "binding.h"

bool fixed_word_at(const agc_native_program *program, unsigned physical, agc_word *value)
{
    if (!program || physical > 0xFFFFu ||
        !agc_site_lookup(program->operands, program->operand_count, (uint16_t)physical, value))
        return false;
    if (*value != WORD_IN_ROPE) return true;
    return program->fixed_data && program->fixed_data(program, (uint16_t)physical, value);
}

bool inline_word(const agc_native_program *program, const apollo_overlay_state *state,
                 unsigned superbank, word q, agc_word *out)
{
    if (q < 02000u || q > 07777u)
        return agc_transfer_operand(program, state, (agc_word)superbank, q, out);
    return fixed_word_at(program, agc_fixed_physical(q, state->words[4], (agc_word)superbank), out);
}
