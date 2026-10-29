/* SPDX-License-Identifier: MIT */
#include "timer_platform.h"
#define REG(a) (*(volatile uint8_t *)(a))
extern void c64_timer_gate(void);
uint16_t c64_timer_previous;
static wl_clock *destination;
static uint8_t installed,remaining,finite,step;
volatile uint8_t c64_timer_count,c64_timer_done,c64_timer_fault;
volatile uint8_t c64_timer_poison,c64_timer_stack_fault;
uint8_t c64_timer_stack[256];
static uint8_t enter(void *user) {
    uint8_t p; (void)user;
    __asm__ volatile("php\nsei\npla" : "=a"(p) : : "memory"); return p;
}
static void leave(void *user,uint8_t p) {
    (void)user;
    if((p & 4u)==0) __asm__ volatile("cli" : : : "memory");
    else __asm__ volatile("sei" : : : "memory");
}
const exec_exclusion c64_timer_exclusion={enter,leave,0};
uint8_t c64_timer_status(void) {
    uint8_t p; __asm__ volatile("php\npla" : "=a"(p) : : "memory"); return p;
}
int c64_timer_install(wl_clock *clock,uint16_t period,uint8_t tick_step,uint8_t budget) {
    uint8_t token=enter(0);
    uint16_t previous=(uint16_t)(REG(0x0314) | ((uint16_t)REG(0x0315)<<8));
    if(installed!=0 || previous!=0xea31 || clock==0 || period<1000 || tick_step==0 || tick_step>2 ||
       REG(0xdc0f)!=8 || REG(0xdc06)!=0xff || REG(0xdc07)!=0xff ||
       (REG(0xd01a) & 1u)!=0) { leave(0,token); return 0; }
    destination=clock; c64_timer_previous=previous;
    step=tick_step; remaining=budget; finite=(uint8_t)(budget!=0);
    c64_timer_poison=0; c64_timer_stack_fault=0;
    c64_timer_count=0; c64_timer_done=0; c64_timer_fault=0;
    REG(0x0314)=(uint8_t)(uintptr_t)c64_timer_gate;
    REG(0x0315)=(uint8_t)((uintptr_t)c64_timer_gate>>8);
    REG(0xdc06)=(uint8_t)(period-1u); REG(0xdc07)=(uint8_t)((period-1u)>>8);
    installed=1; REG(0xdc0d)=0x82; REG(0xdc0f)=0x11;
    leave(0,token); return 1;
}
void c64_timer_remove(void) {
    uint8_t token=enter(0);
    if(installed!=0) {
        REG(0xdc0f)=0; REG(0xdc0d)=2; /* Clear B mask without changing A ownership. */
        REG(0xdc06)=0xff; REG(0xdc07)=0xff;
        REG(0xdc0f)=0x18; REG(0xdc0f)=8;
        REG(0x0314)=(uint8_t)c64_timer_previous; REG(0x0315)=(uint8_t)(c64_timer_previous>>8);
        installed=0;
    }
    leave(0,token);
}
void c64_timer_producer(void) {
    volatile uint8_t frame[16];
    uint8_t i;
    for(i=0;i<16;++i) frame[i]=(uint8_t)(i ^ 0xa6u);
    for(i=0;i<step;++i) if(wl_clock_post(destination)!=WL_OK) c64_timer_fault=1;
    for(i=0;i<16;++i) if(frame[i]!=(uint8_t)(i ^ 0xa6u)) c64_timer_stack_fault=1;
    if(c64_timer_count!=255) ++c64_timer_count;
    if(c64_timer_fault!=0 || (finite!=0 && --remaining==0)) {
        REG(0xdc0f)=0; REG(0xdc0d)=2; c64_timer_done=0x80;
    }
}
