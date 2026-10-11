/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MINUTES_OUTPUT_VECTORS_H
#define DSKY_MINUTES_OUTPUT_VECTORS_H
#ifdef DSKY_MINUTES_ONLY_STAGE
#if DSKY_MINUTES_ONLY_STAGE==0
#include "dsky_minutes_output_vectors_0.h"
#elif DSKY_MINUTES_ONLY_STAGE==1
#include "dsky_minutes_output_vectors_1.h"
#elif DSKY_MINUTES_ONLY_STAGE==2
#include "dsky_minutes_output_vectors_2.h"
#elif DSKY_MINUTES_ONLY_STAGE==3
#include "dsky_minutes_output_vectors_3.h"
#elif DSKY_MINUTES_ONLY_STAGE==4
#include "dsky_minutes_output_vectors_4.h"
#elif DSKY_MINUTES_ONLY_STAGE==5
#include "dsky_minutes_output_vectors_5.h"
#endif
#else
#include "dsky_minutes_output_vectors_all.h"
#endif
#endif
