/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scenarios.h"
#include "state_digest.h"
#include "reference_vectors.h"
int main(void) {
    uint32_t a,b;
    uint8_t n;
    scenario s;
    executive initialized;
    unsigned long pairs=0;
    for(a=0;a<=AGC_MASK;++a) {
        int16_t signed_value=(a & 040000) ? -(int16_t)(077777-a) : (int16_t)a;
        uint16_t magnitude=(uint16_t)(signed_value<0 ? -signed_value : signed_value);
        assert(agc_to_int((agc_word)a)==signed_value);
        assert(agc_correct(agc_extend((agc_word)a))==a);
        assert(agc_negate(agc_negate((agc_word)a))==a);
        assert(agc_classify((agc_word)a)==(a==0 ? AGC_PLUS_ZERO :
               a==32767u ? AGC_MINUS_ZERO : a>=16384u ? AGC_NEGATIVE : AGC_POSITIVE));
        assert(agc_diminished_abs((agc_word)a)==(magnitude ? magnitude-1u : 0));
        assert(agc_compare((agc_word)a,agc_from_int(signed_value))==0);
        for(b=0;b<16;++b) {
            /* Distinct quotient/remainder oracle, preserving both zeros. */
            static const uint16_t boundary[16]={0,1,2,077,0100,0777,01000,037776,
                037777,040000,040001,076777,077676,077775,077776,077777};
            uint32_t sum=a+boundary[b];
            uint16_t expected=(uint16_t)(sum==0 ? 0 : 1u+(sum-1u)%32767u);
            assert(agc_add_word((agc_word)a,boundary[b])==expected); ++pairs;
        }
    }
    for(a=0;a<65536u;++a) {
        int ov=(a>>14)==1 ? 1 : (a>>14)==2 ? -1 : 0;
        assert(agc_overflow((agc_register)a)==ov);
        assert(agc_correct((agc_register)a)==(a%16384u+(a>=32768u ? 16384u : 0)));
    }
    assert(agc_add_register(agc_extend(037777),agc_extend(1))==040000);
    assert(agc_add_register(agc_extend(040000),agc_extend(077776))==0137777);
    scenario_prepare(&s,0); exec_init(&initialized);
    assert(memcmp(&initialized,&s.state,sizeof initialized)==0);
    for(n=0;n<SCENARIO_COUNT;++n) {
        int r;
        scenario_prepare(&s,n);
        r=s.complete ? exec_endjob_select(&s.state) :
            (int)exec_admit(&s.state,s.priority,s.location,s.bank,s.needs_vac);
        assert(scenario_expected(n,&s.state,r));
        assert(state_digest(&s.state,r)==reference_digest[n]);
    }
    exec_init(&initialized);
    assert(exec_admit(&initialized,0,0,0,0)==EXEC_OUT_OF_SCOPE);
    printf("PASS: 32768 word encodings, 65536 register encodings, %lu addition pairs, %u Executive scenarios\n",pairs,SCENARIO_COUNT);
    printf("sizeof(executive)=%lu bytes; sizeof(scenario)=%lu bytes (host)\n",
           (unsigned long)sizeof(executive),(unsigned long)sizeof(scenario));
    return 0;
}
