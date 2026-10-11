/* SPDX-License-Identifier: MIT */
#ifndef NOUN_READER_CASES_H
#define NOUN_READER_CASES_H
#include "noun_reader.h"
#define NOUN_READER_LM_CASE_COUNT 30912u
#define NOUN_READER_CASE_COUNT 58368u
#define NOUN_READER_ROW_WORDS 2052u
enum apollo_fixed_mission noun_reader_mission(uint16_t);
enum apollo_noun_reader_entry noun_reader_entry(uint16_t);
void noun_reader_prepare(apollo_overlay_state *, apollo_noun_reader_io *, uint16_t);
#endif
