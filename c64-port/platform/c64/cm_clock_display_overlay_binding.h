/* SPDX-License-Identifier: MIT */
#ifndef C64_CM_CLOCK_DISPLAY_OVERLAY_BINDING_H
#define C64_CM_CLOCK_DISPLAY_OVERLAY_BINDING_H
#include "cm_clock_display_mission.h"
#include <stdint.h>
/* External latches and native metadata, never another persistent source image.
 * C64 IRQ source producers are excluded while a source group/loader runs. */
#define CM_CLOCK_DISPLAY_IO ((cm_native_io *)(uintptr_t)0xc400)
#define CM_CLOCK_DISPLAY_RUNTIME ((cm_mission_loop *)(uintptr_t)0xc430)
#define CM_CLOCK_DISPLAY_CALL ((cm_mission_call *)(uintptr_t)0xc460)
typedef struct {uint16_t run,source_case,step,group,status,failures;} cm_mission_fixture;
#define CM_CLOCK_DISPLAY_FIXTURE ((cm_mission_fixture *)(uintptr_t)0xc000)
typedef char cm_io_lease[(sizeof(cm_native_io)<=0x20u)?1:-1];
typedef char cm_runtime_lease[(sizeof(cm_mission_loop)<=0x30u)?1:-1];
typedef char cm_call_lease[(sizeof(cm_mission_call)<=0x20u)?1:-1];
typedef char cm_fixture_lease[(sizeof(cm_mission_fixture)<=0x40u)?1:-1];
#endif
