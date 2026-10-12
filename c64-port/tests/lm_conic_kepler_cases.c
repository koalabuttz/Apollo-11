/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "lm_conic_kepler_cases.h"
#define W m->words
/* Physical erasable words (E3/E4/E5 windows) of the inputs. */
enum {RRECT=01502,VRECT=01510,TAU=02073,XKEPNEW=02127,XPREV=01552,TC=01550};
/* A signed fraction q (|q| < 1) as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,double q){
 int32_t v;uint32_t a;agc_word hi,lo;
 if(q>=1.0)q=0.99999999;
 if(q<=-1.0)q=-0.99999999;
 v=(int32_t)(q*268435456.0);a=(uint32_t)(v<0?-v:v);hi=(agc_word)((a>>14)&037777u);lo=(agc_word)(a&037777u);
 if(v<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static double uniform(uint32_t *seed){return (double)(lm_conic_kepler_random(seed)>>8)/16777216.0;}
void lm_conic_kepler_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x4f524b50)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,moon=(unsigned)((n/8u)&1u),mode=(unsigned)((n/256u)%8u);
 lm_conic_kepler_debris(m,io,n);
 if((n/16u)%8u==7u){
  /* Raw edge words in the state vector, TAU., XKEPNEW, XPREV and TC. */
  for(i=0;i<6u;++i){W[RRECT+i]=edge[lm_conic_kepler_random(&seed)%8u];W[VRECT+i]=(agc_word)(lm_conic_kepler_random(&seed)&AGC_MASK);}
  for(i=0;i<2u;++i){W[TAU+i]=(agc_word)(lm_conic_kepler_random(&seed)&AGC_MASK);W[XKEPNEW+i]=edge[lm_conic_kepler_random(&seed)%8u];
   W[XPREV+i]=(agc_word)(lm_conic_kepler_random(&seed)&AGC_MASK);W[TC+i]=(agc_word)(lm_conic_kepler_random(&seed)&AGC_MASK);}
  if(n&64u)for(i=0;i<6u;i+=2u)W[VRECT+i]&=007777u;
  return;
 }
 {
  /* SI units: mu (m^3/cs^2), radius (m), speed (m/cs), time (cs). */
  double mu=moon?4.902778e8:3.986032e10,rs=moon?134217728.0:536870912.0,vs=moon?32.0:128.0,xs=moon?65536.0:131072.0;
  double r=moon?1738000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e7:uniform(&seed)*3.0e5):6378000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e8:uniform(&seed)*1.5e6);
  double vc=sqrt(mu/r),f,v,u[3],t[3],d,gam,a,period,tau,x,alpha;
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
  v=vc*f;gam=(uniform(&seed)-0.5)*(mode==3?0.0:1.2);
  for(i=0;i<3u;++i){put_dp(m,RRECT+2u*i,r*u[i]/rs);put_dp(m,VRECT+2u*i,v*(cos(gam)*t[i]+sin(gam)*u[i])/vs);}
  alpha=2.0/r-v*v/mu;a=alpha>1e-12?1.0/alpha:0.0;
  period=a>0.0?6.283185307179586*sqrt(a*a*a/mu):360000.0;
  switch((n/2048u)%8u){
  case 0:tau=(uniform(&seed)*2.4-1.2)*period;break;
  case 1:tau=(uniform(&seed)*10.0-5.0)*period;break;
  case 2:tau=(uniform(&seed)*0.02-0.01)*period;break;
  case 3:tau=0.0;break;
  default:tau=(uniform(&seed)-0.5)*period;break;
  }
  if(a<=0.0)tau=(uniform(&seed)-0.5)*720000.0;
  if(tau>2.6e8)tau=2.6e8;
  if(tau<-2.6e8)tau=-2.6e8;
  put_dp(m,TAU,tau/268435456.0);
  /* First-order universal anomaly sqrt(mu) tau / r, then the guess. */
  x=sqrt(mu)*tau/r;
  switch((n/16384u)%4u){
  case 0:x*=1.0+(uniform(&seed)-0.5)*0.02;break;
  case 1:x*=0.3+uniform(&seed)*1.4;break;
  case 2:x=(uniform(&seed)-0.5)*xs*(n&1u?0.2:1.6);break;
  default:x=n&2u?0.0:-x;break;
  }
  put_dp(m,XKEPNEW,x/xs);
  put_dp(m,XPREV,(n&4u)?x*(1.0+(uniform(&seed)-0.5)*0.1)/xs:(uniform(&seed)-0.5)*0.2);
  put_dp(m,TC,(tau*(0.9+uniform(&seed)*0.2))/268435456.0);
 }
}
