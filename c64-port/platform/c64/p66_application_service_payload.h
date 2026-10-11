/* SPDX-License-Identifier: MIT */
#ifndef P66_APPLICATION_SERVICE_PAYLOAD_H
#define P66_APPLICATION_SERVICE_PAYLOAD_H
#include "p66_bias.h"
/* Resident-owned external I/O descriptor. Passed by explicit native pointer;
 * no callback/function pointer or C frame in a replaced slot is retained. */
typedef struct {p66_bias_io *control;uint16_t *channel11;uint16_t channel15;
 uint16_t wrote_channel10,channel10;} p66_application_service_arguments;
#endif
