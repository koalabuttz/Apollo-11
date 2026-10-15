/* SPDX-License-Identifier: MIT */
/* lm_interbank_dap_hot_routines and lm_interbank_postjump_hot_routines
 * against lm_interbank_routines: for IBNKCALL, ISWRETRN and POSTJUMP,
 * random CPU states (most of them in the accepted context, Q erasable or
 * fixed, the CADR erasable or a compiled operand) must give the
 * same status, image and io record through either table. */
#include "lm_interbank_dap.h"
#include "lm_interbank_routines.h"
#include <stdio.h>
#include <string.h>
static uint32_t seed=12345u;
static uint16_t rnd(void){seed=seed*1103515245u+12345u;return (uint16_t)(seed>>12);}
static agc_routine find(const agc_routine_table *t,uint16_t physical){uint16_t i;
 for(i=0;i<t->count;++i){if(t->entries[i].physical==physical)return t->entries[i].call;}
 return 0;
}
#define SITES 64u
static agc_site_operand sites[SITES];
static void state(apollo_overlay_state *s,p66_bias_io *io,uint16_t z,uint16_t n){uint16_t i,r=rnd()%16u;
 for(i=0;i<APOLLO_SHARED_WORDS;++i)s->words[i]=rnd();
 s->words[0]=(uint16_t)(rnd()|(uint16_t)(rnd()<<1));s->words[1]=(uint16_t)(rnd()|(uint16_t)(rnd()<<1));
 s->words[3]=(uint16_t)((rnd()%8u)<<8);s->words[4]=(uint16_t)(rnd()&076000u);
 s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));s->words[7]=0;s->words[5]=z;
 io->superbank=(uint16_t)(rnd()&0160u);io->interrupt_enabled=rnd()%2u;
 /* Q: a compiled caller with an operand, a fixed one without, erasable or under EB. */
 if(r<6u)s->words[2]=(uint16_t)(02000u+sites[n%SITES].physical%1024u);
 else if(r<9u)s->words[2]=(uint16_t)(04000u+rnd()%04000u);
 else if(r<12u)s->words[2]=(uint16_t)(0100u+rnd()%01300u);
 else s->words[2]=(uint16_t)(01400u+rnd()%0400u);
 if(r<6u)s->words[4]=(uint16_t)((sites[n%SITES].physical/1024u)<<10),s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));
 /* BUF2 return address, mostly valid. */
 if(rnd()%8u)s->words[072]=(uint16_t)(0100u+rnd()%07700u);
 /* Now and then break one condition. */
 switch(rnd()%24u){case 0:s->words[7]=1;break;case 1:s->words[6]^=1;break;case 2:io->superbank=1;break;
  case 3:io->interrupt_enabled=2;break;case 4:s->words[3]=0100;break;case 5:s->words[2]=07776u;break;default:break;}
}
int main(void){agc_native_program p;apollo_overlay_state a,b;p66_bias_io ia,ib;uint32_t n,checks=0,fails=0,ok=0;uint16_t i,e;
 static const uint16_t z[3]={04674u,04707u,04635u};
 static const char *const name[3]={"IBNKCALL","ISWRETRN","POSTJUMP"};
 for(i=0;i<SITES;++i){sites[i].physical=(uint16_t)(4u*1024u+i*331u%(28u*1024u));sites[i].word=rnd()%2u?(uint16_t)(rnd()&077777u):(uint16_t)(rnd()|0100000u);}
 for(i=1;i<SITES;++i){agc_site_operand t=sites[i];uint16_t j=i;while(j&&sites[j-1u].physical>t.physical){sites[j]=sites[j-1u];--j;}sites[j]=t;}
 memset(&p,0,sizeof p);p.operands=sites;p.operand_count=SITES;
 for(n=0;n<200000u;++n)for(e=0;e<3u;++e){enum exec_status sa,sb;uint16_t key=(uint16_t)(2048u+(z[e]-04000u));
  state(&a,&ia,z[e],(uint16_t)n);b=a;ib=ia;
  sa=find(&lm_interbank_routines,key)(&p,&a,&ia);sb=find(e<2u?&lm_interbank_dap_hot_routines:&lm_interbank_postjump_hot_routines,key)(&p,&b,&ib);
  ++checks;if(sa==EXEC_OK)++ok;
  if(sa!=sb||memcmp(&a,&b,sizeof a)||memcmp(&ia,&ib,sizeof ia)){if(++fails<5)printf("FAIL: %s case %lu status %d vs %d\n",name[e],(unsigned long)n,(int)sa,(int)sb);}
 }
 printf("lm-interbank-dap: %lu checks (%lu returned), %lu failures\n",(unsigned long)checks,(unsigned long)ok,(unsigned long)fails);
 return fails!=0;
}
