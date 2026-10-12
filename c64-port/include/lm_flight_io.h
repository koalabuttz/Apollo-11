/* SPDX-License-Identifier: MIT */
#ifndef LM_FLIGHT_IO_H
#define LM_FLIGHT_IO_H
#include "p66_bias.h"
/* Device record for Lane F (LM powered flight and DAP) routines of the
 * control-transfer ABI. bias comes first, so Executive and Waitlist routines
 * (lm_exec_routines) that take p66_bias_io read the same record. Output
 * channels hold the bits the original writes with WOR/WAND/WRITE. */
typedef struct {
 p66_bias_io bias;
 agc_word chan14;   /* channel 14: CDU drive, thrust drive (BIT4), ... */
 agc_word chan5,chan6;  /* RCS jet commands */
 agc_word chan12;   /* IMU error counter enable, trim gimbal drives, ... */
 agc_word chan30,chan31; /* inputs: PGNCS control (30 bit 10), mode select (31 bits 13-14) */
} lm_flight_io;
#endif
