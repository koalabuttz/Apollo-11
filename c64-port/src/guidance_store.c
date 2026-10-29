/* SPDX-License-Identifier: MIT */
#include "guidance_store.h"
enum p66_store_status p66_store_elapsed(p66_preparation *s,p66_pushdown *p) {
    agc_word old_low;
    if(s->mpac[7]!=0 || p->base<0401 || p->base>0661 ||
       (uint16_t)(p->base-0401u)%054u!=0) return P66_STORE_OUT_OF_SCOPE;
    old_low=p->words[31];
    p->words[30]=s->mpac[0]; p->words[31]=s->mpac[1];
    /* DCA MPAC / DXCH gives the old destination pair to A/L. CCS MODE
     * with +0 discards that old high word and clears A before TC Q. */
    s->rate.registers.a=0; s->rate.registers.l=agc_extend(old_low);
    return P66_STORE_OK;
}
