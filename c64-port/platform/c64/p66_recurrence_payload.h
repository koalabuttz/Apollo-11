/* SPDX-License-Identifier: MIT */
#ifndef P66_RECURRENCE_PAYLOAD_H
#define P66_RECURRENCE_PAYLOAD_H
#include "p66_application_recurrence.h"
typedef struct {p66_application_recurrence *binding;exec_address *next;uint16_t ticks;uint16_t *consumed;} p66_recurrence_arguments;
#endif
