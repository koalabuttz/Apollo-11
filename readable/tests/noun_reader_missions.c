/* SPDX-License-Identifier: MIT */
/* Both missions' readable noun table readers in one test program: each
 * mission's objects are built into one object with apollo_noun_reader
 * renamed readable_<m>_noun_reader (the Makefile), and this gives the
 * faithful port's name back, choosing by the mission as the faithful
 * reader does. */
#include "noun_reader.h"

bool readable_lm_noun_reader(apollo_overlay_state *, apollo_noun_reader_io *,
                             enum apollo_fixed_mission, enum apollo_noun_reader_entry);
bool readable_cm_noun_reader(apollo_overlay_state *, apollo_noun_reader_io *,
                             enum apollo_fixed_mission, enum apollo_noun_reader_entry);

bool apollo_noun_reader(apollo_overlay_state *state, apollo_noun_reader_io *io,
                        enum apollo_fixed_mission mission, enum apollo_noun_reader_entry entry)
{
    if (mission == APOLLO_FIXED_LM) return readable_lm_noun_reader(state, io, mission, entry);
    if (mission == APOLLO_FIXED_CM) return readable_cm_noun_reader(state, io, mission, entry);
    return false;
}
