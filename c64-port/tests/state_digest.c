/* SPDX-License-Identifier: MIT */
#include "state_digest.h"
static uint32_t add(uint32_t h,uint16_t w) {
    h=(h ^ (uint8_t)w)*UINT32_C(16777619);
    return (h ^ (uint8_t)(w>>8))*UINT32_C(16777619);
}
uint32_t state_digest(const executive *s,int result) {
    uint8_t i,j;
    uint32_t h=add(UINT32_C(2166136261),(uint16_t)result);
    for(i=0;i<EXEC_CORE_COUNT;++i)
        for(j=0;j<EXEC_CORE_WORDS;++j) h=add(h,s->core[i][j]);
    for(i=0;i<EXEC_VAC_COUNT;++i)
        for(j=0;j<EXEC_VAC_WORDS;++j) h=add(h,s->vac[i][j]);
    h=add(h,s->newjob); h=add(h,s->newprio);
    h=add(h,s->newloc[0]); h=add(h,s->newloc[1]);
    h=add(h,s->locctr); h=add(h,s->fixloc); h=add(h,s->ovfind);
    return add(h,s->alarm);
}
