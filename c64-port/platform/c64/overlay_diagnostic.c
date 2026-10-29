/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "overlay_platform.h"
#include "overlay_modules.h"
#include "native_dispatch.h"
static c64_overlay_io io={8,0,0,0,0,0};
static apollo_overlay_runtime overlays;
static executive executive_state;
static exec_native scheduler;
static uint16_t failures;
static uint8_t before_port,before_status;
static enum exec_action body(executive *state,void *user,exec_address *resume) {
    static const uint8_t modules[4]={1,1,2,1};
    static const uint16_t values[4]={12,15,34,39};
    static const uint16_t results[4]={0xa100,0xa101,0xb200,0xa100};
    unsigned index; uint16_t result=0; uint8_t status;
    (void)state; (void)user; (void)resume;
    for(index=0;index<4u;++index) {
        enum apollo_overlay_status loaded=apollo_overlay_call(&overlays,modules[index],(uint16_t)(index+2u),&result);
        ((volatile uint8_t *)0xc080)[7u+index]=(uint8_t)loaded;
        ((volatile uint8_t *)0xc090)[index]=io.last_error;
        if(loaded!=APOLLO_OVERLAY_OK)
            ++failures;
        if(result!=results[index] || overlays.state->words[0300]!=values[index]) ++failures;
    }
    if(overlays.state->words[0301]!=0 || overlays.state->words[0302]!=0) ++failures;
    /* Test cached and loading calls with BASIC mapped and I masked. Body code
     * deliberately lives under BASIC ROM at $a000 in these proof modules. */
    {
        uint8_t token,port=*(volatile uint8_t *)1;
        __asm__ volatile("php\nsei\npla" : "=a"(token) : : "memory");
        *(volatile uint8_t *)1=(uint8_t)(port|1u);
        if(apollo_overlay_call(&overlays,1,0,&result)!=APOLLO_OVERLAY_OK || result!=0xa101) ++failures;
        if(apollo_overlay_call(&overlays,2,0,&result)!=APOLLO_OVERLAY_OK || result!=0xb200) ++failures;
        __asm__ volatile("php\npla" : "=a"(status) : : "memory");
        if((status & 4u)==0 || *(volatile uint8_t *)1!=(uint8_t)(port|1u)) ++failures;
        *(volatile uint8_t *)1=port;
        __asm__ volatile("pha\nplp" : : "a"(token) : "memory");
    }
    __asm__ volatile("php\npla" : "=a"(status) : : "memory");
    if(*(volatile uint8_t *)1!=before_port || (status & 12u)!=(before_status & 12u)) ++failures;
    return EXEC_FINISH;
}
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc080;
    static const exec_mapping mapping[]={{{02000,0},body}};
    unsigned i;
    uint16_t result=0x4321;
    apollo_overlay_state *image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
    mail[0]=0; puts("APOLLO NATIVE DISK OVERLAYS");
    for(i=0;i<APOLLO_SHARED_WORDS;++i) image->words[i]=(uint16_t)(i^0x7376u);
    image->words[0300]=10;
    apollo_overlay_init(&overlays,overlay_modules,2,c64_overlay_backend(&io),
        (uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);
    before_port=*(volatile uint8_t *)1;
    __asm__ volatile("php\npla" : "=a"(before_status) : : "memory");
    exec_init(&executive_state);
    if(exec_admit(&executive_state,02000,02000,0,0)!=EXEC_OK ||
       exec_native_init(&scheduler,&executive_state,mapping,1,0)!=EXEC_OK ||
       exec_native_start(&scheduler)!=EXEC_OK || exec_native_step(&scheduler)!=EXEC_IDLE) ++failures;
    /* Only three designated source words may have changed across disk loads. */
    for(i=0;i<APOLLO_SHARED_WORDS;++i) if(i!=0300 && i!=0301 && i!=0302 && image->words[i]!=(uint16_t)(i^0x7376u)) ++failures;
    if(apollo_overlay_call(&overlays,99,0,&result)!=APOLLO_OVERLAY_UNMAPPED || result!=0x4321) ++failures;
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=6; mail[4]=(uint8_t)image->words[0300];
    mail[5]=(uint8_t)image->words[0301]; mail[6]=(uint8_t)image->words[0302];
    puts(failures ? "OVERLAY FAILURE" : "NATIVE DISK LOAD CALL RETURN PASS");
    mail[0]=0xa5; return failures;
}
