/* SPDX-License-Identifier: MIT */
/* The entry image a run restores when it refuses (the files whose io record
 * carries `undo`: SERVICER, BURN_BABY_BURN, IMU_PERFORMANCE_TESTS,
 * LUNAR_LANDING, AGS_INITIALIZATION). Host runners and the C64 diagnostics
 * give io->undo an image of its own and copy into it. The C64 flight
 * resident has no 4 KB of RAM to spare, so its modules (RK_LM or RK_CM)
 * stash the image in the REU at FLIGHT_UNDO_REU instead; io->undo is then
 * only a non-null marker (the resident sets it to 1) and is never read.
 * Runs do not nest (an interrupt is taken between runs), so one scratch
 * area serves every module. */
#ifndef FLIGHT_UNDO_H
#define FLIGHT_UNDO_H
#include "overlay_runtime.h"
#if defined(__mos__) && (defined(RK_LM) || defined(RK_CM))
#include "overlay_reu.h"
/* $1F2000-$1F2FFF: above the CM rope image, below the VICE records at
 * $1F8000, free on both missions (docs/FLIGHT-STORE.md). */
#define FLIGHT_UNDO_REU 0x1F2000ul
#define FLIGHT_UNDO_SAVE(io, s)                                                                    \
    ((void)(io),                                                                                   \
     (void)apollo_reu_transfer(APOLLO_REU_STASH, (s), FLIGHT_UNDO_REU, (uint16_t)sizeof *(s)))
#define FLIGHT_UNDO_RESTORE(entry, s)                                                              \
    ((void)(entry),                                                                                \
     (void)apollo_reu_transfer(APOLLO_REU_FETCH, (s), FLIGHT_UNDO_REU, (uint16_t)sizeof *(s)))
#define FLIGHT_UNDO_ALIAS(io, s) 0
#else
#define FLIGHT_UNDO_SAVE(io, s)       (*(io)->undo = *(s))
#define FLIGHT_UNDO_RESTORE(entry, s) (*(s) = *(entry)->undo)
#define FLIGHT_UNDO_ALIAS(io, s)      ((io)->undo == (s))
#endif
#endif
