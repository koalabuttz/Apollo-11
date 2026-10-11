/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_CONSUMER_VECTORS_H
#define DSKY_DECIMAL_CONSUMER_VECTORS_H
#ifdef DSKY_DECIMAL_CONSUMER_ONLY_STAGE
#if DSKY_DECIMAL_CONSUMER_ONLY_STAGE==0
#include "dsky_decimal_consumer_vectors_0.h"
#elif DSKY_DECIMAL_CONSUMER_ONLY_STAGE==1
#include "dsky_decimal_consumer_vectors_1.h"
#elif DSKY_DECIMAL_CONSUMER_ONLY_STAGE==2
#include "dsky_decimal_consumer_vectors_2.h"
#elif DSKY_DECIMAL_CONSUMER_ONLY_STAGE==3
#include "dsky_decimal_consumer_vectors_3.h"
#elif DSKY_DECIMAL_CONSUMER_ONLY_STAGE==4
#include "dsky_decimal_consumer_vectors_4.h"
#elif DSKY_DECIMAL_CONSUMER_ONLY_STAGE==5
#include "dsky_decimal_consumer_vectors_5.h"
#elif DSKY_DECIMAL_CONSUMER_ONLY_STAGE==6
#include "dsky_decimal_consumer_vectors_6.h"
#endif
#else
#include "dsky_decimal_consumer_vectors_all.h"
#endif
#endif
