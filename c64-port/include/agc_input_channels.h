/* SPDX-License-Identifier: MIT */
#ifndef AGC_INPUT_CHANNELS_H
#define AGC_INPUT_CHANNELS_H
#include "p66_bias.h"
#include "cm_native_image.h"
/* Input channels that compiled routines read (tools/interp_compile.py spec
 * key 'channel_inputs'): the scalers (3 HISCALAR, 4 LOSCALAR) and the discrete
 * inputs 030-033, as READ returns them. They are not erasable memory: the
 * caller latches them for the run, so a scaler is frozen while it lasts.
 * The mission's io record comes first, so routines without channel reads
 * take the same record.
 *
 * Interim: the field names are the Kernel lane's agc_hardware ones, and
 * compiled code reads them only through AGC_CHANNEL(n). When agc_hardware
 * (program->hardware) carries them, AGC_CHANNEL moves there and this header
 * goes. */
typedef struct { agc_word channel3, channel4, channel30, channel31, channel32, channel33; } agc_input_channels;
typedef struct { p66_bias_io io; agc_input_channels in; } lm_channel_io;
typedef struct { cm_native_io io; agc_input_channels in; } cm_channel_io;
/* Channel n (3, 4, 30-33) of the run; agc_channels_in is set by the routine's wrapper. */
#define AGC_CHANNEL(n) (agc_channels_in->channel##n)
#endif
