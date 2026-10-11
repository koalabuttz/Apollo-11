/* SPDX-License-Identifier: MIT */
#ifndef CM_CHARIN_NUMBER_H
#define CM_CHARIN_NUMBER_H
#include "cm_native_image.h"
/* Actual Comanche CHARIN40,2000 through NUM octal/decimal V/N/R1/R2/R3
 * to ENDOFJOB5217. Domain follows independently source-qualified
 * dsky_charin_number, bank40/SUPER100 retainedEB0..7. Actual CM source
 * caller/DECON address stores and channel11 event are retained. */
enum exec_status cm_charin_number(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11);
#endif
