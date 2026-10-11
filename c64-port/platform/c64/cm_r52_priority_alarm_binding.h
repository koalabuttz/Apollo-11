/* SPDX-License-Identifier: MIT */
#ifndef CM_R52_PRIORITY_ALARM_BINDING_H
#define CM_R52_PRIORITY_ALARM_BINDING_H
#include "cm_r52_priority_alarm.h"
typedef struct cm_r52_priority_alarm_fixture {uint32_t source_case;uint16_t run,status,step,failures;} cm_r52_priority_alarm_fixture;
#define CM_R52_PRIORITY_ALARM_FIXTURE ((cm_r52_priority_alarm_fixture *)(uintptr_t)0xc000)
#define CM_R52_PRIORITY_ALARM_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_r52_priority_alarm_fixture_fits[(sizeof(cm_r52_priority_alarm_fixture)<=0x40)?1:-1];
typedef char cm_r52_priority_alarm_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
