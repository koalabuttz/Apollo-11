/* SPDX-License-Identifier: MIT */
#ifndef CM_CONTEXT_CHANGE_BINDING_H
#define CM_CONTEXT_CHANGE_BINDING_H
#include "cm_context_change.h"
typedef struct cm_ctx_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_transfer next;} cm_ctx_fixture;
#define CM_CTX_FIXTURE ((cm_ctx_fixture *)(uintptr_t)0xc000)
#define CM_CTX_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_ctx_fixture_fits[(sizeof(cm_ctx_fixture)<=0x40)?1:-1];
typedef char cm_ctx_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
