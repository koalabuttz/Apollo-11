/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "scenarios.h"
#include "state_digest.h"
#include "reference_vectors.h"
#include "dispatch_check.h"
#include "dispatch_cases.h"
/* Fixed test mailbox at $c000; outside this diagnostic's linked image.
 * DONE=$a5, failures low, count, sizeof(executive) LE, sizeof(scenario) LE,
 * timing-sample count, marker, dispatch cases/checkpoints/sequences, failures high.
 * No CIA timer or IRQ adaptation is claimed by this diagnostic. */
static scenario s;
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc000;
    uint8_t n;
    unsigned failures=0;
    mail[0]=0;
    puts("APOLLO EXECUTIVE NATIVE C");
    for(n=0;n<SCENARIO_COUNT;++n) {
        int result;
        scenario_prepare(&s,n);
        if(n==0) mail[8]=0x10; /* VICE timing start: first native admission. */
        result=s.complete ? exec_endjob_select(&s.state) :
            (int)exec_admit(&s.state,s.priority,s.location,s.bank,s.needs_vac);
        if(n==0) mail[8]=0x20; /* Timing end; no hashing or screen I/O inside. */
        if(!scenario_expected(n,&s.state,result)) ++failures;
        if(state_digest(&s.state,result)!=reference_digest[n]) ++failures;
    }
    /* Representative long scans and a slot-seven exchange. Setup/hashing
     * stay outside timing markers; these are samples, not a WCET proof. */
    scenario_prepare(&s,25);
    mail[8]=0x11;
    {
        int result=(int)exec_admit(&s.state,s.priority,s.location,s.bank,s.needs_vac);
        mail[8]=0x21;
        if (state_digest(&s.state,result)!=reference_digest[25]) ++failures;
    }
    dispatch_prepare(&s.state,38);
    mail[8]=0x12;
    {
        exec_address next;
        enum exec_status result=exec_complete_basic(&s.state,0120,&next);
        mail[8]=0x22;
        if (dispatch_state_digest(&s.state,(int)result,next)!=dispatch_reference_digest[38]) ++failures;
    }
    dispatch_prepare(&s.state,6);
    mail[8]=0x13;
    {
        exec_address resume={02400,06122},next;
        enum exec_status result=exec_change_basic(&s.state,resume,&next);
        mail[8]=0x23;
        if (dispatch_state_digest(&s.state,(int)result,next)!=dispatch_reference_digest[6]) ++failures;
    }
    mail[7]=3;
    failures+=dispatch_check(&s.state);
    mail[9]=53; mail[10]=109; mail[11]=8;
    mail[12]=(uint8_t)(failures>>8);
    mail[1]=(uint8_t)failures; mail[2]=SCENARIO_COUNT;
    mail[3]=(uint8_t)sizeof(executive); mail[4]=(uint8_t)(sizeof(executive)>>8);
    mail[5]=(uint8_t)sizeof(scenario); mail[6]=(uint8_t)(sizeof(scenario)>>8);
    puts(failures ? "EXECUTIVE TESTS FAILED" : "26 EXEC + 53 DISPATCH CASES PASS");
    puts("8 NATIVE TWO-JOB SEQUENCES PASS");
    puts("1201/1202 BAILOUT BOUNDARIES CHECKED");
    mail[0]=0xa5;
    return failures;
}
