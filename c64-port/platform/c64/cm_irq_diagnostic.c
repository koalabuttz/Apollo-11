/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_irq_platform.h"
#include "cm_irq_cases.h"
#include "cm_irq_check.h"
#define MAIL ((volatile uint8_t *)0xc040)
#define REG(a) (*(volatile uint8_t *)(a))
static cm_executive state,before;
static unsigned failures;
static uint8_t mode;
static void check(int good) { if(!good) ++failures; }
static __attribute__((noinline)) void arithmetic(void) {
 uint8_t data[32],i; uint16_t k,sum=0; uint32_t h=UINT32_C(0x12345678);
 for(i=0;i<32;++i) data[i]=(uint8_t)(i*7u+3u);
 for(k=0;k<512;++k) { i=(uint8_t)(k&31u); h=((h<<5)|(h>>27)) ^ ((uint32_t)data[i]+(uint32_t)k*0x101u); data[i]^=(uint8_t)h; }
 for(i=0;i<32;++i) sum=(uint16_t)(sum+data[i]);
 check(h==UINT32_C(0xd71d5ead) && sum==4545);
}
static enum exec_status publish(cm_irq_queue *q,const cm_irq_request *req,void *user) {
 uint8_t installed; (void)user;
 check((cm_c64_irq_status()&4u)==0); before=state;
 installed=(uint8_t)cm_c64_irq_install(q,req,1); check(installed);
 if(!installed) return EXEC_OUT_OF_SCOPE;
 if(mode==0) { cm_c64_irq_poison=1; check(cm_c64_register_probe()==0); }
 else arithmetic();
 while(cm_c64_irq_done==0) __asm__ volatile("nop" : : : "memory");
 check(cm_c64_irq_count==1 && cm_c64_irq_last_status==EXEC_OK && cm_c64_irq_stack_fault==0);
 check(q->write==1 && q->read==0 && q->item[0].priority==req->priority && q->item[0].address.location==req->address.location && q->item[0].address.bank==req->address.bank && q->item[0].needs_vac==req->needs_vac);
 check(memcmp(&state,&before,sizeof state)==0); check((cm_c64_irq_status()&4u)==0);
 cm_c64_irq_remove(); check(*(volatile uint16_t *)0x0314==0xea31);
 return (enum exec_status)cm_c64_irq_last_status;
}
static enum cm_exec_action noop(cm_executive *s,void *u,exec_address *resume) { (void)s; (void)u; (void)resume; check((cm_c64_irq_status()&4u)==0); return CM_EXEC_FINISH; }
static void overflow(void) {
 static cm_irq_queue q; static cm_exec_native n; static cm_irq_runtime irq;
 static const cm_exec_mapping m[]={{{HIGH_ENTRY,HIGH_BANK},noop}};
 cm_irq_request req={03000,{HIGH_ENTRY,HIGH_BANK},1}; uint8_t installed,token,old_jiffy;
 cm_exec_init(&state); before=state; cm_irq_init(&q);
 check(cm_exec_native_init(&n,&state,m,1,0)==EXEC_OK); check(cm_irq_bind(&irq,&n,&q,&cm_c64_exclusion)==EXEC_OK);
 old_jiffy=REG(0x00a2); installed=(uint8_t)cm_c64_irq_install(&q,&req,5); check(installed); if(!installed) return;
 arithmetic(); while(cm_c64_irq_done==0) __asm__ volatile("nop" : : : "memory");
 check(cm_c64_irq_count==5 && cm_c64_irq_last_status==EXEC_QUEUE_FULL && q.write==4 && q.read==0 && q.overflow==1 && q.rejected==1);
 check(REG(0x00a2)!=old_jiffy && cm_c64_irq_stack_fault==0 && memcmp(&state,&before,sizeof state)==0);
 check(cm_irq_poll(&irq)==EXEC_QUEUE_FULL && memcmp(&state,&before,sizeof state)==0 && q.read==0);
 cm_c64_irq_remove(); token=cm_c64_exclusion.enter(0); check((cm_c64_irq_status()&4u)!=0);
 check(cm_irq_poll(&irq)==EXEC_QUEUE_FULL && (cm_c64_irq_status()&4u)!=0); cm_c64_exclusion.leave(0,token); check((cm_c64_irq_status()&4u)==0);
}
int main(void) {
 uint8_t outer,inner;
 MAIL[0]=0; puts("COMANCHE IRQ / SEVEN CORES");
 memset(cm_c64_irq_stack,0xcd,256);
 outer=cm_c64_exclusion.enter(0); inner=cm_c64_exclusion.enter(0);
 check((cm_c64_irq_status()&4u)!=0); cm_c64_exclusion.leave(0,inner); check((cm_c64_irq_status()&4u)!=0); cm_c64_exclusion.leave(0,outer); check((cm_c64_irq_status()&4u)==0);
 MAIL[5]=1;
 mode=0; failures+=cm_irq_check_sequence(&state,42,&cm_c64_exclusion,publish,0,1,0); MAIL[5]=2;
 mode=1; failures+=cm_irq_check_sequence(&state,42,&cm_c64_exclusion,publish,0,0,1); MAIL[5]=3;
 overflow(); MAIL[5]=4;
 check(*(volatile uint16_t *)0x0314==0xea31);
 MAIL[1]=(uint8_t)failures; MAIL[2]=(uint8_t)(failures>>8); MAIL[3]=7; MAIL[4]=0;
 puts(failures?"CM IRQ FAILED":"CM IRQ FULL STATE / ISR PASS"); MAIL[0]=0xa5; return failures;
}
