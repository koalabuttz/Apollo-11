/* SPDX-License-Identifier: MIT */
#ifndef HORIZONTAL_VECTORS_H
#define HORIZONTAL_VECTORS_H
#if defined(HORIZONTAL_DIAGNOSTIC_PART) && HORIZONTAL_DIAGNOSTIC_PART==0
#include "horizontal_vectors_a.h"
#elif defined(HORIZONTAL_DIAGNOSTIC_PART) && HORIZONTAL_DIAGNOSTIC_PART==1
#include "horizontal_vectors_b.h"
#elif defined(HORIZONTAL_DIAGNOSTIC_PART) && HORIZONTAL_DIAGNOSTIC_PART==2
#include "horizontal_vectors_c.h"
#elif defined(HORIZONTAL_DIAGNOSTIC_PART) && HORIZONTAL_DIAGNOSTIC_PART==3
#include "horizontal_vectors_d.h"
#else
#include "horizontal_vectors_all.h"
#endif
#endif
