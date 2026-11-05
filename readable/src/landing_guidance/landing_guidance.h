/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc (Luminary099 pp. 798-828): so far
 * only FASTCHNG (p. 826), which THROTTLE calls. The faithful C of FASTCHNG
 * is in c64-port/src/lm_throttle.c. */
#ifndef READABLE_LANDING_GUIDANCE_H
#define READABLE_LANDING_GUIDANCE_H

#include "../agc/agc.h"

#if LUMINARY099
void fastchng(void); /* FASTCHNG */
#endif

#endif
