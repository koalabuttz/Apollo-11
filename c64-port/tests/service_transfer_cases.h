/* SPDX-License-Identifier: MIT */
#ifndef SERVICE_TRANSFER_CASES_H
#define SERVICE_TRANSFER_CASES_H
/* Cases for the SERVICE_ROUTINES transfer tables (milestone service-transfer).
 * Site cases: two per genuine `TC UPFLAG`, `TC DOWNFLAG` and `TC GENTRAN` of
 * each mission, entered at the callee with Q at the operand word as that TC
 * leaves it. Entry cases: UPENT2, DOWNENT2, DELAYJOB, TCGETCAD, WAKER, its
 * finish, B5OFF (and CM TRFAILOF/TRFAILON) from random images, with any Q
 * (half below 0100) for the entries that never read it. ibq-style trampoline
 * from the entry to the first unmapped continuation. */
#include "agc_transfer.h"
#define SVT_ENTRY_CASES 512u
#define SVT_MAX_STEPS 64u
uint16_t svt_cases(void);
/* Mission (0 LM, 1 CM) and logical Z of the entry. */
uint8_t svt_mission(uint16_t n);
void svt_input(apollo_overlay_state *s, uint16_t *superbank, uint16_t *inhint, uint16_t n);
/* 0 refused, 1 reached an unmapped continuation (image and *inhint left there). */
unsigned svt_run(apollo_overlay_state *s, uint16_t *superbank, uint16_t *inhint, uint16_t n);
uint32_t svt_digest(apollo_overlay_state *s, uint16_t n);
#endif
