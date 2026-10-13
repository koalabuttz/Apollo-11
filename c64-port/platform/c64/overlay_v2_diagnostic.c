/* SPDX-License-Identifier: MIT
 * ABI 2 target proof: REU module store, job and task slots, service table,
 * continuation actions and measured DMA swap cost, with interrupts enabled
 * through resident RAM vectors and KERNAL banked out ($01=$35). */
#include <stdio.h>
#include <stdint.h>
#include "overlay_reu.h"
#define LM 1u
#define ID(n) APOLLO_OV2_ID(APOLLO_LANE_PROOF,n)
extern void ov2_irq_entry(void);
extern void ov2_nmi_entry(void);
extern volatile uint16_t ov2_irq_count;
uint16_t c64_ov2_invoke(void *,uint8_t *,apollo_ov2_frame *,uint16_t);
static uint8_t record[APOLLO_REU_INDEX_RECORD],header[APOLLO_OV2_HEADER_BYTES];
static apollo_reu_store store;
static apollo_ov2_runtime rt;
static uint16_t failures;
static volatile uint8_t *const mail=(volatile uint8_t *)0xc080;

/* CIA2 timer A counts PHI2, timer B counts A underflows: 32-bit cycles. */
static void clock_start(void) {
    volatile uint8_t *cia=(volatile uint8_t *)0xdd00;
    cia[0x0d]=0x7f; cia[0x0e]=0; cia[0x0f]=0;
    cia[4]=0xff; cia[5]=0xff; cia[6]=0xff; cia[7]=0xff;
    cia[0x0f]=0x51; cia[0x0e]=0x11;
}
static uint32_t clock_stop(void) {
    volatile uint8_t *cia=(volatile uint8_t *)0xdd00;
    uint16_t a,b;
    cia[0x0e]=0; cia[0x0f]=0;
    a=(uint16_t)(cia[4]|(cia[5]<<8)); b=(uint16_t)(cia[6]|(cia[7]<<8));
    return ((uint32_t)(uint16_t)(0xffffu-b)<<16)+(uint16_t)(0xffffu-a);
}
static void put32(uint8_t at,uint32_t v) {
    mail[at]=(uint8_t)v; mail[at+1u]=(uint8_t)(v>>8); mail[at+2u]=(uint8_t)(v>>16);
}
static void expect(uint8_t ok,uint8_t step) {
    if(!ok) { ++failures; if(!mail[4]) mail[4]=step; }
}
static enum apollo_ov2_status run(uint8_t context,uint16_t module,uint16_t selector,
    uint16_t *value,uint8_t *action,apollo_ov2_cont *resume) {
    apollo_ov2_cont c; c.module=module; c.selector=selector;
    return apollo_ov2_run(&rt,context,c,value,action,resume);
}
int main(void) {
    apollo_overlay_state *image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
    uint16_t i,value,irq_before; uint8_t action,old_port,status;
    apollo_ov2_cont resume;
    for(i=0;i<0x20u;++i) mail[i]=0;
    puts("APOLLO OVERLAY ABI 2 REU PROOF");
    for(i=0;i<APOLLO_SHARED_WORDS;++i) image->words[i]=(uint16_t)(i^0x7376u);
    image->words[0300]=10; image->words[0301]=0; image->words[0302]=0; image->words[0303]=0;

    /* Enter the ABI 2 runtime configuration with resident RAM vectors. */
    old_port=*(volatile uint8_t *)1;
    __asm__ volatile("sei");
    *(volatile uint8_t *)1=0x35;
    *(volatile uint16_t *)0xfffe=(uint16_t)(uintptr_t)ov2_irq_entry;
    *(volatile uint16_t *)0xfffa=(uint16_t)(uintptr_t)ov2_nmi_entry;
    __asm__ volatile("cli");

    expect(apollo_reu_store_open(&store,0,LM,0x0202u,record),1);
    clock_start();
    expect(apollo_reu_store_verify(&store,(uint8_t *)0xc100)==APOLLO_OV2_OK,2);
    put32(0x10,clock_stop());
    apollo_ov2_init(&rt,apollo_reu_store_backend(&store,c64_ov2_invoke,0),
        (uint8_t *)0x8000,(uint8_t *)0xe000,header,image,LM,0x0202u,0);

    /* Resident service call from the job slot. */
    value=0;
    expect(run(0,ID(1),0,&value,&action,&resume)==APOLLO_OV2_OK && action==APOLLO_OV2_DONE && value==11,3);
    /* CALL into the task slot and RET back; both slots stay cached. */
    value=0;
    expect(run(1,ID(1),1,&value,&action,&resume)==APOLLO_OV2_OK && value==0x33 &&
        image->words[0301]==0x33 && image->words[0303]==1 &&
        rt.loaded[0]==ID(1) && rt.loaded[1]==ID(3) && rt.seams[1].depth==0,4);
    /* Historical address -> resolver -> large module -> kernel action. */
    expect(run(2,ID(1),3,&value,&action,&resume)==APOLLO_OV2_OK && action==0x21u &&
        resume.module==ID(1) && resume.selector==0 && image->words[0302]!=0,5);
    mail[0x18]=(uint8_t)image->words[0302]; mail[0x19]=(uint8_t)(image->words[0302]>>8);
    expect(apollo_ov2_run(&rt,2,resume,&value,&action,&resume)==APOLLO_OV2_OK &&
        action==APOLLO_OV2_DONE && value==12,6);

    /* Swap cost: cold near-full job module, then cached; cold task module. */
    apollo_ov2_restart(&rt);
    clock_start();
    expect(run(0,ID(2),1,&value,&action,&resume)==APOLLO_OV2_OK && value==0x5a00u,7);
    put32(0x08,clock_stop());
    clock_start();
    expect(run(0,ID(2),1,&value,&action,&resume)==APOLLO_OV2_OK && value==0x5a00u,8);
    put32(0x0b,clock_stop());
    clock_start();
    expect(run(8,ID(3),1,&value,&action,&resume)==APOLLO_OV2_OK && value==0x33,9);
    put32(0x13,clock_stop());
    mail[0x16]=(uint8_t)rt.store.lookup(rt.store.user,ID(2),&(apollo_ov2_dirent){0});

    /* Unmapped module faults and latches; restart clears. */
    expect(run(0,ID(9),0,&value,&action,&resume)==APOLLO_OV2_UNMAPPED,10);
    expect(run(0,ID(1),0,&value,&action,&resume)==APOLLO_OV2_FAULTED,11);
    apollo_ov2_restart(&rt);

    /* Interrupts kept arriving through the RAM vector while slots loaded. */
    irq_before=ov2_irq_count;
    for(i=0;i<2000u && ov2_irq_count==irq_before;++i) {}
    expect(ov2_irq_count!=irq_before && ov2_irq_count>2u,12);
    __asm__ volatile("php\npla" : "=a"(status) : : "memory");
    expect((status & 0x0cu)==0 && *(volatile uint8_t *)1==0x35,13);

    for(i=0;i<APOLLO_SHARED_WORDS;++i)
        if(i<0300u || i>0303u) expect(image->words[i]==(uint16_t)(i^0x7376u),14);
    mail[5]=(uint8_t)ov2_irq_count; mail[6]=(uint8_t)(ov2_irq_count>>8);

    __asm__ volatile("sei");
    *(volatile uint8_t *)1=old_port;
    __asm__ volatile("cli");
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8); mail[3]=2;
    puts(failures ? "ABI 2 FAILURE" : "ABI 2 REU PROOF PASS");
    mail[0]=0xa5;
    return failures;
}
