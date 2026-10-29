/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_WORD_BINDING_H
#define CM_ABORT_WORD_BINDING_H
#include "cm_abort_word.h"
typedef struct cm_abort_word_fixture {uint32_t source_case;uint16_t run,status,step,failures,fixed_word,fixed_address,needs_data;} cm_abort_word_fixture;
#define CM_ABORT_WORD_FIXTURE ((cm_abort_word_fixture *)(uintptr_t)0xc000)
#define CM_ABORT_WORD_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_abort_word_fixture_fits[(sizeof(cm_abort_word_fixture)<=0x40)?1:-1];
typedef char cm_abort_word_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
