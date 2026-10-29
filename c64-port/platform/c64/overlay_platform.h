/* SPDX-License-Identifier: MIT */
#ifndef C64_OVERLAY_PLATFORM_H
#define C64_OVERLAY_PLATFORM_H
#include "overlay_runtime.h"
typedef struct {
    uint8_t device,opened,eof,saved_port,saved_status,last_error;
} c64_overlay_io;
/* No existing IRQ gate is changed. Caller must quiesce every native IRQ
 * producer (stock ROM IRQs may run during serial I/O), and start with decimal
 * mode clear. Platform restores the incoming processor I/D and $01 mapping.
 * Resident and slot must remain accessible with BASIC ROM unmapped. */
apollo_overlay_backend c64_overlay_backend(c64_overlay_io *);
#endif
