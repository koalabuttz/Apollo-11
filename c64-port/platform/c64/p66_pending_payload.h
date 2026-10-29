/* SPDX-License-Identifier: MIT */
#ifndef P66_PENDING_PAYLOAD_H
#define P66_PENDING_PAYLOAD_H
#include "p66_application_pending.h"
typedef struct {p66_application_pending *binding;exec_address *resume;uint16_t service;} p66_pending_arguments;
#endif
