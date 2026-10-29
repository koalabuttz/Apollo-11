/* SPDX-License-Identifier: MIT */
#ifndef P66_APPLICATION_RECURRENCE_H
#define P66_APPLICATION_RECURRENCE_H
#include "p66_application.h"
#include "lm_clock.h"
#include "lm_waitlist_irq_dispatch.h"
#include "p66_rodtask_image.h"
#include "lm_idle_start.h"
enum p66_recurrence_service {P66_RECURRENCE_RODTASK=57,P66_RECURRENCE_DISPATCH=58,
 P66_RECURRENCE_RESUME=59,P66_RECURRENCE_IDLE=84,P66_RECURRENCE_PREPARE=85,P66_RECURRENCE_ADOPT=86};
typedef struct p66_application_recurrence {p66_application_binding application;uint16_t in_interrupt,pending_t3;
 uint16_t (*invoke_recurrence)(void *,struct p66_application_recurrence *,exec_address *,uint16_t,uint16_t,uint16_t *);
 void *recurrence_modules;
} p66_application_recurrence;
/* Actual one-shot P66VERTA queued RODTASK, delivered from literal idle ADVAN.
 * Hardware pulses stop when a job is admitted; consumed reports that prefix.
 * The source task does not requeue itself. Other due task bodies and +1 loop
 * routes remain excluded. Caller owns navigation/sensor input producers. */
enum exec_status p66_application_recur(p66_application_recurrence *,uint16_t centiseconds,uint16_t *consumed);
enum exec_status p66_application_recur_prepare(p66_application_recurrence *,uint16_t,uint16_t *);
enum exec_status p66_application_recur_adopt(p66_application_recurrence *,exec_address);
uint16_t p66_recurrence_entry(p66_application_recurrence *,exec_address *,uint16_t,uint16_t,uint16_t *);
#endif
