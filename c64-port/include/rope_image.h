/* SPDX-License-Identifier: MIT */
#ifndef ROPE_IMAGE_H
#define ROPE_IMAGE_H
/* Read-only image of a mission's whole fixed rope, for the one program that
 * reads every word of it as data: AGC_BLOCK_TWO_SELF-CHECK's ROPECHK and
 * STSHOSUM (verb 91 show-banksum), through FXADRS and SUPDACAL. Only the
 * self-check sources may include this header (tools/ci_guard.py rejects any
 * other #include): every other module reads original constants through
 * fixed_data.h, whose certified spans keep instruction words from being read
 * as data. A lane that needs another word certifies its span there.
 *
 * Host: apollo_rope_load() reads the yaYUL binary of the unmodified rope
 * (build/fixed-data/<mission>/MAIN.agc.bin: big-endian words with the parity
 * bit lowest, banks in the order 2, 3, 0, 1, 4, ..., 043).
 * C64: the image is resident in the REU (tools/rope_image_pack.py), two bytes
 * per physical word, little-endian, at APOLLO_ROPE_REU_LM / APOLLO_ROPE_REU_CM:
 * the top 256 KB of the 2 MB REU, above the overlay ABI 2 module store
 * (docs/OVERLAY_ABI_V2.md). */
#include <stdbool.h>
#include "agc_word.h"
#define APOLLO_ROPE_WORDS (36u*1024u)
/* A flight store (one mission, built with RK_LM or RK_CM) carries its rope at
 * 0x1e0000 whichever the mission (tools/flight_store_build.py
 * FLIGHT_ROPE_BASE); builds with both ropes keep the LM's at 0x1c0000. */
#if defined(RK_LM) && RK_LM && !(defined(RK_CM) && RK_CM)
#define APOLLO_ROPE_REU_LM UINT32_C(0x1e0000)
#else
#define APOLLO_ROPE_REU_LM UINT32_C(0x1c0000)
#endif
#define APOLLO_ROPE_REU_CM UINT32_C(0x1e0000)
/* mission 0 Luminary099, 1 Comanche055; physical bank*1024+offset (fixed-fixed
 * banks 2 and 3 as themselves). False when no image is loaded or the address
 * is outside the rope; *word is then left intact. */
bool apollo_rope_read(unsigned mission,uint16_t physical,agc_word *word);
/* Host only: load the mission's yaYUL binary. False on a missing or
 * mis-sized file. */
bool apollo_rope_load(unsigned mission,const char *path);
#endif
