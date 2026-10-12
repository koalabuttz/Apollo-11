/* SPDX-License-Identifier: MIT */
#ifndef CM_ENTRY_DAP_H
#define CM_ENTRY_DAP_H
#include "agc_image_ops.h"
/* Comanche055 CM_ENTRY_DIGITAL_AUTOPILOT (bank 15 2434-3754, bank 20
 * 3612-3737): the entry DAP's CDU read and body-rate task (READGYMB,
 * BODYRATE), the T5 attitude and rate update (ATTRATES) with the pitch and
 * yaw dead zones and the exo-atmospheric DAP, the 2-second roll DAP
 * (CM/RCS) and its jet timing (JETCALL), the FDAI and telemetry dump, and
 * the bank-20 start-up (CM/DAPON, CM/DAPIC), compiled one original word at
 * a time over the canonical 2048-word image. Channels 5 (pitch/yaw jets)
 * and 6 (roll jets) are written.
 * Entry: EB E6, FB bank 15 (BB 032006) or bank 20 (BB 040006) as the
 * entry's bank, any raw A/L/Q, erasable contents and channels, SW/NDX
 * -13..+13 (the telemetry file index CM/DUMPR keeps), JETAG -1, +0 or +1
 * (the values the file stores; -0 would fall from CCS JETAG into 3DDZ),
 * and Z one of
 *   bank 15: 2434 READGYMB, 2665 ATTRATES, 3207 SETJTAG, 3650 JETCALL
 *     (waitlist, T5 and restart tasks), and the returns from FLUSHJET
 *     (2446, 2476), PHASCHNG (2500, 3056, 3215), WAITLIST (2507, 3064,
 *     3642), SPCOS/SPSIN (2540, 2543, 2551, 2607, 2631, 2641, 2700, 2712,
 *     2724) and IBNKCALL NEEDLER (3717); at 3064, CMDAPMOD -1, +0, -0 or
 *     +1 (the values the file stores);
 *   bank 20: 3612 CM/DAPON, 3672 CM/DAPIC, and the returns
 *     from DOWNFLAG (3616, 3620), FLUSHJET (3627), BANKCALL DELAYJOB
 *     (3636), WAITLIST (3716) and 2PHSCHNG (3732).
 * Otherwise CM_ENTRY_DAP_OUT_OF_SCOPE, image and io unchanged.
 * CM_ENTRY_DAP_OK leaves Z at the external routine the file transfers to
 * (TASKOVER, WAITLIST, PHASCHNG, 2PHSCHNG, FLUSHJET, SPSIN, SPCOS,
 * IBNKCALL, RESUME, DOWNFLAG, BANKCALL or POSTJUMP) with Q its return, or
 * at bank 20's 2434 with Q 3715 when an overflowed TS CM/GYMDT skips
 * CM/DAPIC's TC WAITLIST and its 2CADR word runs as TC 2434.
 * CM_ENTRY_DAP_FAULT (an INDEX outside the file's tables or a TC Q to an
 * unknown return) cannot occur from an admitted entry. */
enum cm_entry_dap_status {
    CM_ENTRY_DAP_OK = 0,
    CM_ENTRY_DAP_OUT_OF_SCOPE = 1,
    CM_ENTRY_DAP_FAULT = 2
};
typedef struct { agc_word channel5, channel6; } cm_entry_dap_io;
enum cm_entry_dap_status cm_entry_dap(agc_image *, cm_entry_dap_io *);
#endif
