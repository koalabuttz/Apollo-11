/* SPDX-License-Identifier: MIT */
/* Fast side of the agc_fast_ops differential: the records of
 * agc_image_ops_differential.py, applied with the afo_* helper whenever the
 * operand (and its low word, for the double forms) is a plain cell under
 * the record's EB, else with the ai_* helper, as AFO() picks at compile
 * time. Odd records use the by-value *f form on the cell's value where one
 * exists, so those forms meet yaAGC too. */
#include <stdio.h>
#include "agc_fast_ops.h"
enum {CA,CS,AD,SU,MASK,TS,XCH,LXCH,QXCH,ADS,INCR,AUG,DIM,DAS,DCA,DCS,DXCH,MP,DV,CCS,BZF,BZMF,OPS};
static agc_image s;
int main(void){
 uint16_t in[6+2048];unsigned long n=0,fast=0;
 AFO_BIND(&s);
 while(fread(in,sizeof in,1,stdin)==1){
  unsigned i,k=in[1],next=0700u+in[2],eb=(s.words[3]>>8)&7u,ea;uint8_t plain,plain2,f=(uint8_t)(n++&1u);afo_cell *p;
  for(i=0;i<2048u;++i)s.words[i]=in[6+i];
  for(i=0;i<in[2];++i)s.words[0700u+i]=in[3+i];
  eb=(s.words[3]>>8)&7u;ea=AFO_EA(eb,k);p=&s.words[ea&03777u];
  plain=k<02000u&&AFO_PLAINX(ea);plain2=plain&&k!=01377u&&k!=01777u&&AFO_PLAINX(ea+1u);
  if(in[0]==DAS||in[0]==DCA||in[0]==DCS||in[0]==DXCH)plain=plain2;
  fast+=plain;
#define F1(op,lo,fn) case op:if(!plain)ai_##lo(&s,k);else if(f)afo_##fn((agc_word)(*p&077777u));else afo_##lo(p);break;
#define F0(op,lo) case op:if(plain)afo_##lo(p);else ai_##lo(&s,k);break;
  switch(in[0]){
  F1(CA,ca,caf) F1(CS,cs,csf) F1(AD,ad,adf) F1(SU,su,suf) F1(MASK,mask,maskf)
  case TS:next+=plain?afo_ts(p):ai_ts(&s,k);break;
  F0(XCH,xch) F0(LXCH,lxch) F0(QXCH,qxch) F0(ADS,ads) F0(INCR,incr) F0(AUG,aug) F0(DIM,dim) F0(DAS,das)
  case DCA:if(!plain)ai_dca(&s,k);else if(f){agc_word v[2];v[0]=p[0];v[1]=p[1];afo_dcaf(v);}else afo_dca(p);break;
  case DCS:if(!plain)ai_dcs(&s,k);else if(f){agc_word v[2];v[0]=p[0];v[1]=p[1];afo_dcsf(v);}else afo_dcs(p);break;
  F0(DXCH,dxch) F1(MP,mp,mpf) F1(DV,dv,dvf)
  case CCS:next+=!plain?ai_ccs(&s,k):f?afo_ccsf(*p):afo_ccs(p);break;
  case BZF:if(afo_bzf())next=k;ai_dca(&s,0);break;
  case BZMF:if(afo_bzmf())next=k;ai_dca(&s,0);break;
  default:return 2;
  }
  s.words[5]=(agc_word)next;s.words[7]=0;s.words[2]&=077777u; /* rows hold Q in 15 bits */
  if(fwrite(s.words,sizeof s.words,1,stdout)!=1)return 2;
 }
 fprintf(stderr,"%lu records, %lu on the fast path\n",n,fast);
 return 0;
}
