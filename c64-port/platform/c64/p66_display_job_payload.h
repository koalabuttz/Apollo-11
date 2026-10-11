/* SPDX-License-Identifier: MIT */
#ifndef P66_DISPLAY_JOB_PAYLOAD_H
#define P66_DISPLAY_JOB_PAYLOAD_H
#include "p66_bias.h"
#include "agc_word.h"
typedef struct {p66_bias_io *io;agc_word *channel11;} p66_display_job_arguments;
#endif
