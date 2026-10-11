/* SPDX-License-Identifier: MIT */
#ifndef VELOCITY_VECTORS_H
#define VELOCITY_VECTORS_H
#if defined(VELOCITY_DIAGNOSTIC_HALF) && VELOCITY_DIAGNOSTIC_HALF==0
#define VL_FIRST_CASE 0u
#define VL_LAST_CASE 80u
#include "velocity_vectors_a.h"
#elif defined(VELOCITY_DIAGNOSTIC_HALF) && VELOCITY_DIAGNOSTIC_HALF==1
#define VL_FIRST_CASE 80u
#define VL_LAST_CASE 147u
#include "velocity_vectors_b.h"
#else
#define VL_FIRST_CASE 0u
#define VL_LAST_CASE 147u
#include "velocity_vectors_all.h"
#endif
#endif
