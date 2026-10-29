/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_RECORD_BINDING_H
#define CM_ALARM_RECORD_BINDING_H
#include "cm_alarm_record.h"
typedef struct cm_alarm_fixture {uint32_t source_case;uint16_t run,status,step,failures;agc_word literal;} cm_alarm_fixture;
#define CM_ALARM_FIXTURE ((cm_alarm_fixture *)(uintptr_t)0xc000)
#define CM_ALARM_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_alarm_fixture_fits[(sizeof(cm_alarm_fixture)<=0x40)?1:-1];
typedef char cm_alarm_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
