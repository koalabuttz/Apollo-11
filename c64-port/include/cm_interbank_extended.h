/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_EXTENDED_H
#define CM_INTERBANK_EXTENDED_H
#include "cm_interbank.h"
/* Original interbank groups including the TS overflow branches. Q remains a
 * valid ordinary compiled caller/return address; saved return words are TC
 * descriptors. All raw16 A/L inputs are accepted. No instruction decoder.
 * SUPDACAL data is fetched beforehand through passive original-word storage;
 * caller holds producer exclusion across the returned C frames/slot loads. */
enum exec_status cm_interbank_extended_address(const apollo_overlay_state *,const cm_native_io *,uint16_t *);
enum exec_status cm_interbank_extended(apollo_overlay_state *,cm_native_io *,enum cm_interbank_entry,agc_word inline_cadr,agc_word fetched_original_word);
#endif
