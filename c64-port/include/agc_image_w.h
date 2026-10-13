/* SPDX-License-Identifier: MIT */
/* W: the AGC image's words, for code written against an image pointer s.
 * Flight builds (RK_FIXED_IMAGE) keep the one image at
 * APOLLO_SHARED_ADDRESS, so the C64 addresses its words absolutely instead
 * of through s: llvm-mos codes s->words[k] as (zp),y with 16-bit offset
 * arithmetic, and the fixed form is about 40% smaller. (void)s keeps s
 * used. Host and diagnostic builds index through s as before. */
#ifndef AGC_IMAGE_W_H
#define AGC_IMAGE_W_H
#include "overlay_runtime.h"
#if defined(__mos__) && defined(RK_FIXED_IMAGE) && RK_FIXED_IMAGE
#define W ((void)s,(uint16_t *)APOLLO_SHARED_ADDRESS)
#else
#define W s->words
#endif
#endif
