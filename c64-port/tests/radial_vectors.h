/* SPDX-License-Identifier: MIT */
#ifndef RADIAL_VECTORS_H
#define RADIAL_VECTORS_H
#if defined(RADIAL_DIAGNOSTIC_HALF) && RADIAL_DIAGNOSTIC_HALF==0
#include "radial_vectors_a.h"
#elif defined(RADIAL_DIAGNOSTIC_HALF) && RADIAL_DIAGNOSTIC_HALF==1
#include "radial_vectors_b.h"
#elif defined(RADIAL_DIAGNOSTIC_HALF) && RADIAL_DIAGNOSTIC_HALF==2
#include "radial_vectors_c.h"
#elif defined(RADIAL_DIAGNOSTIC_HALF) && RADIAL_DIAGNOSTIC_HALF==3
#include "radial_vectors_d.h"
#elif defined(RADIAL_DIAGNOSTIC_HALF) && RADIAL_DIAGNOSTIC_HALF==4
#include "radial_vectors_e.h"
#elif defined(RADIAL_DIAGNOSTIC_HALF) && RADIAL_DIAGNOSTIC_HALF==5
#include "radial_vectors_f.h"
#else
#include "radial_vectors_all.h"
#endif
#endif
