/* SPDX-License-Identifier: MIT */
#ifndef RECOVERY_H
#define RECOVERY_H
#include "restart.h"
enum recovery_abort_entry { RECOVERY_FLAGS, RECOVERY_MR_KLEAN,
 RECOVERY_P00KLEAN, RECOVERY_V37KLEAN, RECOVERY_POODOO_PHASE4 };
typedef struct {
 restart_phases records;
 agc_word tbase[6],longbase[2];
 agc_word flagword2,flagword3,flagword10;
 agc_register a,l;
 agc_word q,bbank,superbank,interrupt_enabled;
} recovery_abort_state;
/* Bounded original abort cleanup: FLAGS / MR.KLEAN / P00KLEAN / V37KLEAN
 * through TC Q, or POODOO CAF OCT35 through GOPOODOO entry (before INHINT).
 * Groups 1..6 only. Normal 15-bit stored words, 16-bit A/L registers,
 * stable tables and caller exclusion; Q=0400..07777 for TC Q returns.
 * BBANK must select source bank 13 for FLAGS, bank 5 for group cleanup. Bases, priorities, addresses retained.
 * No alarm recording, live SERVICER/display, or restart execution implied.
 * Returns 0 on success; unsupported entry returns 1 with no mutation. */
uint8_t recovery_abort(recovery_abort_state *,enum recovery_abort_entry);
#endif
