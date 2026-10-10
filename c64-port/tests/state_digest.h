/* SPDX-License-Identifier: MIT. Diagnostic-only serialization, not Apollo logic. */
#ifndef STATE_DIGEST_H
#define STATE_DIGEST_H
#include "executive.h"
uint32_t state_digest(const executive *s,int result);
uint32_t dispatch_state_digest(const executive *s, int result, exec_address next);
#endif
