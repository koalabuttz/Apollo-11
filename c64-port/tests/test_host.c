/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scenarios.h"
#include "state_digest.h"
#include "reference_vectors.h"
#include "dispatch_check.h"
#include "dispatch_cases.h"
static unsigned called;
static enum exec_action unmapped_resume(executive *s,void *user,exec_address *resume) {
    (void)s; (void)user; ++called;
    resume->location=LOW_RESUME; resume->bank=HIGH_BANK; return EXEC_YIELD;
}
static enum exec_action invalid_action(executive *s,void *user,exec_address *resume) {
    (void)s; (void)user; (void)resume; ++called; return (enum exec_action)99;
}
static void invalid_dispatch(void) {
    executive s,before;
    exec_address next={01234,0567},old_next=next,resume={02400,06122};
    exec_native runtime;
    exec_mapping map[2]={{{LOW_ENTRY,LOW_BANK},unmapped_resume},{{LOW_RESUME,LOW_BANK},invalid_action}};
    dispatch_prepare(&s,0); before=s;
    s.newjob=13; before=s;
    assert(exec_change_basic(&s,resume,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&before,&s,sizeof s)==0 && memcmp(&old_next,&next,sizeof next)==0);
    s.newjob=12; s.core[1][EXEC_LOC]=agc_negate(02345); before=s;
    assert(exec_change_basic(&s,resume,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&before,&s,sizeof s)==0);
    assert(exec_complete_basic(&s,0120,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&before,&s,sizeof s)==0); /* VAC not released */
    assert(exec_start_basic(&s,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&before,&s,sizeof s)==0);
    dispatch_prepare(&s,0); before=s; resume.location=010000;
    assert(exec_change_basic(&s,resume,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&before,&s,sizeof s)==0);
    assert(exec_complete_basic(&s,0200,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&before,&s,sizeof s)==0);
    assert(exec_change_basic(&s,resume,0)==EXEC_OUT_OF_SCOPE);
    s.alarm=01202; before=s;
    assert(exec_change_basic(&s,resume,&next)==EXEC_BAILOUT_PENDING);
    assert(exec_complete_basic(&s,0,&next)==EXEC_BAILOUT_PENDING);
    assert(exec_start_basic(&s,&next)==EXEC_BAILOUT_PENDING);
    assert(memcmp(&before,&s,sizeof s)==0);
    map[1]=map[0];
    assert(exec_native_init(&runtime,&s,map,2,0)==EXEC_OUT_OF_SCOPE);
    assert(exec_native_step(&runtime)==EXEC_OUT_OF_SCOPE);
    map[1].address.location=LOW_RESUME; map[1].call=invalid_action;
    exec_init(&s); assert(exec_admit(&s,02000,LOW_ENTRY,LOW_BANK,0)==EXEC_OK);
    assert(exec_native_init(&runtime,&s,map,2,0)==EXEC_OK);
    assert(exec_native_start(&runtime)==EXEC_OK);
    assert(exec_native_start(&runtime)==EXEC_OUT_OF_SCOPE);
    called=0; assert(exec_native_step(&runtime)==EXEC_UNMAPPED);
    assert(called==1); assert(exec_native_step(&runtime)==EXEC_UNMAPPED && called==1);
    /* Same location, different bank never accidentally calls a continuation. */
    exec_init(&s); assert(exec_admit(&s,02000,LOW_RESUME,LOW_BANK,0)==EXEC_OK);
    assert(exec_native_init(&runtime,&s,map,2,0)==EXEC_OK);
    assert(exec_native_start(&runtime)==EXEC_OK);
    called=0; assert(exec_native_step(&runtime)==EXEC_INVALID_ACTION);
    assert(exec_native_step(&runtime)==EXEC_INVALID_ACTION && called==1);
}
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
    assert(dispatch_check(&initialized)==0);
    assert(dispatch_demo(&initialized,42,0,0));
    invalid_dispatch();
    puts("PASS: 53 dispatch cases, 109 assembly-derived checkpoints, 8 native two-job sequences; dispatch fault checks");
    printf("PASS: 32768 word encodings, 65536 register encodings, %lu addition pairs, %u Executive scenarios\n",pairs,SCENARIO_COUNT);
    printf("sizeof(executive)=%lu bytes; sizeof(scenario)=%lu bytes (host)\n",
           (unsigned long)sizeof(executive),(unsigned long)sizeof(scenario));
    return 0;
}
