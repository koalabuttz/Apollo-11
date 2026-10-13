/* SPDX-License-Identifier: MIT */
#ifndef INTERBANK_ORACLE_CASES_H
#define INTERBANK_ORACLE_CASES_H
/* Cases for milestone interbank-oracle: every registered entry of both
 * missions' INTER-BANK_COMMUNICATION through agc_transfer_run, against yaAGC.
 * Cases 0 .. IBO_SITES-1: every genuine fixed site whose callee reads an
 * inline CADR (BANKCALL, IBNKCALL, POSTJUMP, USPRCADR; LM sites first), entered
 * at the callee with Q at the operand word as the TC leaves it. Then
 * IBO_PER_ENTRY cases per entry and mission (n%2 mission) from a random image:
 * the inline-CADR entries with an erasable operand word, SUPDACAL with its
 * data word one of IBO_DATA rope words (tests/interbank_oracle_data.h, frozen
 * from the ropes), the rest with any ordinary Q. To the first continuation
 * outside the file. */
#include "agc_transfer.h"
#define IBO_ENTRIES 14u
#define IBO_PER_ENTRY 48u
#define IBO_DATA 32u
#define IBO_MAX_STEPS 4u
#define IBO_CASES ibo_cases()
uint16_t ibo_cases(void);
uint8_t ibo_mission(uint16_t n);
/* Physical fixed word of SUPDACAL data slot i of mission cm. */
uint16_t ibo_data_physical(uint8_t cm, uint16_t i);
void ibo_input(apollo_overlay_state *s, uint16_t *superbank, uint16_t *inhint, uint16_t n);
/* 0 refused, 1 reached a continuation outside the file. */
unsigned ibo_run(apollo_overlay_state *s, uint16_t *superbank, uint16_t *inhint, uint16_t n);
uint32_t ibo_digest(apollo_overlay_state *s, uint16_t n);
#endif
