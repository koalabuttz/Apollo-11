/* SPDX-License-Identifier: MIT */
#ifndef P66_NATIVE_H
#define P66_NATIVE_H
#include "overlay_runtime.h"
#include "irq_delivery.h"
/* Compiled RODCOMP groups, never interpreter opcodes. Every entry returns
 * before another module may replace the overlay. The image is authoritative;
 * numerical objects exist only on this call's C stack. */
enum p66_native_group { P66_NATIVE_PREPARE=1, P66_NATIVE_SCALE_TIME=2,
    P66_NATIVE_ELAPSED_STORE=3, P66_NATIVE_DIVIDE_GRAVITY=4,
    P66_NATIVE_VELOCITY=5 };
enum p66_native_status { P66_NATIVE_OK=0, P66_NATIVE_UNSUPPORTED=1,
    P66_NATIVE_BAD_VAC=2, P66_NATIVE_BAD_MODE=3, P66_NATIVE_BOUNDARY=4 };
/* Caller excludes every source producer throughout the call. Unsupported
 * selector, MODE or VAC ownership rejects unchanged. Successful output owns
 * only documented numerical words; interpreter/Executive control is separate.
 * A/L preserve arithmetic boundary debris, not suspended CPU registers. */
uint16_t p66_native_entry(apollo_overlay_state *,uint16_t selector);
void p66_native_export_executive(apollo_overlay_state *,const executive *);
void p66_native_import_executive(executive *,const apollo_overlay_state *);
typedef struct {
    apollo_overlay_state *image;
    exec_native *runtime;
    const exec_exclusion *producers;
    /* Called under producer exclusion, after arithmetic and before DANZIG.
     * Observer must not dispatch, publish producers or retain transient views. */
    void (*observe)(void *,const apollo_overlay_state *,agc_word completed_loc);
    void *observer;
    enum p66_native_status status;
    /* Resident overlay invoker returns only after its C frame has unwound.
     * No scheduling occurs inside this hook. Null uses linked host kernels. */
    uint16_t (*invoke)(void *,apollo_overlay_state *,uint16_t);
    void *modules;
} p66_native_binding;
/* Table includes the source RODTASK SUPERBNK060 admission as well as the
 * physical bank pair. Last entry is a deliberate bounded stop before UNIT:
 * it faults without releasing the current job/VAC or inventing ENDJOB. */
extern const exec_context_mapping p66_native_mappings[12];
#define P66_NATIVE_MAPPING_COUNT 12u
/* Synchronize the Executive transient view before/after runtime operations.
 * Alarm is native metadata, preserved by import; other unowned words remain.
 * All calls must use caller exclusion; bindings require restoring hooks. */
enum exec_status p66_native_step(p66_native_binding *);
#endif
