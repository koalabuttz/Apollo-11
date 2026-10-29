/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "lm_conic_timethet_cases.h"
#define W m->words
/* Physical erasable words (E5) of the inputs. */
enum {RVEC=02654,VVEC=02743,SNTH=02727,CSTH=02731};
/* A signed fraction q (|q| < 1) as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,double q){
 int32_t v;uint32_t a;agc_word hi,lo;
 if(q>=1.0)q=0.99999999;
 if(q<=-1.0)q=-0.99999999;
 v=(int32_t)(q*268435456.0);a=(uint32_t)(v<0?-v:v);hi=(agc_word)((a>>14)&037777u);lo=(agc_word)(a&037777u);
 if(v<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static double uniform(uint32_t *seed){return (double)(lm_conic_timethet_random(seed)>>8)/16777216.0;}
void lm_conic_timethet_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x54544854)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,moon=(unsigned)((n/8u)&1u),mode=(unsigned)((n/256u)%8u);
 lm_conic_timethet_debris(m,io,n);
 if((n&2047u)==1333u){
  /* Hyperbolic exits (VVEC 3x circular, R1A = -h^2 exactly) with theta
   * chosen so GETX leaves W = -h 2^-16 exactly: W^2 + R1A/16 is exactly
   * zero, WLOOP keeps W, and POLYCOEF's BMN takes INFINITY. Found by search
   * over the native translation, then confirmed by the oracle. */
  static const agc_word hit[4][8]={
   {067762,063310,016542,006206,076314,071463,016213,016650},
   {014004,032045,013523,032154,003412,007534,011472,012174},
   {065230,071665,023457,004545,076314,071463,016642,003056},
   {006461,014636,025247,024462,076314,071463,014223,002506}};
  const agc_word *h=hit[(n>>11)%4u];
  W[RVEC]=001142;W[RVEC+1u]=013200;
  for(i=2u;i<6u;++i)W[RVEC+i]=0;
  for(i=0;i<4u;++i)W[VVEC+i]=h[i];
  W[VVEC+4u]=0;W[VVEC+5u]=0;
  W[SNTH]=h[4];W[SNTH+1u]=h[5];W[CSTH]=h[6];W[CSTH+1u]=h[7];
  return;
 }
 if((n/16u)%8u==7u){
  /* Raw edge words in the state vector and the angle. */
  for(i=0;i<6u;++i){W[RVEC+i]=edge[lm_conic_timethet_random(&seed)%8u];W[VVEC+i]=(agc_word)(lm_conic_timethet_random(&seed)&AGC_MASK);}
  for(i=0;i<2u;++i){W[SNTH+i]=edge[lm_conic_timethet_random(&seed)%8u];W[CSTH+i]=(agc_word)(lm_conic_timethet_random(&seed)&AGC_MASK);}
  if(n&64u)for(i=0;i<6u;i+=2u)W[VVEC+i]&=007777u;
  return;
 }
 {
  /* SI units: mu (m^3/cs^2), radius (m), speed (m/cs). */
  double mu=moon?4.902778e8:3.986032e10,rs=moon?134217728.0:536870912.0,vs=moon?32.0:128.0;
  double r=moon?1738000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e7:uniform(&seed)*3.0e5):6378000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e8:uniform(&seed)*1.5e6);
  double vc=sqrt(mu/r),f,v,u[3],t[3],d,gam,th;
  for(;;){for(i=0;i<3u;++i)u[i]=uniform(&seed)*2.0-1.0;d=sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);if(d>0.1&&d<1.0)break;}
  for(i=0;i<3u;++i)u[i]/=d;
  for(;;){for(i=0;i<3u;++i)t[i]=uniform(&seed)*2.0-1.0;d=t[0]*u[0]+t[1]*u[1]+t[2]*u[2];for(i=0;i<3u;++i)t[i]-=d*u[i];d=sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]);if(d>0.1)break;}
  for(i=0;i<3u;++i)t[i]/=d;
  switch(mode){
  case 0:f=0.3+uniform(&seed)*1.1;break;
  case 1:f=sqrt(2.0)*(1.0+(uniform(&seed)-0.5)*1e-4);break;
  case 2:f=1.45+uniform(&seed)*1.0;break;
  default:f=0.6+uniform(&seed)*0.75;break;
  }
  v=vc*f;
  /* Flight-path angle; one case in sixteen almost radial (COGA overflow). */
  gam=(n/4096u)%16u==5u?(uniform(&seed)<0.5?1.0:-1.0)*(1.5707963-uniform(&seed)*0.002):(uniform(&seed)-0.5)*(mode==3?0.0:1.2);
  for(i=0;i<3u;++i){put_dp(m,RVEC+2u*i,r*u[i]/rs);put_dp(m,VVEC+2u*i,v*(cos(gam)*t[i]+sin(gam)*u[i])/vs);}
  /* Transfer angle theta (SNTH/CSTH at B1). */
  switch((n/2048u)%8u){
  case 0:th=uniform(&seed)*6.283185307179586;break;
  case 1:th=(uniform(&seed)-0.5)*0.02;break;
  case 2:th=6.283185307179586-uniform(&seed)*0.02;break;
  case 3:th=3.141592653589793+(uniform(&seed)-0.5)*0.02;break;
  case 4:th=uniform(&seed)*3.141592653589793;break;
  case 5:th=3.141592653589793+uniform(&seed)*3.141592653589793;break;
  default:th=uniform(&seed)*6.283185307179586;break;
  }
  put_dp(m,SNTH,sin(th)/2.0);put_dp(m,CSTH,cos(th)/2.0);
 }
}
