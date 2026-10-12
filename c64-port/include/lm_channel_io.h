/* SPDX-License-Identifier: MIT */
#ifndef LM_CHANNEL_IO_H
#define LM_CHANNEL_IO_H
#include "p66_bias.h"
/* An LM routine's io record with the input channels it reads: the
 * p66_bias_io latches first (routines that read no channel see only that),
 * then channels 30 and 31 as yaAGC's InputChannel holds them (37777 and
 * 77777 when nothing is pressed or failed). */
typedef struct { p66_bias_io control; agc_word chan30, chan31; } lm_channel_io;
/* tests/interp_case.c carries a channel in an edit whose address is
 * IRC_CHANNEL_EDIT | channel. */
#define IRC_CHANNEL_EDIT 0x8000u
#endif
