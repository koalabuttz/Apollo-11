/* SPDX-License-Identifier: MIT */
#ifndef CM_FRESH_START_H
#define CM_FRESH_START_H
#include "agc_transfer.h"
#include "cm_native_image.h"
/* Comanche055 FRESH_START_AND_RESTART, bank 05 and fixed-fixed (fresh start,
 * hardware and software restart, the restart-group scan, ENEMA, T5IDLOC
 * and GOTOPOOH), as native routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md). Entries: SLAP1, MR.KLEAN, P00KLEAN,
 * V37KLEAN, GOPROG, ENEMA (GOPROG2), GOPROG3, T5IDLOC, GOTOPOOH and the
 * return points after every TC to code outside the file (BANKCALL VAC5STOR,
 * IBNKCALL SETCOARS, WAITLIST, MMDSPLAY, STOPRATE, SWCALL RESTARTS, ALARM,
 * PHASCHNG). Each routine runs until it leaves Z at such an external entry,
 * at POSTJUMP (DUMMYJOB +2, GOP00FIX), at NOQRSM +1, or at the fixed-fixed
 * return of a BANKCALL'd KLEAN entry. The fixed constants are compiled from
 * the source listing (src/cm_fresh_start_operands.inc); the CADRs after the
 * file's BANKCALL/IBNKCALL/POSTJUMP are in cm_interbank_site_table, which a
 * program running these routines uses as its operand table. io is a
 * cm_fresh_io (cm_native_io first, so cm_interbank_routines read the same
 * record); channel 11 is control.channel11. */
typedef struct {
 cm_native_io control;            /* channels 11-14, 15 (main_keys), 16 (navigation_keys), SUPERBNK */
 agc_word channel5,channel6;      /* outputs this file writes */
 agc_word channel33;              /* input; a write sets bits 11-15 (yaAGC CpuWriteIO) */
 agc_word warning;                /* nonzero: the AGC warning condition persists (yaAGC WarningFilter
                                     over its threshold), so a write leaves channel 33 bit 14 at 0 */
} cm_fresh_io;
#define CM_FS_ROUTINES 17u
#define CM_FS_ERESTORE 01360u
#define CM_FS_SKEEP4 01374u
#define CM_FS_SKEEP5 01375u
#define CM_FS_SKEEP7 01377u
extern const agc_routine_table cm_fresh_start_routines;
/* Entry Z addresses (bank 05; T5IDLOC is bank 05 too; GOTOPOOH fixed-fixed), for tests. */
extern const uint16_t cm_fresh_entries[CM_FS_ROUTINES];
#endif
