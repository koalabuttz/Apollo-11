/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "lm_conic_timerad_cases.h"
#define W m->words
/* Physical erasable words (E5) of the inputs. */
enum {RVEC=02654,VVEC=02743,SGNRDOT=02754,RDESIRED=02755};
/* A signed fraction q (|q| < 1) as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,double q){
 int32_t v;uint32_t a;agc_word hi,lo;
 if(q>=1.0)q=0.99999999;
 if(q<=-1.0)q=-0.99999999;
 v=(int32_t)(q*268435456.0);a=(uint32_t)(v<0?-v:v);hi=(agc_word)((a>>14)&037777u);lo=(agc_word)(a&037777u);
 if(v<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static double uniform(uint32_t *seed){return (double)(lm_conic_timerad_random(seed)>>8)/16777216.0;}
void lm_conic_timerad_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x52414454)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,moon=(unsigned)((n/8u)&1u),mode=(unsigned)((n/256u)%8u);
 lm_conic_timerad_debris(m,io,n);
 /* SGNRDOT: the sign of the radial rate wanted (CSI leaves it as is). */
 W[SGNRDOT]=(n/2u)%4u==0u?(agc_word)(lm_conic_timerad_random(&seed)&AGC_MASK):(n/2u)%4u<3u?1u:077776u;
 if((n/16u)%8u==7u){
  /* Raw edge words in the state vector and RDESIRED. */
  for(i=0;i<6u;++i){W[RVEC+i]=edge[lm_conic_timerad_random(&seed)%8u];W[VVEC+i]=(agc_word)(lm_conic_timerad_random(&seed)&AGC_MASK);}
  W[RDESIRED]=edge[lm_conic_timerad_random(&seed)%8u];W[RDESIRED+1u]=(agc_word)(lm_conic_timerad_random(&seed)&AGC_MASK);
  if(n&64u)for(i=0;i<6u;i+=2u)W[VVEC+i]&=007777u;
  return;
 }
 {
  /* SI units: mu (m^3/cs^2), radius (m), speed (m/cs). */
  double mu=moon?4.902778e8:3.986032e10,rs=moon?134217728.0:536870912.0,vs=moon?32.0:128.0;
  double r=moon?1738000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e7:uniform(&seed)*3.0e5):6378000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e8:uniform(&seed)*1.5e6);
  double vc=sqrt(mu/r),f,v,u[3],t[3],d,gam,en,h,e,a,rp,ra,rd;
  for(;;){for(i=0;i<3u;++i)u[i]=uniform(&seed)*2.0-1.0;d=sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);if(d>0.1&&d<1.0)break;}
  for(i=0;i<3u;++i)u[i]/=d;
  for(;;){for(i=0;i<3u;++i)t[i]=uniform(&seed)*2.0-1.0;d=t[0]*u[0]+t[1]*u[1]+t[2]*u[2];for(i=0;i<3u;++i)t[i]-=d*u[i];d=sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]);if(d>0.1)break;}
  for(i=0;i<3u;++i)t[i]/=d;
  gam=(uniform(&seed)-0.5)*1.2;
  switch(mode){
  case 0:f=0.3+uniform(&seed)*1.1;break;
  case 1:f=sqrt(2.0)*(1.0+(uniform(&seed)-0.5)*1e-4);break;
  case 2:f=1.45+uniform(&seed)*1.0;break;
  /* Circular or almost: CIRCULAR's UNIT overflow, and the edge of it. */
  case 3:f=1.0+(uniform(&seed)-0.5)*((n/4096u)%2u?1e-3:1e-7);gam=(uniform(&seed)-0.5)*((n/4096u)%2u?1e-3:1e-7);break;
  default:f=0.6+uniform(&seed)*0.75;break;
  }
  v=vc*f;
  for(i=0;i<3u;++i){put_dp(m,RVEC+2u*i,r*u[i]/rs);put_dp(m,VVEC+2u*i,v*(cos(gam)*t[i]+sin(gam)*u[i])/vs);}
  /* Orbit radii for RDESIRED: inside, at and outside [rp, ra]. */
  en=v*v/2.0-mu/r;h=r*v*cos(gam);e=sqrt(fmax(0.0,1.0+2.0*en*h*h/(mu*mu)));
  a=-mu/(2.0*en);rp=h*h/(mu*(1.0+e));ra=en<0.0?a*(1.0+e):4.0*r;
  switch((n/2048u)%8u){
  case 0:case 1:rd=-1.0;break;
  case 2:rd=rp+(ra-rp)*uniform(&seed);break;
  case 3:rd=rp*(1.0+(uniform(&seed)-0.5)*2e-6);break;
  case 4:rd=ra*(1.0+(uniform(&seed)-0.5)*2e-6);break;
  case 5:rd=uniform(&seed)<0.5?rp*(0.5+uniform(&seed)*0.5):ra*(1.0+uniform(&seed));break;
  case 6:rd=r*(1.0+(uniform(&seed)-0.5)*1e-3);break;
  default:rd=rp+(ra-rp)*uniform(&seed);break;
  }
  /* P32/P72 CSI passes DPPOSMAX (the apocenter time). */
  if(rd<0.0){W[RDESIRED]=037777;W[RDESIRED+1u]=037777;}else put_dp(m,RDESIRED,rd/rs);
 }
}
