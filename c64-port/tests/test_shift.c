/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "shift_cases.h"
int main(void) {
    shift_case c,before;unsigned operation,mode;
    assert(sh_check(&c)==0);
    for(operation=0;operation<4u;++operation)for(mode=0;mode<=077777u;++mode) {
        uint16_t expected=operation<2u?0:077776;
        if(mode==expected)continue;
        sh_prepare(&c,(uint16_t)(operation*15u*SH_GROUP_SIZE+SH_GROUP_SIZE+137u));
        c.previous.previous.previous.preparation.mpac[7]=(agc_word)mode;
        before=c;assert(!sh_apply(&c));assert(memcmp(&c,&before,sizeof c)==0);
    }
    for(operation=0;operation<4u;++operation) {
        sh_prepare(&c,(uint16_t)(operation*15u*SH_GROUP_SIZE+137u));
        c.count=15;before=c;assert(!sh_apply(&c));assert(memcmp(&c,&before,sizeof c)==0);
        c.count=0;c.short_form=1;before=c;assert(!sh_apply(&c));assert(memcmp(&c,&before,sizeof c)==0);
        c.count=(uint8_t)(operation<2u?5:9);before=c;assert(!sh_apply(&c));assert(memcmp(&c,&before,sizeof c)==0);
    }
    puts("PASS: 168 complete original scalar/vector shift states; all unsupported MODEs and count boundaries reject unchanged");
    return 0;
}
