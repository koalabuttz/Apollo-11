/* SPDX-License-Identifier: MIT */
#ifndef DSKY_HOURS_OUTPUT_VECTORS_H
#define DSKY_HOURS_OUTPUT_VECTORS_H
#ifdef DSKY_HOURS_ONLY_STAGE
#if DSKY_HOURS_ONLY_STAGE==0
#include "dsky_hours_output_vectors_0.h"
#elif DSKY_HOURS_ONLY_STAGE==1
#include "dsky_hours_output_vectors_1.h"
#elif DSKY_HOURS_ONLY_STAGE==2
#include "dsky_hours_output_vectors_2.h"
#elif DSKY_HOURS_ONLY_STAGE==3
#include "dsky_hours_output_vectors_3.h"
#elif DSKY_HOURS_ONLY_STAGE==4
#include "dsky_hours_output_vectors_4.h"
#elif DSKY_HOURS_ONLY_STAGE==5
#include "dsky_hours_output_vectors_5.h"
#elif DSKY_HOURS_ONLY_STAGE==6
#include "dsky_hours_output_vectors_6.h"
#elif DSKY_HOURS_ONLY_STAGE==7
#include "dsky_hours_output_vectors_7.h"
#endif
#else
#include "dsky_hours_output_vectors_all.h"
#endif
#endif
