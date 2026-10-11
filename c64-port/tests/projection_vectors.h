/* SPDX-License-Identifier: MIT */
#ifndef PROJECTION_VECTORS_H
#define PROJECTION_VECTORS_H
#if defined(PROJECTION_DIAGNOSTIC_PART) && PROJECTION_DIAGNOSTIC_PART==0
#include "projection_vectors_a.h"
#elif defined(PROJECTION_DIAGNOSTIC_PART) && PROJECTION_DIAGNOSTIC_PART==1
#include "projection_vectors_b.h"
#elif defined(PROJECTION_DIAGNOSTIC_PART) && PROJECTION_DIAGNOSTIC_PART==2
#include "projection_vectors_c.h"
#elif defined(PROJECTION_DIAGNOSTIC_PART) && PROJECTION_DIAGNOSTIC_PART==3
#include "projection_vectors_d.h"
#else
#include "projection_vectors_all.h"
#endif
#endif
