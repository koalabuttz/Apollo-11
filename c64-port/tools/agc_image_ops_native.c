/* SPDX-License-Identifier: MIT */
/* Native side of the instruction differential: applies the named
 * agc_image_ops helper to each record (see agc_image_ops_reference.c) and
 * sets Z to the address the original instruction continues at. */
#include <stdio.h>
#include "agc_image_ops.h"
enum {CA,CS,AD,SU,MASK,TS,XCH,LXCH,QXCH,ADS,INCR,AUG,DIM,DAS,DCA,DCS,DXCH,MP,DV,CCS,BZF,BZMF,OPS};
int main(void){
 static agc_image s;uint16_t in[6+2048];
 while(fread(in,sizeof in,1,stdin)==1){
  unsigned i,k=in[1],next=0700u+in[2];
  for(i=0;i<2048u;++i)s.words[i]=in[6+i];
  for(i=0;i<in[2];++i)s.words[0700u+i]=in[3+i];
  switch(in[0]){
  case CA:ai_ca(&s,k);break;case CS:ai_cs(&s,k);break;case AD:ai_ad(&s,k);break;case SU:ai_su(&s,k);break;
  case MASK:ai_mask(&s,k);break;case TS:next+=ai_ts(&s,k);break;case XCH:ai_xch(&s,k);break;
  case LXCH:ai_lxch(&s,k);break;case QXCH:ai_qxch(&s,k);break;case ADS:ai_ads(&s,k);break;
  case INCR:ai_incr(&s,k);break;case AUG:ai_aug(&s,k);break;case DIM:ai_dim(&s,k);break;
  case DAS:ai_das(&s,k);break;case DCA:ai_dca(&s,k);break;case DCS:ai_dcs(&s,k);break;case DXCH:ai_dxch(&s,k);break;
  case MP:ai_mp(&s,k);break;case DV:ai_dv(&s,k);break;case CCS:next+=ai_ccs(&s,k);break;
  case BZF:if(ai_bzf(&s))next=k;ai_dca(&s,0);break;
  case BZMF:if(ai_bzmf(&s))next=k;ai_dca(&s,0);break;
  default:return 2;
  }
  s.words[5]=(agc_word)next;s.words[7]=0;
  if(fwrite(s.words,sizeof s.words,1,stdout)!=1)return 2;
 }
 return 0;
}
