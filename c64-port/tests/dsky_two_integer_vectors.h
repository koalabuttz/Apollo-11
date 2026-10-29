/* SPDX-License-Identifier: MIT */
#ifndef DSKY_TWO_INTEGER_VECTORS_H
#define DSKY_TWO_INTEGER_VECTORS_H
#ifdef DSKY_TWO_INTEGER_ONLY_STAGE
#if DSKY_TWO_INTEGER_ONLY_STAGE==0
#include "dsky_two_integer_vectors_0.h"
#elif DSKY_TWO_INTEGER_ONLY_STAGE==1
#include "dsky_two_integer_vectors_1.h"
#elif DSKY_TWO_INTEGER_ONLY_STAGE==2
#include "dsky_two_integer_vectors_2.h"
#elif DSKY_TWO_INTEGER_ONLY_STAGE==3
#include "dsky_two_integer_vectors_3.h"
#elif DSKY_TWO_INTEGER_ONLY_STAGE==4
#include "dsky_two_integer_vectors_4.h"
#endif
#else
#include "dsky_two_integer_vectors_all.h"
#endif
#endif
