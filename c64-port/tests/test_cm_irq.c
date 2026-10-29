/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_irq_cases.h"
#include "cm_irq_check.h"
static unsigned errors;
#define CHECK(c) do { if(!(c)) { fprintf(stderr,"CM IRQ failure line%d: %s\n",__LINE__,#c); ++errors; } } while(0)
static uint8_t masked; static unsigned enters,leaves;
static uint8_t enter(void *user) { uint8_t old=masked; (void)user; masked=1; ++enters; return old; }
static void leave(void *user,uint8_t old) { (void)user; CHECK(masked==1); masked=old; ++leaves; }
static const exec_exclusion x={enter,leave,0};
static enum exec_status post(cm_irq_queue *q,const cm_irq_request *r,void *user) { uint8_t token; (void)user; CHECK(masked==0); token=enter(0); CHECK(q->read==0 && q->write==0); CHECK(cm_irq_post(q,r)==EXEC_OK); leave(0,token); return EXEC_OK; }
static enum cm_exec_action noop(cm_executive *s,void *u,exec_address *a) { (void)s; (void)u; (void)a; return CM_EXEC_FINISH; }
static enum cm_exec_action failing(cm_executive *s,void *user,exec_address *resume) {
 cm_irq_runtime *r=user; exec_address a={02000,06003}; (void)s; (void)resume;
 CHECK(masked==0); CHECK(cm_irq_admit(r,03000,a,1)==EXEC_NO_CORE);
 return CM_EXEC_FINISH; /* Latched helper fault blocks ENDJOB and exchange. */
}
int main(void) {
 cm_executive s,before; cm_exec_native native; cm_irq_queue q; cm_irq_runtime irq;
 cm_irq_request req={02000,{02000,06003},0}; const cm_exec_mapping maps[]={{{02000,06003},noop}};
 uint16_t n; uint8_t manual,typed,token,inner;
 errors+=cm_irq_check_cases(&x);
 for(n=42;n<50;++n) for(manual=0;manual<2;++manual) for(typed=0;typed<2;++typed) errors+=cm_irq_check_sequence(&s,(uint8_t)n,&x,post,0,manual,typed);
 token=enter(0); inner=enter(0); leave(0,inner); CHECK(masked==1); leave(0,token); CHECK(masked==0);
 cm_exec_init(&s); cm_irq_init(&q); before=s; CHECK(cm_exec_native_init(&native,&s,maps,1,0)==EXEC_OK); CHECK(cm_irq_bind(&irq,&native,&q,&x)==EXEC_OK);
 CHECK(cm_irq_bind(&irq,&native,&q,&x)==EXEC_OUT_OF_SCOPE);
 for(n=0;n<4;++n) CHECK(cm_irq_post(&q,&req)==EXEC_OK);
 CHECK(cm_irq_post(&q,&req)==EXEC_QUEUE_FULL && q.write==4 && q.read==0 && q.rejected==1);
 CHECK(cm_irq_poll(&irq)==EXEC_QUEUE_FULL && memcmp(&s,&before,sizeof s)==0);
 for(n=0;n<300;++n) { CHECK(cm_irq_post(&q,&req)==EXEC_QUEUE_FULL); }
 CHECK(q.rejected==255);
 cm_exec_init(&s); cm_irq_init(&q); before=s; CHECK(cm_exec_native_init(&native,&s,maps,1,0)==EXEC_OK); CHECK(cm_irq_bind(&irq,&native,&q,&x)==EXEC_OK);
 req.address.location=02001; CHECK(cm_irq_post(&q,&req)==EXEC_OK); CHECK(cm_irq_poll(&irq)==EXEC_UNMAPPED && q.read==0 && memcmp(&s,&before,sizeof s)==0);
 cm_irq_init(&q); req.address.location=02000; req.address.bank=010; CHECK(cm_irq_post(&q,&req)==EXEC_OUT_OF_SCOPE && q.write==0 && q.overflow==0);
 req.address.bank=06003; req.priority=0; CHECK(cm_irq_post(&q,&req)==EXEC_OUT_OF_SCOPE && q.write==0);
 CHECK(cm_irq_post(0,&req)==EXEC_OUT_OF_SCOPE && cm_irq_post(&q,0)==EXEC_OUT_OF_SCOPE);
 {
  const cm_exec_mapping f[]={{{02000,06003},failing}};
  cm_exec_init(&s); cm_irq_init(&q); CHECK(cm_exec_native_init(&native,&s,f,1,&irq)==EXEC_OK); CHECK(cm_irq_bind(&irq,&native,&q,&x)==EXEC_OK);
  req.priority=02000; req.address.location=02000; req.address.bank=06003;
  CHECK(cm_irq_admit(&irq,02000,req.address,0)==EXEC_OK && cm_irq_start(&irq)==EXEC_OK);
  for(n=0;n<6;++n) CHECK(cm_irq_admit(&irq,02000,req.address,0)==EXEC_OK);
  before=s; CHECK(cm_irq_step(&irq)==EXEC_NO_CORE && native.running==0 && native.fault==EXEC_NO_CORE);
  CHECK(s.alarm==EXEC_NO_CORE && s.core[0][CM_EXEC_PRIORITY]==before.core[0][CM_EXEC_PRIORITY] && s.core[0][CM_EXEC_PUSH]==before.core[0][CM_EXEC_PUSH]);
  CHECK(s.vac[0][0]==0 && masked==0);
 }
 masked=1; errors+=cm_irq_check_cases(&x); CHECK(masked==1); masked=0;
 CHECK(enters==leaves); if(errors!=0) return 1;
 puts("PASS: CM IRQ: 512 source admissions, 32 actual callback sequences / 240 checkpoints, FIFO wrap, overflow, retained head, typed maps and restoring guards"); return 0;
}
