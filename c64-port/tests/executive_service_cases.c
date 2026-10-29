/* SPDX-License-Identifier: MIT
 * Input constructors only; original assembly supplies expected results. */
#include "executive_service_cases.h"
#include "scenarios.h"
void exec_service_prepare(exec_service_case *c, unsigned n) {
    static const agc_word banks[]={0,06000,040003,076167};
    static const agc_word pointers[]={0,AGC_NEG_ZERO,0401,073376};
    scenario initial;
    unsigned i,j,slot;
    scenario_prepare(&initial,(uint8_t)(n%26u));
    c->state=initial.state;
    for(i=0;i<EXEC_CORE_COUNT;++i)
        for(j=0;j<8;++j)
            c->state.core[i][j]=(agc_word)((n*71u+i*613u+j*127u)&AGC_MASK);
    for(i=0;i<EXEC_VAC_COUNT;++i)
        for(j=1;j<EXEC_VAC_WORDS;++j)
            c->state.vac[i][j]=(agc_word)((n*113u+i*83u+j*733u)&AGC_MASK);
    c->address.location=(agc_word)(02345u+(n%127u));
    if(n&1u) c->address.location=(agc_word)(c->address.location^AGC_MASK);
    c->address.bank=banks[n%4u];
    c->q=(agc_word)(01000u+(n%0777u));
    if(n<4u) c->q=(agc_word)n;
    if(n==4u) c->q=07777;
    c->superbank=(agc_word)((n%4u)*040u);
    c->control.intbit15=040000;
    c->control.edop=0;
    c->state.newprio=(agc_word)((1u+(n%31u))*01000u);
    if(n>=EXEC_SERVICE_SPVAC_COUNT) {
        slot=1u+(n%7u);
        c->state.newjob=(agc_word)(slot*EXEC_CORE_WORDS);
        c->state.core[0][EXEC_PRIORITY]=02401;
        c->state.core[0][EXEC_LOC]=(agc_word)(02000u+(n%01777u));
        c->state.core[0][EXEC_BANK]=banks[(n/8u)%4u];
        c->state.core[0][EXEC_PUSH]=pointers[(n/16u)%4u];
        c->state.core[slot][EXEC_PRIORITY]=03455;
        c->state.core[slot][EXEC_LOC]=(agc_word)(03000u+(n%0377u));
        if(n&1u) c->state.core[slot][EXEC_LOC]^=AGC_MASK;
        c->state.core[slot][EXEC_BANK]=banks[(n/2u)%4u];
        c->state.core[slot][EXEC_PUSH]=pointers[(n/4u)%4u];
        c->state.ovfind=pointers[(n/32u)%4u];
        if(n==256u) c->state.core[slot][EXEC_LOC]=AGC_NEG_ZERO;
        if(n==257u) c->state.core[slot][EXEC_LOC]=070001;
    }
}
