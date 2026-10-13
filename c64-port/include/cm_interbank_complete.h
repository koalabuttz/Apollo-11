/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_COMPLETE_H
#define CM_INTERBANK_COMPLETE_H
#include "cm_native_image.h"
#include "agc_word.h"
#include <stdbool.h>
enum cm_interbank_complete_entry {CM_IC_BANKCALL,CM_IC_SWCALL,CM_IC_SWRETURN,CM_IC_POSTJUMP,CM_IC_BANKJUMP,CM_IC_MAKECADR,CM_IC_SUPDACAL,CM_IC_IBNKCALL,CM_IC_ISWCALL,CM_IC_ISWRETRN,CM_IC_USPRCADR,CM_IC_SUPERSW,CM_IC_QJUMP,CM_IC_PRIO12};
#define CM_INTERBANK_COMPLETE_PRIO12_WORD UINT16_C(012000)
bool cm_interbank_complete_literal(uint16_t physical_fixed_word,agc_word *out);
/* Complete original explicit interbank controls. Ordinary source Q is a valid
 * compiled caller0100..07775; saved TC descriptors are0100..07777. Canonical
 * ordinary memory is15-bit; A/L retain raw16. Fixed inline literals and passive
 * SUPDACAL words are resolved by the caller's immutable native data reader.
 * Erasable inline aliases are read after the original stores. No opcode decoder.
 * Inline program words must occupy ordinary physical erasable storage>=0100,
 * excluding the CPU/editing/hardware counter registers as code storage.
 * Negative-overflow BANKJUMP maps its fixed source-modified CCS1777 and all four
 * successors into MAKECADR. TC Q then reaches register Q containing -1 rather
 * than a valid caller descriptor; return this explicit PC2 continuation before
 * any fetched register word executes. It is not a normal compiled-caller return.
 * Source-only continuation bodies remain caller-owned. Producers excluded;
 * rejection preserves complete image and external latches. */
bool cm_interbank_complete(apollo_overlay_state *,cm_native_io *,enum cm_interbank_complete_entry,agc_word fixed_inline_cadr,agc_word passive_fetched_word);
bool cm_interbank_complete_data_address(const apollo_overlay_state *,const cm_native_io *,uint16_t *physical_fixed_word);
/* lm_interbank for one group of entries each (false for any other entry), so
 * a routine table registering only those entries links only their code:
 * BANKCALL, SWCALL, IBNKCALL and ISWCALL; SWRETURN and ISWRETRN; POSTJUMP
 * and BANKJUMP. */
bool cm_interbank_complete_call(apollo_overlay_state *,cm_native_io *,enum cm_interbank_complete_entry,agc_word fixed_inline_cadr);
bool cm_interbank_complete_return(apollo_overlay_state *,cm_native_io *,enum cm_interbank_complete_entry);
bool cm_interbank_complete_jump(apollo_overlay_state *,cm_native_io *,enum cm_interbank_complete_entry,agc_word fixed_inline_cadr);
#endif
