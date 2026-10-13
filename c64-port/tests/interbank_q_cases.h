/* SPDX-License-Identifier: MIT */
#ifndef INTERBANK_Q_CASES_H
#define INTERBANK_Q_CASES_H
/* Cases for the INTER-BANK entries that take an unusual Q (milestone
 * interbank-q): SWCALL -1 (RTB's TC SWCALL -1, through SWCALL to the callee)
 * and BANKJUMP with Q below 0100 (SAVELOCS' TC L), in both missions. Case n:
 * n%4 = 0 LM SWCALL -1, 1 CM SWCALL -1, 2 LM BANKJUMP, 3 CM BANKJUMP. The
 * image is random erasable with coherent EB/FB/BB, A the callee's CADR (banks
 * 04-37), Q any return address (SWCALL -1) or 0-077 in half the BANKJUMP
 * cases. ibq_run takes the trampoline from the entry to the callee, which no
 * table maps (EXEC_UNMAPPED). */
#include "agc_transfer.h"
#define IBQ_CASES 1024u
#define IBQ_MAX_STEPS 4u
uint16_t ibq_entry(uint16_t n);
void ibq_input(apollo_overlay_state *s, uint16_t *superbank, uint16_t n);
/* 0 refused, 1 reached the callee (image left there). */
unsigned ibq_run(apollo_overlay_state *s, uint16_t *superbank, uint16_t n);
uint32_t ibq_digest(apollo_overlay_state *s, uint16_t n);
#endif
