/* SPDX-License-Identifier: MIT */
#ifndef LM_FLIGHT_IO_H
#define LM_FLIGHT_IO_H
#include "p66_bias.h"
#include "agc_transfer.h"
/* Lane F (LM powered flight and DAP) routines take the mission's base
 * record, p66_bias_io, and drive the output channels and read the inputs in
 * the program's agc_hardware: channel 14 (CDU drive, thrust drive), 5 and 6
 * (RCS jets), 12 (IMU error counter enable, trim gimbal drives), 13 (RHC
 * counter enable and read), the inputs 30 and 31 (PGNCS control, mode
 * select) and 4 (LOSCALAR), and in_interrupt (set by EDRUPT). A routine run
 * on a program without agc_hardware refuses.
 *
 * Their tests and references keep the two together: the record the
 * routines take, then the channels the case's program points at. */
typedef struct {
 p66_bias_io bias;
 agc_hardware hw;
} lm_flight_io;
/* In the routines' files: lmf_hw, the run's agc_hardware, which each routine
 * sets first with LMF_HW_SET(p) (false: refuse). A flight module build names
 * the resident's record with -DAGC_CHANNELS_HW=rk_hardware instead
 * (tools/flight_store_build.py): absolute addressing, nothing to set. */
#ifdef AGC_CHANNELS_HW
extern agc_hardware AGC_CHANNELS_HW;
#define lmf_hw (&AGC_CHANNELS_HW)
#define LMF_HW_SET(p) ((void)(p), 1)
#else
static __attribute__((unused)) agc_hardware *lmf_hw;
#define LMF_HW_SET(p) ((lmf_hw = (p)->hardware) != 0)
#endif
#endif
