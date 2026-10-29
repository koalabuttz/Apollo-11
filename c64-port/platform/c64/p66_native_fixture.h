/* SPDX-License-Identifier: MIT */
#ifndef P66_NATIVE_FIXTURE_H
#define P66_NATIVE_FIXTURE_H
#include <stdint.h>
/* Diagnostic-only metadata outside the sole flight erasable image. Never
 * execution pointers or suspended frames; valid between returned C calls. */
typedef struct {uint32_t fingerprints[7];uint16_t failures;uint8_t stage,number,mixed;} p66_native_fixture;
#define P66_NATIVE_FIXTURE ((p66_native_fixture *)(uintptr_t)0xc000)
#endif
