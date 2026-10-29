/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "executive_service_check.h"
int main(void) {
    exec_service_case c,saved;
    exec_transfer transfer={{01234,05670},1},old=transfer;
    unsigned n;
    for(n=0;n<EXEC_SERVICE_CASE_COUNT;++n) assert(exec_service_check(n));
    /* EDOP nonzero means CHANG2's source safe point was not reached. Reject
     * every normal raw value without changing context, control or output. */
    for(n=1;n<=AGC_MASK;++n) {
        exec_service_prepare(&c,257); c.control.edop=(agc_word)n; saved=c;
        assert(exec_change_interpretive(&c.state,c.superbank,&c.control,&transfer)==EXEC_OUT_OF_SCOPE);
        assert(memcmp(&c,&saved,sizeof c)==0);
        assert(memcmp(&transfer,&old,sizeof transfer)==0);
    }
    for(n=0;n<=AGC_MASK;++n) {
        if(n>0 && n<=84u && n%EXEC_CORE_WORDS==0) continue;
        exec_service_prepare(&c,257); c.state.newjob=(agc_word)n; saved=c;
        assert(exec_change_interpretive(&c.state,c.superbank,&c.control,&transfer)==EXEC_OUT_OF_SCOPE);
        assert(memcmp(&c,&saved,sizeof c)==0);
        assert(memcmp(&transfer,&old,sizeof transfer)==0);
    }
    exec_service_prepare(&c,0); c.state.newprio=0; saved=c;
    assert(exec_spvac(&c.state,c.address,&c.q)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&c,&saved,sizeof c)==0);
    exec_service_prepare(&c,257); c.state.core[1][EXEC_LOC]=07777;
    c.state.newjob=12; c.state.core[1][EXEC_PRIORITY]=03455;
    c.state.core[1][EXEC_BANK]=06000;
    assert(exec_change_interpretive(&c.state,c.superbank,&c.control,&transfer)==EXEC_OK);
    assert(transfer.interpretive==0 && transfer.address.location==07777);
    exec_service_prepare(&c,257); c.state.core[1][EXEC_LOC]=070000;
    c.state.newjob=12; c.state.core[1][EXEC_PRIORITY]=03455;
    c.state.core[1][EXEC_BANK]=06000; saved=c; old=transfer;
    assert(exec_change_interpretive(&c.state,c.superbank,&c.control,&transfer)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&c,&saved,sizeof c)==0);
    assert(memcmp(&transfer,&old,sizeof transfer)==0);
    puts("PASS: 768 original-derived SPVAC/CHANG2 state checks and unsupported safe-point rejection");
    return 0;
}
