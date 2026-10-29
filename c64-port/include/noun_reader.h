/* SPDX-License-Identifier: MIT */
#ifndef APOLLO_NOUN_READER_H
#define APOLLO_NOUN_READER_H
#include "fixed_data.h"
#include "overlay_runtime.h"
enum apollo_noun_reader_entry {
    APOLLO_LODNNTAB, APOLLO_GTSFOUT, APOLLO_GTSFIN
};
typedef struct apollo_noun_reader_io {
    uint16_t superbank, interrupt_enabled;
} apollo_noun_reader_io;
/* Original three bank42 table readers, returning through the source DXCH Z
 * convention. Image alone owns all RAM; producers are excluded. Start Z is
 * the selected original entry, FB=064000, matching BB/retained EB0..7 and
 * SUPER100/120/140/160. A/L carry a logical caller PC0000..7777 and bank pair; first
 * exchange applies original register overflow correction. NOUNREG holds
 * noun0..99 (either zero), or SFTEMP1 holds an even pair index within that
 * mission's scale table (either zero). All ordinary RAM words are15bit.
 * Both signs of zero and all documented table entries are supported.
 * Unsupported contexts leave image/IO entirely unchanged. */
bool apollo_noun_reader(apollo_overlay_state *, apollo_noun_reader_io *,
                       enum apollo_fixed_mission,
                       enum apollo_noun_reader_entry);
#endif
