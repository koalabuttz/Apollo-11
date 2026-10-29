/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OCTAL_CONSUMER_VECTORS_H
#define DSKY_OCTAL_CONSUMER_VECTORS_H
#ifdef DSKY_OCTAL_CONSUMER_ONLY_STAGE
#if DSKY_OCTAL_CONSUMER_ONLY_STAGE==0
#include "dsky_octal_consumer_vectors_0.h"
#elif DSKY_OCTAL_CONSUMER_ONLY_STAGE==1
#include "dsky_octal_consumer_vectors_1.h"
#elif DSKY_OCTAL_CONSUMER_ONLY_STAGE==2
#include "dsky_octal_consumer_vectors_2.h"
#elif DSKY_OCTAL_CONSUMER_ONLY_STAGE==3
#include "dsky_octal_consumer_vectors_3.h"
#endif
#else
#include "dsky_octal_consumer_vectors_all.h"
#endif
#endif
