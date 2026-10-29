/* SPDX-License-Identifier: MIT */
#include "interpreter_native_check.h"
#include "interpreter_native_vectors.h"
#include "state_digest.h"
static uint32_t add(uint32_t h,agc_word w) {
    h=(h^(uint8_t)w)*UINT32_C(16777619);
    return (h^(uint8_t)(w>>8))*UINT32_C(16777619);
}
uint32_t in_digest(const in_run *r,enum exec_status status) {
    uint32_t h=state_digest(&r->state,(int)status);
    h=add(h,r->runtime.current.location); h=add(h,r->runtime.current.bank);
    h=add(h,(agc_word)r->runtime.current_kind);
    h=add(h,r->runtime.interpreter_control.intbit15);
    return add(h,r->runtime.interpreter_control.edop);
}
int in_checkpoint(const in_run *r,unsigned n,unsigned step,enum exec_status status) {
    unsigned row=n<7u ? n*IN_STEPS+step : 7u*IN_STEPS+n-7u;
    if(n>=IN_CASE_COUNT || step>=in_steps(n)) return 0;
    return r->failures==0 && in_digest(r,status)==interpreter_native_vectors[row];
}
