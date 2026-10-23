/* SPDX-License-Identifier: MIT */
#ifndef FLIGHT_PART_H
#define FLIGHT_PART_H
/* A file split into flight parts (docs/FLIGHT-PARTS.md) runs as one routine
 * on the host. On the C64 a part's run that reaches another part's key
 * hands the routine back, and the next step's part goes on with it. The
 * part marks that hand-off itself (FLIGHT_PART_LEFT, in its _leave) in the
 * resident's byte at FLIGHT_PART_AT (rk_part_cont, platform/c64/
 * resident_flight.h), so the resident charges the continuation one routine
 * less. FS_PART_GROUP (tools/flight_store_build.py) is defined only in a
 * C64 part module; elsewhere the mark does nothing. */
#define FLIGHT_PART_AT 0x6ff6u
#ifdef FS_PART_GROUP
#define FLIGHT_PART_LEFT() (*(volatile unsigned char *)FLIGHT_PART_AT = (unsigned char)(FS_PART_GROUP))
#else
#define FLIGHT_PART_LEFT() ((void)0)
#endif
#endif
