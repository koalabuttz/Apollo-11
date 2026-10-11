/* SPDX-License-Identifier: MIT */
#ifndef ACCELERATION_PREFIX_H
#define ACCELERATION_PREFIX_H
#include "agc_word.h"
static agc_dp acc_prefix_time(uint16_t n){
    static const agc_dp times[12]={{0,0},{077777,077777},{0,1},{0,077776},{1,0},{077776,077777},{037777,037777},{040000,040000},{0,010000},{040000,1},{02000,0},{0100,012345}};
    return times[n%12u];
}
#endif
