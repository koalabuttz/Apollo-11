/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "scenarios.h"
/* Fixed test mailbox at $c000; outside SDK's default $0801..$9fff image.
 * DONE=$a5, failures, count, sizeof(executive) LE, sizeof(scenario) LE.
 * No CIA timer or IRQ adaptation is claimed by this diagnostic. */
static scenario s;
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc000;
    uint8_t n,failures=0;
    mail[0]=0;
    puts("APOLLO EXECUTIVE NATIVE C");
    for(n=0;n<SCENARIO_COUNT;++n) {
        int result;
        scenario_prepare(&s,n);
        result=s.complete ? exec_endjob_select(&s.state) :
            (int)exec_admit(&s.state,s.priority,s.location,s.bank,s.needs_vac);
        if(!scenario_expected(n,&s.state,result)) ++failures;
    }
    mail[1]=failures; mail[2]=SCENARIO_COUNT;
    mail[3]=(uint8_t)sizeof(executive); mail[4]=(uint8_t)(sizeof(executive)>>8);
    mail[5]=(uint8_t)sizeof(scenario); mail[6]=(uint8_t)(sizeof(scenario)>>8);
    puts(failures ? "EXECUTIVE TESTS FAILED" : "26 EXECUTIVE SCENARIOS PASS");
    puts("1201/1202 BAILOUT BOUNDARIES CHECKED");
    mail[0]=0xa5;
    return failures;
}
