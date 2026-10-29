/* SPDX-License-Identifier: MIT */
#ifndef C64_CM_SERVICE_OVERLAY_BINDING_H
#define C64_CM_SERVICE_OVERLAY_BINDING_H
#include "cm_service_loop.h"
#include <stdint.h>
/* External latches and native metadata, never another persistent source image.
 * C64 IRQ source producers are excluded while a source group/loader runs. */
#define CM_SERVICE_IO ((cm_native_io *)(uintptr_t)0xc400)
#define CM_SERVICE_RUNTIME ((cm_service_loop *)(uintptr_t)0xc430)
#define CM_SERVICE_CALL ((cm_service_call *)(uintptr_t)0xc440)
typedef struct {uint16_t run,source_case,step,group,status,failures;} cm_service_fixture;
#define CM_SERVICE_FIXTURE ((cm_service_fixture *)(uintptr_t)0xc000)
typedef char cm_io_lease[(sizeof(cm_native_io)<=0x20u)?1:-1];
typedef char cm_runtime_lease[(sizeof(cm_service_loop)<=0x10u)?1:-1];
typedef char cm_call_lease[(sizeof(cm_service_call)<=0x20u)?1:-1];
typedef char cm_fixture_lease[(sizeof(cm_service_fixture)<=0x40u)?1:-1];
#endif
