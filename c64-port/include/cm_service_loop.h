/* SPDX-License-Identifier: MIT */
#ifndef CM_SERVICE_LOOP_H
#define CM_SERVICE_LOOP_H
#include "cm_native_image.h"
enum cm_service_group {
 CM_SERVICE_T3_BEGIN,CM_SERVICE_TASK_FINISH,CM_SERVICE_SETJTAG,
 CM_SERVICE_CLOCK_ENTRY,CM_SERVICE_CLOCK_RETURN,CM_SERVICE_CLOCK_FINISH,
 CM_SERVICE_TIG5_ENTRY,CM_SERVICE_TIG5_RETURN,CM_SERVICE_TIG5_FINISH,
 CM_SERVICE_SVCT3_ENTRY,CM_SERVICE_SVCT3_RETURN,CM_SERVICE_SVCT3_RECHECK,
 CM_SERVICE_WAITLIST,CM_SERVICE_FINDVAC,CM_SERVICE_NOVAC,CM_SERVICE_FIXDELAY,
 CM_SERVICE_LONG_ENTER,CM_SERVICE_LONG_CYCLE,CM_SERVICE_LONG_RETURN,CM_SERVICE_LONG_TARGET,
 CM_SERVICE_RESTART_GROUP,CM_SERVICE_RESTART_PART2,CM_SERVICE_RESTART_TYPEB,
 CM_SERVICE_RESTART_CALL,CM_SERVICE_INIT_DISPLAY,CM_SERVICE_INIT_DISPLAY_RETURN,
 CM_SERVICE_SOURCE_RESUME,CM_SERVICE_APPLICATION
};
enum cm_service_status { CM_SERVICE_ADVANCED=0,CM_SERVICE_RESUME=1,CM_SERVICE_UNMAPPED=2,CM_SERVICE_INVALID=3,CM_SERVICE_FAULT=4,CM_SERVICE_REENTER=5 };
typedef struct {enum cm_service_group group;exec_address entry,argument;agc_word literal;} cm_service_call;
typedef enum cm_service_status (*cm_service_invoke)(const cm_service_call *,apollo_overlay_state *,cm_native_io *,void *);
typedef struct {
 apollo_overlay_state *image;cm_native_io *io;cm_service_invoke invoke;void *user;
 uint16_t completed_groups;uint8_t in_callback;
} cm_service_loop;
/* Resolver selects compiled source bodies/services only. No instruction word
 * or rope is fetched. Unknown genuine application entries remain visible. */
enum cm_service_status cm_service_identify(const apollo_overlay_state *,const cm_native_io *,cm_service_call *);
/* Each invocation returns before the next dispatcher step. The resident
 * callback may load an overlay after the preceding native C frame returned.
 * Producer exclusion spans each group and pending source service pairs.
 * Runtime metadata contains no persistent flight-word image. */
enum cm_service_status cm_service_step(cm_service_loop *);
/* Actual compiled body implementation for source qualification/standalone
 * targets; final overlay driver supplies its own loader-backed invoke hook. */
enum cm_service_status cm_service_builtin_invoke(const cm_service_call *,apollo_overlay_state *,cm_native_io *,void *);
#endif
