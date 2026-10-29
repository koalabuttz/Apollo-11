/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MONITOR_VECTORS_H
#define DSKY_MONITOR_VECTORS_H
#ifdef DSKY_MONITOR_ONLY_STAGE
#if DSKY_MONITOR_ONLY_STAGE==0
#include "dsky_monitor_vectors_0.h"
#elif DSKY_MONITOR_ONLY_STAGE==1
#include "dsky_monitor_vectors_1.h"
#elif DSKY_MONITOR_ONLY_STAGE==2
#include "dsky_monitor_vectors_2.h"
#elif DSKY_MONITOR_ONLY_STAGE==3
#include "dsky_monitor_vectors_3.h"
#elif DSKY_MONITOR_ONLY_STAGE==4
#include "dsky_monitor_vectors_4.h"
#elif DSKY_MONITOR_ONLY_STAGE==5
#include "dsky_monitor_vectors_5.h"
#elif DSKY_MONITOR_ONLY_STAGE==6
#include "dsky_monitor_vectors_6.h"
#elif DSKY_MONITOR_ONLY_STAGE==7
#include "dsky_monitor_vectors_7.h"
#elif DSKY_MONITOR_ONLY_STAGE==8
#include "dsky_monitor_vectors_8.h"
#elif DSKY_MONITOR_ONLY_STAGE==9
#include "dsky_monitor_vectors_9.h"
#elif DSKY_MONITOR_ONLY_STAGE==10
#include "dsky_monitor_vectors_10.h"
#endif
#else
#include "dsky_monitor_vectors_all.h"
#endif
#endif
