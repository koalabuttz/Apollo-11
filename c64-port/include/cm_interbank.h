/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_H
#define CM_INTERBANK_H
#include "cm_native_image.h"
enum cm_interbank_entry {CM_IB_BANKCALL,CM_IB_SWCALL,CM_IB_SWRETURN,CM_IB_POSTJUMP,CM_IB_BANKJUMP,CM_IB_MAKECADR,CM_IB_SUPDACAL,CM_IB_IBNKCALL,CM_IB_ISWCALL,CM_IB_ISWRETURN,CM_IB_USPRCADR,CM_IB_SUPERSW,CM_IB_QJUMP,CM_IB_PRIO12};
/* Original complete interbank control groups. inline_cadr is the explicit
 * compiled source inline literal for call/postjump/USPRCADR entries; no rope
 * instruction is fetched/decoded. SUPDACAL reads certified immutable data
 * spans only. Unsupported guards preserve canonical image and external I/O. */
enum exec_status cm_interbank(apollo_overlay_state *,cm_native_io *,enum cm_interbank_entry,agc_word);
#endif
