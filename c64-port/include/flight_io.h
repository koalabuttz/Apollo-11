/* SPDX-License-Identifier: MIT */
#ifndef FLIGHT_IO_H
#define FLIGHT_IO_H
/* One device record for the whole flight program (docs/FLIGHT-STORE.md).
 * Each lane's routine tables take io in their own layout: p66_bias_io or
 * cm_native_io, or a file's own record that extends one of them
 * (lm_fresh_io, lm_sv_io, ...). Those layouts disagree at the same
 * offsets, so the flight program keeps one canonical record: the channels in
 * the program's agc_hardware, everything else here. Before a table's routines
 * run, flight_io_enter writes the table's own record from the canonical one;
 * after, flight_io_leave writes back every field that record holds. */
#include "agc_transfer.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#include "lm_fresh_start.h"
#include "lm_servicer.h"
#include "lm_lunar_landing.h"
#include "lm_imu_performance.h"
#include "lm_burn_baby.h"
#include "lm_ags_init.h"
#include "imu_mode_switching.h"
#include "lm_downtelemetry.h"
#include "cm_fresh_start.h"
#include "cm_downtelemetry.h"
#include "agc_input_channels.h"
typedef struct {
    agc_word superbank, interrupt_enabled;
    agc_word channel16, channel34, channel35;   /* the rest are in agc_hardware */
    uint16_t trap31a, trap31b, trap32;
    agc_word executive_alarm, waitlist_alarm;
    agc_word warning;                           /* yaAGC WarningFilter over its threshold */
    apollo_overlay_state *undo;                 /* scratch image some files take */
} flight_io;
/* The _CHANNEL kinds are the base record followed by the input channels
 * compiled routines read (include/agc_input_channels.h); they only read
 * them. */
enum flight_io_kind {
    FLIGHT_IO_P66, FLIGHT_IO_CM_NATIVE, FLIGHT_IO_LM_FRESH, FLIGHT_IO_LM_SV,
    FLIGHT_IO_LM_LLD, FLIGHT_IO_LM_IPT, FLIGHT_IO_LM_BB, FLIGHT_IO_LM_AGS, FLIGHT_IO_LM_IMU,
    FLIGHT_IO_LM_DNTM, FLIGHT_IO_CM_FRESH, FLIGHT_IO_CM_DNTM, FLIGHT_IO_LM_CHANNEL, FLIGHT_IO_CM_CHANNEL,
    FLIGHT_IO_KINDS
};
typedef union {
    p66_bias_io p66; cm_native_io cm; lm_fresh_io lm_fresh; lm_sv_io lm_sv;
    lm_lld_io lm_lld; lm_ipt_io lm_ipt; lm_bb_io lm_bb; lm_ags_io lm_ags; lm_imu_io lm_imu;
    lm_dntm_io lm_dntm; cm_fresh_io cm_fresh; cm_dntm_io cm_dntm;
    lm_channel_io lm_channel; cm_channel_io cm_channel;
} flight_io_buffer;
/* The record of kind in buffer, from io and hw; returns buffer. */
void *flight_io_enter(enum flight_io_kind kind, const flight_io *io, const agc_hardware *hw, flight_io_buffer *buffer);
/* Back from buffer into io and hw. */
void flight_io_leave(enum flight_io_kind kind, flight_io *io, agc_hardware *hw, const flight_io_buffer *buffer);
#endif
