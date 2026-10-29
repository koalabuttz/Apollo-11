/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MONITOR_CONSUMER_H
#define DSKY_MONITOR_CONSUMER_H
#include "dsky_monitor.h"
enum dsky_monitor_consumer_stage {DSKY_MONC_DISPLAY,DSKY_MONC_NOUN,DSKY_MONC_AFTER,DSKY_MONC_TEST,DSKY_MONC_MIXED,DSKY_MONC_MACHINE,DSKY_MONC_DISPATCH,DSKY_MONC_OCTAL_BEGIN,DSKY_MONC_CHANNEL,DSKY_MONC_OCTAL_FORMAT,DSKY_MONC_OCTAL_RETURN,DSKY_MONC_DECIMAL_LOAD,DSKY_MONC_DECIMAL_SELECT,DSKY_MONC_SCALAR,DSKY_MONC_DP,DSKY_MONC_DECIMAL_CALL,DSKY_MONC_DECIMAL_FORMAT,DSKY_MONC_DECIMAL_RETURN,DSKY_MONC_PASTE_BEGIN,DSKY_MONC_VERB,DSKY_MONC_PASTE_UPDATED,DSKY_MONC_BLANK,DSKY_MONC_PASTE_BLANKED,DSKY_MONC_BUSY,DSKY_MONC_FRONTIER};
/* Explicit compiled MONDO/output/paste stages. Canonical image/IO persist;
 * caller holds restoring producer exclusion and distinct next storage.
 * Selected existing scalar/DP/octal domains apply; unsupported arithmetic,
 * time/integer scaling, alarm/request/release and real END remain frontiers.
 * No arbitrary instruction decode, saved-return substitution or new poll. */
enum p66_bias_status dsky_monitor_consumer(apollo_overlay_state *,dsky_charin_io *,enum dsky_nv_mission,enum dsky_monitor_consumer_stage,agc_word,enum dsky_monitor_consumer_stage *);
/* Finite known compiled source entries only; unsupported -> FRONTIER. */
enum dsky_monitor_consumer_stage dsky_monitor_consumer_entry(const apollo_overlay_state *,enum dsky_nv_mission);
#endif
