/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_IMAGE_BINDING_H
#define CM_ALARM_IMAGE_BINDING_H
#include "cm_alarm_image.h"
typedef struct cm_alarm_image_fixture {uint32_t source_case;uint16_t run,status,step,failures,fixed_word,fixed_address,needs_data;} cm_alarm_image_fixture;
#define CM_ALARM_IMAGE_FIXTURE ((cm_alarm_image_fixture *)(uintptr_t)0xc000)
#define CM_ALARM_IMAGE_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_alarm_image_fixture_fits[(sizeof(cm_alarm_image_fixture)<=0x40)?1:-1];
typedef char cm_alarm_image_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
