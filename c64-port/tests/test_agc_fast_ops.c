/* SPDX-License-Identifier: MIT */
/* agc_fast_ops against agc_image_ops: every fast helper, by-value forms
 * included, on random images whose operand is a plain cell, compared
 * whole-image with the ai_* helper on a copy. MP and DV operands are drawn
 * mostly from edge values and the full 16-bit A/L range. Built for the host
 * (C cores) and for the 6502 (assembly cores, platform/c64/agc_fast_ops_diagnostic.c);
 * tools/agc_fast_ops_differential.py checks the same helpers against yaAGC. */
#include <stdio.h>
#include <string.h>
#include "agc_fast_ops.h"
#ifndef CASES
#define CASES 200000ul
#endif
#ifndef MULDIV_CASES
#define MULDIV_CASES 1000000ul
#endif
#include "agc_hardware_divide.h"
#if RK_FIXED_IMAGE
#define F (*AFO_S)
#else
static agc_image f;
#define F f
#endif
static agc_image g;
static uint32_t seed=0x1969u;
static uint16_t rnd(void){seed=seed*1103515245u+12345u;return (uint16_t)(seed>>12);}
static const uint16_t edge[]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,2,077775};
static uint16_t w15(void){return (rnd()&3u)?(uint16_t)(rnd()&077777u):edge[rnd()%12u];}
static uint16_t w16(void){uint16_t v=w15();return (rnd()&3u)?(uint16_t)(v|((v<<1)&0100000u)):rnd();}
enum {CA,CS,AD,SU,MASK,TS,XCH,LXCH,QXCH,ADS,INCR,AUG,DIM,DAS,DCA,DCS,DXCH,MP,DV,CCS,CAF,CSF,ADF,SUF,MASKF,MPF,DVF,CCSF,DCAF,DCSF,TC,BZF,BZMF,OPS};
#ifdef AFO_CHECK_QUIET
#define REPORT(...) ((void)0)
#else
#define REPORT(...) printf(__VA_ARGS__)
#endif
/* Failures over cases random helper cases and muldiv MP/DV cases. */
unsigned long afo_check(unsigned long cases,unsigned long muldiv);
unsigned long afo_check(unsigned long cases,unsigned long muldiv){
 unsigned long n,bad=0;unsigned i;
 AFO_BIND(&F);
 for(n=0;n<cases;++n){
  unsigned op=rnd()%OPS,k=040u+rnd()%01336u;uint16_t l,v[2];uint8_t x=0,y=0;afo_cell *p;
  for(i=0;i<16u;++i)F.words[i]=0;
  F.words[0]=w16();l=w16();F.words[1]=(uint16_t)((l&0137777u)|((l>>1)&040000u));F.words[2]=w15();
  F.words[3]=(uint16_t)((rnd()&7u)<<8);
  F.words[k]=w15();F.words[k+1u]=w15();v[0]=w15();v[1]=w15();
  memcpy(&g,&F,sizeof g);p=&F.words[k];
  switch(op){
  case CA:afo_ca(p);ai_ca(&g,k);break;case CS:afo_cs(p);ai_cs(&g,k);break;
  case AD:afo_ad(p);ai_ad(&g,k);break;case SU:afo_su(p);ai_su(&g,k);break;
  case MASK:afo_mask(p);ai_mask(&g,k);break;case TS:x=afo_ts(p);y=ai_ts(&g,k);break;
  case XCH:afo_xch(p);ai_xch(&g,k);break;case LXCH:afo_lxch(p);ai_lxch(&g,k);break;
  case QXCH:afo_qxch(p);ai_qxch(&g,k);break;case ADS:afo_ads(p);ai_ads(&g,k);break;
  case INCR:afo_incr(p);ai_incr(&g,k);break;case AUG:afo_aug(p);ai_aug(&g,k);break;
  case DIM:afo_dim(p);ai_dim(&g,k);break;case DAS:afo_das(p);ai_das(&g,k);break;
  case DCA:afo_dca(p);ai_dca(&g,k);break;case DCS:afo_dcs(p);ai_dcs(&g,k);break;
  case DXCH:afo_dxch(p);ai_dxch(&g,k);break;case MP:afo_mp(p);ai_mp(&g,k);break;
  case DV:afo_dv(p);ai_dv(&g,k);break;case CCS:x=afo_ccs(p);y=ai_ccs(&g,k);break;
  case CAF:afo_caf(v[0]);ai_caf(&g,v[0]);break;case CSF:afo_csf(v[0]);ai_csf(&g,v[0]);break;
  case ADF:afo_adf(v[0]);ai_adf(&g,v[0]);break;case SUF:afo_suf(v[0]);ai_suf(&g,v[0]);break;
  case MASKF:afo_maskf(v[0]);ai_maskf(&g,v[0]);break;case MPF:afo_mpf(v[0]);ai_mpf(&g,v[0]);break;
  case DVF:afo_dvf(v[0]);ai_dvf(&g,v[0]);break;case CCSF:x=afo_ccsf(v[0]);y=ai_ccsf(&g,v[0]);break;
  case DCAF:afo_dcaf(v);ai_dcaf(&g,v);break;case DCSF:afo_dcsf(v);ai_dcsf(&g,v);break;
  case TC:afo_tc(v[0]);ai_tc(&g,v[0]);break;
  case BZF:x=afo_bzf();y=ai_bzf(&g);break;default:x=afo_bzmf();y=ai_bzmf(&g);break;
  }
  if(x!=y||memcmp(&F,&g,sizeof g)){
   if(++bad<=10)REPORT("FAIL op %u k %o: A %06o/%06o L %06o/%06o Q %06o/%06o K %06o/%06o skip %u/%u\n",op,k,
    F.words[0],g.words[0],F.words[1],g.words[1],F.words[2],g.words[2],F.words[k],g.words[k],x,y);
  }
 }
 REPORT("agc_fast_ops: %lu cases, %lu failures\n",n,bad);
 /* MP and DV alone, registers only, against agc_multiply and
  * agc_hardware_divide (the cores the ai_* helpers use). */
 for(n=0;n<muldiv;++n){
  uint16_t a=w16(),l=w16(),k=w15();agc_register_pair want,in;
  l=(uint16_t)((l&0137777u)|((l>>1)&040000u));
  F.words[0]=a;F.words[1]=l;
  if(n&1u){afo_mpf(k);want=agc_multiply(a,k);want.l=(uint16_t)((want.l&0137777u)|((want.l>>1)&040000u));}
  else{in.a=a;in.l=l;afo_dvf(k);want=agc_hardware_divide(in,(agc_register)(k|((k<<1)&0100000u)));want.l=(uint16_t)((want.l&0137777u)|((want.l>>1)&040000u));}
  if(F.words[0]!=want.a||F.words[1]!=want.l){
   if(++bad<=20)REPORT("FAIL %s A %06o L %06o k %06o: %06o %06o want %06o %06o\n",(n&1u)?"MP":"DV",a,l,k,F.words[0],F.words[1],want.a,want.l);
  }
 }
 REPORT("agc_fast_ops MP/DV: %lu cases, %lu failures in all\n",n,bad);
 return bad;
}
#ifndef AFO_CHECK_NO_MAIN
int main(void){return afo_check(CASES,MULDIV_CASES)!=0;}
#endif
