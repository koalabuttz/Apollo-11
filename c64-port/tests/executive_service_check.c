/* SPDX-License-Identifier: MIT */
#include "executive_service_check.h"
#include "executive_service_vectors.h"
#include "state_digest.h"
static uint32_t add(uint32_t h,agc_word word) {
    h=(h^(uint8_t)word)*UINT32_C(16777619);
    return (h^(uint8_t)(word>>8))*UINT32_C(16777619);
}
uint32_t exec_service_digest(const exec_service_case *c,enum exec_status status,
                            exec_transfer transfer) {
    uint32_t h=state_digest(&c->state,(int)status);
    h=add(h,c->q); h=add(h,c->control.intbit15); h=add(h,c->control.edop);
    h=add(h,transfer.address.location); h=add(h,transfer.address.bank);
    return add(h,transfer.interpretive);
}
int exec_service_check(unsigned n) {
    exec_service_case c;
    exec_transfer transfer={{0,0},0};
    enum exec_status status;
    if(n>=EXEC_SERVICE_CASE_COUNT) return 0;
    exec_service_prepare(&c,n);
    status=n<EXEC_SERVICE_SPVAC_COUNT ? exec_spvac(&c.state,c.address,&c.q) :
        exec_change_interpretive(&c.state,c.superbank,&c.control,&transfer);
    return exec_service_digest(&c,status,transfer)==executive_service_vectors[n];
}
