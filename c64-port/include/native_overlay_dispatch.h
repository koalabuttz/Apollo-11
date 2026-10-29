/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_OVERLAY_DISPATCH_H
#define NATIVE_OVERLAY_DISPATCH_H
#include "irq_delivery.h"
enum exec_overlay_stage {EXEC_OVERLAY_PREPARE=1,EXEC_OVERLAY_FINISH=2};
/* This frame and every pointer target remain resident until the step returns.
 * The resolved callback is from the resident mapping, never the replaced slot.
 * Preparation and finish are separately invoked, returning through the normal
 * C ABI before any mapped callback loads its own native stage. */
typedef struct {exec_native *runtime;exec_continuation call;exec_address resume;enum exec_action action;} exec_overlay_frame;
typedef struct {uint16_t (*invoke)(void *,exec_overlay_frame *,uint16_t stage);void *user;} exec_overlay_dispatch;
/* Same two source guard intervals and IRQ seams as exec_native_step. The
 * invoker loads/executes the requested locked scheduler stage synchronously.
 * It returns its exact exec_status, or0xffff for a loader/ABI failure; that
 * failure latches OUT_OF_SCOPE without executing a callback/lifecycle again.
 * The callback returns before the scheduler finish slot is loaded. Storage,
 * mapping callbacks and exclusion hooks must be resident. Binding requires
 * already-installed restoring exclusion hooks (an empty IRQ queue suffices). */
/* Explicit native action chooses finish family;0 denotes unsupported stage. */
uint8_t exec_overlay_module(uint16_t stage,enum exec_action action);
extern const exec_native_services exec_overlay_services;
enum exec_status exec_overlay_bind(exec_native *,const exec_overlay_dispatch *);
#endif
