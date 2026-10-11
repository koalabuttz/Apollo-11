/* SPDX-License-Identifier: MIT */
#include "cm_irq_cases.h"
#include "cm_irq_check.h"
#include "cm_irq_vectors.h"
static uint16_t flatten(const cm_executive *s,uint16_t *w) {
 uint8_t i,j; uint16_t k=0;
 for(i=0;i<7;++i) for(j=0;j<12;++j) w[k++]=s->core[i][j];
 for(i=0;i<5;++i) for(j=0;j<44;++j) w[k++]=s->vac[i][j];
 w[k++]=s->newjob; w[k++]=s->newprio; w[k++]=s->newloc[0]; w[k++]=s->newloc[1];
 w[k++]=s->locctr; w[k++]=s->fixloc; w[k++]=s->ovfind; w[k++]=s->alarm; return k;
}
static uint32_t digest(const uint16_t *words,uint16_t count) {
 uint32_t h=UINT32_C(2166136261); uint16_t i;
 for(i=0;i<count;++i) { h=(h^(uint8_t)words[i])*UINT32_C(16777619); h=(h^(uint8_t)(words[i]>>8))*UINT32_C(16777619); } return h;
}
unsigned cm_irq_check_cases(const exec_exclusion *x) {
 uint16_t n,k; unsigned errors=0; uint8_t j; cm_scenario c; uint16_t words[313];
 for(n=0;n<512;++n) {
  words[0]=(uint16_t)cm_irq_case_run(&c,n,x); k=flatten(&c.state,words+1);
  if(k!=312 || digest(words,313)!=cm_irq_admission_hashes[n]) ++errors;
  for(j=0;j<16;++j) if(cm_irq_admission_indices[j]==n) for(k=0;k<313;++k) if(words[k]!=cm_irq_admission_full[j][k]) ++errors;
 } return errors;
}
typedef struct { unsigned errors; uint8_t n,observed; } checks;
static void observe(void *user,uint8_t step,const cm_executive *s,enum exec_status status,exec_address next) {
 checks *c=user; uint16_t words[315],k,index=(uint16_t)((c->n-42u)*8u+step);
 words[0]=(uint16_t)status; k=flatten(s,words+1); words[k+1]=next.location; words[k+2]=next.bank;
 if(k!=312 || digest(words,315)!=cm_irq_sequence_hashes[index]) ++c->errors;
 if(c->n==42) for(k=0;k<315;++k) if(words[k]!=cm_irq_sequence_full[step][k]) ++c->errors;
 ++c->observed;
}
unsigned cm_irq_check_sequence(cm_executive *s,uint8_t n,const exec_exclusion *x,cm_irq_publish publish,void *user,uint8_t manual,uint8_t typed) {
 checks c={0,n,0}; unsigned errors=cm_irq_sequence_run(s,n,x,publish,user,observe,&c,manual,typed);
 if(c.observed!=(manual!=0 ? 8 : 7)) ++c.errors;
 return errors+c.errors;
}
