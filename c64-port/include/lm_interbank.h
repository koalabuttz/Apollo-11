/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERBANK_H
#define LM_INTERBANK_H
#include "p66_bias.h"
#include "agc_word.h"
#include <stdbool.h>
enum lm_interbank_entry {LM_IB_BANKCALL,LM_IB_SWCALL,LM_IB_SWRETURN,LM_IB_POSTJUMP,LM_IB_BANKJUMP,LM_IB_MAKECADR,LM_IB_SUPDACAL,LM_IB_IBNKCALL,LM_IB_ISWCALLL,LM_IB_ISWRETRN,LM_IB_USPRCADR,LM_IB_SUPERSW,LM_IB_QJUMP,LM_IB_PRIO12};
#define LM_INTERBANK_PRIO12_WORD UINT16_C(012000)
bool lm_interbank_literal(uint16_t physical_fixed_word,agc_word *out);
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
bool lm_interbank(apollo_overlay_state *,p66_bias_io *,enum lm_interbank_entry,agc_word fixed_inline_cadr,agc_word passive_fetched_word);
bool lm_interbank_data_address(const apollo_overlay_state *,const p66_bias_io *,uint16_t *physical_fixed_word);
/* lm_interbank for one group of entries each (false for any other entry), so
 * a routine table registering only those entries links only their code:
 * BANKCALL, SWCALL, IBNKCALL and ISWCALLL; SWRETURN and ISWRETRN; POSTJUMP
 * and BANKJUMP. */
bool lm_interbank_call(apollo_overlay_state *,p66_bias_io *,enum lm_interbank_entry,agc_word fixed_inline_cadr);
bool lm_interbank_return(apollo_overlay_state *,p66_bias_io *,enum lm_interbank_entry);
bool lm_interbank_jump(apollo_overlay_state *,p66_bias_io *,enum lm_interbank_entry,agc_word fixed_inline_cadr);
#endif
