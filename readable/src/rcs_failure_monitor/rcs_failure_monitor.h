/* SPDX-License-Identifier: MIT */
/* RCS_FAILURE_MONITOR.agc, the LM's only: the jet failure monitor
 * (Luminary099 pages 190-192). The faithful C is c64-port/src/lm_rcsmon.c.
 *
 * T4JUMP calls RCSMONIT every 0.48 seconds. When the crew isolate a pair
 * of failed jets (a valve switch sets a bit of channel 32's low 8), it
 * marks the two jets in the DAP's masks CH5MASK and CH6MASK so jet
 * selection leaves them out, and clears them again when the valve opens;
 * PVALVEST keeps the valve states it has acted on. One change per pass,
 * the highest-numbered bit first. A pass that changes the masks starts
 * 1/ACCJOB (NOVAC), so the DAP's switch curves follow. Every pass ends at
 * RCSMONEX, which is RESUME. */
#ifndef READABLE_RCS_FAILURE_MONITOR_H
#define READABLE_RCS_FAILURE_MONITOR_H

#include "../agc/agc.h"

#if LUMINARY099
void rcsmon(void);
void one_accfix_after_novac(void); /* 1/ACCFIX +4 */
#endif

#endif
