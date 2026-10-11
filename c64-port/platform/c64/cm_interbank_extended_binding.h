/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_EXTENDED_BINDING_H
#define CM_INTERBANK_EXTENDED_BINDING_H
#include "cm_interbank_extended.h"
typedef struct cm_ib_ext_fixture {uint32_t source_case;uint16_t run,status,step,failures,literal,fetched;} cm_ib_ext_fixture;
#define CM_IB_EXT_FIXTURE ((cm_ib_ext_fixture *)(uintptr_t)0xc000)
#define CM_IB_EXT_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_ib_ext_fixture_fits[(sizeof(cm_ib_ext_fixture)<=0x40)?1:-1];
typedef char cm_ib_ext_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
