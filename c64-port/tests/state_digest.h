/* SPDX-License-Identifier: MIT. Diagnostic-only serialization, not Apollo logic. */
#ifndef STATE_DIGEST_H
#define STATE_DIGEST_H
#include "executive.h"
uint32_t state_digest(const executive *s,int result);
#endif
