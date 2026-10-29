/* SPDX-License-Identifier: MIT */
#ifndef DSKY_FIXED_DISPLAY_VECTORS_H
#define DSKY_FIXED_DISPLAY_VECTORS_H
#ifdef DSKY_FIXED_DISPLAY_ONLY_STAGE
#if DSKY_FIXED_DISPLAY_ONLY_STAGE==0
#include "dsky_fixed_display_vectors_0.h"
#elif DSKY_FIXED_DISPLAY_ONLY_STAGE==1
#include "dsky_fixed_display_vectors_1.h"
#else
#include "dsky_fixed_display_vectors_2.h"
#endif
#else
#include "dsky_fixed_display_vectors_all.h"
#endif
#endif
