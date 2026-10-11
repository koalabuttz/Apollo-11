/* SPDX-License-Identifier: MIT */
#include "cm_irq_platform.h"
#define REG(a) (*(volatile uint8_t *)(a))
extern void cm_c64_irq_gate(void);
extern uint8_t cm_c64_irq_saved[32];
uint16_t cm_c64_irq_previous;
static cm_irq_queue *destination;
static cm_irq_request source;
static uint8_t old_mask,old_d011,installed;
static volatile uint8_t remaining;
volatile uint8_t cm_c64_irq_count,cm_c64_irq_done,cm_c64_irq_last_status,cm_c64_irq_poison;
uint8_t cm_c64_irq_stack[256];
volatile uint16_t cm_c64_irq_arithmetic;
volatile uint8_t cm_c64_irq_stack_fault;
static uint8_t enter(void *user) {
    uint8_t p;
    (void)user;
    __asm__ volatile("php\nsei\npla" : "=a"(p) : : "memory");
    return p;
}
static void leave(void *user,uint8_t p) {
    (void)user;
    if ((p & 4u)==0) __asm__ volatile("cli" : : : "memory");
    else __asm__ volatile("sei" : : : "memory");
}
const exec_exclusion cm_c64_exclusion={enter,leave,0};
uint8_t cm_c64_irq_status(void) {
    uint8_t p;
    __asm__ volatile("php\npla" : "=a"(p) : : "memory");
    return p;
}
int cm_c64_irq_install(cm_irq_queue *q,const cm_irq_request *request,uint8_t budget) {
    uint8_t token=enter(0);
    uint16_t previous=(uint16_t)(REG(0x0314) | ((uint16_t)REG(0x0315)<<8));
    if (installed != 0 || previous != 0xea31 || (REG(0xd01a) & 1u) != 0 ||
        q == 0 || request == 0 || budget == 0 || budget>5) {
        leave(0,token); return 0;
    }
    destination=q; source=*request; cm_c64_irq_previous=previous;
    old_mask=REG(0xd01a); old_d011=(uint8_t)(REG(0xd011) & 0x7fu);
    REG(0x0314)=(uint8_t)(uintptr_t)cm_c64_irq_gate;
    REG(0x0315)=(uint8_t)((uintptr_t)cm_c64_irq_gate>>8);
    installed=1; cm_c64_irq_poison=0; cm_c64_irq_arithmetic=0x99; cm_c64_irq_stack_fault=0;
    cm_c64_irq_arm(budget);
    leave(0,token); return 1;
}
void cm_c64_irq_arm(uint8_t budget) {
    uint8_t token=enter(0);
    if (installed != 0 && budget>0 && budget<=5) {
        REG(0xd01a)=(uint8_t)(REG(0xd01a) & 0xfeu);
        cm_c64_irq_count=0; cm_c64_irq_done=0; cm_c64_irq_last_status=0;
        remaining=budget;
        REG(0xd011)=(uint8_t)(REG(0xd011) & 0x7fu); REG(0xd012)=250;
        REG(0xd019)=1;
        REG(0xd01a)=(uint8_t)(REG(0xd01a) | 1u);
    }
    leave(0,token);
}
void cm_c64_irq_remove(void) {
    uint8_t token=enter(0);
    if (installed != 0) {
        REG(0xd01a)=old_mask; REG(0xd019)=1;
        REG(0xd011)=old_d011; /* Previous disabled raster comparator is not observable. */
        REG(0x0314)=(uint8_t)cm_c64_irq_previous; REG(0x0315)=(uint8_t)(cm_c64_irq_previous>>8);
        installed=0; remaining=0;
    }
    leave(0,token);
}
/* Called only by the assembly gate, with all 32 imaginary registers preserved,
 * D clear, I set, and an isolated 256-byte dynamic software stack. */
void cm_c64_irq_producer(void) {
    volatile uint8_t frame[16];
    uint8_t i;
    if (remaining != 0) {
        /* Exercise actual isolated dynamic storage, not only the SP register. */
        for(i=0;i<16;++i) frame[i]=(uint8_t)(i ^ 0xa6u);
        cm_c64_irq_arithmetic=(uint16_t)(cm_c64_irq_arithmetic+0x17u);
        cm_c64_irq_last_status=(uint8_t)cm_irq_post(destination,&source);
        for(i=0;i<16;++i)
            if(frame[i] != (uint8_t)(i ^ 0xa6u)) cm_c64_irq_stack_fault=1;
        ++cm_c64_irq_count; --remaining;
        if (remaining==0) {
            REG(0xd01a)=(uint8_t)(REG(0xd01a) & 0xfeu);
            cm_c64_irq_done=0x80;
        }
    }
}
