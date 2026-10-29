/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "gravity_cases.h"
static void emit(const gravity_case *c,uint16_t n,uint8_t step,uint8_t stage) {
    uint16_t w[GV_WORD_COUNT]; unsigned i;
    gv_words(c,w); printf("%u %u %u",n,step,stage);
    for(i=0;i<GV_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(void) {
    uint16_t n;
    for(n=0;n<GV_CASE_COUNT;++n) {
        gravity_case c; uint8_t step; gv_prepare(&c,n);
        for(step=0;step<gv_steps(n);++step) {
            gv_next(&c,n,step);
            if(gv_integrated(n)) { gv_prefix(&c,n); emit(&c,n,step,0); }
            if(gv_apply(&c)!=P66_PUSH_OK) { fprintf(stderr,"scope rejection case %u/%u MODE %05o push %05o base %05o\n",n,step,c.previous.previous.previous.previous.preparation.mpac[7],c.previous.previous.previous.push.pushloc,c.previous.previous.previous.push.base); return 2; }
            emit(&c,n,step,1);
        }
    }
    return 0;
}
