/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "lm_periapo_cases.h"
#define W m->words
/* Physical erasable words of the inputs. */
enum {MPAC=0154,RVEC=02654,VVEC=02743,RLS=02022};
/* A signed fraction q (|q| < 1) as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,double q){
 int32_t v;uint32_t a;agc_word hi,lo;
 if(q>=1.0)q=0.99999999;
 if(q<=-1.0)q=-0.99999999;
 v=(int32_t)(q*268435456.0);a=(uint32_t)(v<0?-v:v);hi=(agc_word)((a>>14)&037777u);lo=(agc_word)(a&037777u);
 if(v<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static double uniform(uint32_t *seed){return (double)(lm_periapo_random(seed)>>8)/16777216.0;}
/* A vector of three DP components at words at, at+2 and at+4. */
static void put_vec(apollo_overlay_state *m,unsigned at,const double v[3],double scale){
 unsigned i;for(i=0;i<3u;++i)put_dp(m,at+2u*i,v[i]/scale);
}
/* An Earth or Moon state vector: r (m), v (m/cs). */
static void orbit(uint32_t *seed,uint32_t n,unsigned moon,double r3[3],double v3[3]){
 double mu=moon?4.902778e8:3.986032e10;
 double r=moon?1738000.0+((n/128u)%4u==0u?uniform(seed)*1.0e7:uniform(seed)*3.0e5):6378000.0+((n/128u)%4u==0u?uniform(seed)*1.0e8:uniform(seed)*1.5e6);
 double vc=sqrt(mu/r),f,u[3],t[3],d,gam;unsigned i,mode=(unsigned)((n/512u)%8u);
 for(;;){for(i=0;i<3u;++i)u[i]=uniform(seed)*2.0-1.0;d=sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);if(d>0.1&&d<1.0)break;}
 for(i=0;i<3u;++i)u[i]/=d;
 for(;;){for(i=0;i<3u;++i)t[i]=uniform(seed)*2.0-1.0;d=t[0]*u[0]+t[1]*u[1]+t[2]*u[2];for(i=0;i<3u;++i)t[i]-=d*u[i];d=sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]);if(d>0.1)break;}
 for(i=0;i<3u;++i)t[i]/=d;
 gam=(uniform(seed)-0.5)*1.2;
 switch(mode){
 case 0:f=0.3+uniform(seed)*1.1;break;
 case 1:f=sqrt(2.0)*(1.0+(uniform(seed)-0.5)*1e-4);break;
 case 2:f=1.45+uniform(seed)*1.0;break;
 case 3:f=1.0+(uniform(seed)-0.5)*1e-5;gam=(uniform(seed)-0.5)*1e-5;break;
 default:f=0.6+uniform(seed)*0.75;break;
 }
 for(i=0;i<3u;++i){r3[i]=r*u[i];v3[i]=vc*f*(cos(gam)*t[i]+sin(gam)*u[i]);}
}
void lm_periapo_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x41504f47)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,kind=(unsigned)(n%8u),moon=(unsigned)((n/8u)&1u),vac,pd,raw=(n/64u)%8u==5u;
 double r3[3],v3[3],rl[3],d;
 lm_periapo_debris(m,io,n);
 vac=W[0120];pd=W[0166];
 /* RLS (E4,1422): a lunar landing site, B27 m. */
 for(;;){for(i=0;i<3u;++i)rl[i]=uniform(&seed)*2.0-1.0;d=sqrt(rl[0]*rl[0]+rl[1]*rl[1]+rl[2]*rl[2]);if(d>0.1&&d<1.0)break;}
 for(i=0;i<3u;++i)rl[i]*=1737000.0/d;
 put_vec(m,RLS,rl,134217728.0);
 orbit(&seed,n,moon,r3,v3);
 switch(kind){
 case 0:case 1:case 2:case 4:
  /* PERIAPO1, VECSHIFT: V in MPAC (B7 m/cs), R pushed (B29 m); the
   * routine rescales both by RTX2 for the Moon. */
  put_vec(m,MPAC,v3,128.0);put_vec(m,pd-6u,r3,536870912.0);
  if(raw)for(i=0;i<6u;++i){W[MPAC+i]=edge[lm_periapo_random(&seed)%8u];W[pd-6u+i]=(agc_word)(lm_periapo_random(&seed)&AGC_MASK);}
  break;
 case 3:
  /* PERIAPO: RVEC and VVEC already at the Earth (B29, B7) or Moon (B27,
   * B5) scale. */
  put_vec(m,RVEC,r3,moon?134217728.0:536870912.0);put_vec(m,VVEC,v3,moon?32.0:128.0);
  if(raw)for(i=0;i<6u;++i){W[RVEC+i]=edge[lm_periapo_random(&seed)%8u];W[VVEC+i]=(agc_word)(lm_periapo_random(&seed)&AGC_MASK);}
  break;
 case 5:case 6:{
  /* SHIFTR1 and MAXCHK: an altitude or radius in MPAC (B29 m), around
   * MAXNM (9999.9 NM) for MAXCHK. */
  double h=(n/64u)%4u==0u?18519814.8*(1.0+(uniform(&seed)-0.5)*1e-6):(uniform(&seed)*2.0-0.3)*(kind==6?2.5e7:8e7);
  put_dp(m,MPAC,h/536870912.0);
  if(raw){W[MPAC]=edge[lm_periapo_random(&seed)%8u];W[MPAC+1u]=(agc_word)(lm_periapo_random(&seed)&AGC_MASK);}
  break;}
 default:
  /* LEMSTORE/CSMSTORE: RATT and VATT (0D, 6D) from the precision
   * integration. */
  put_vec(m,vac,r3,536870912.0);put_vec(m,vac+6u,v3,128.0);
  break;
 }
}
