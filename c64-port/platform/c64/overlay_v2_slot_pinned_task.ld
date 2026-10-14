/* SPDX-License-Identifier: MIT */
/* Slot class 4: the pinned task region, the task slot's $e000..$fff9
 * (include/overlay_v2.h). */
MEMORY { slot (rw) : ORIGIN = 0xe000, LENGTH = 0x1ffa }
