/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "lm_conic_lambert_cases.h"
#define W m->words
/* Physical erasable words (E5) of the inputs. */
enum {R1VEC=02654,R2VEC=02662,TDESIRED=02670,GEOMSGN=02672,UN=02673,VTARGTAG=02701,COGA=02765};
/* A signed fraction q (|q| < 1) as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,double q){
 int32_t v;uint32_t a;agc_word hi,lo;
 if(q>=1.0)q=0.99999999;
 if(q<=-1.0)q=-0.99999999;
 v=(int32_t)(q*268435456.0);a=(uint32_t)(v<0?-v:v);hi=(agc_word)((a>>14)&037777u);lo=(agc_word)(a&037777u);
 if(v<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static double uniform(uint32_t *seed){return (double)(lm_conic_lambert_random(seed)>>8)/16777216.0;}
/* Stumpff functions and the universal-variable Kepler step: the position
 * dt after (r0, v0). 0 when the iteration does not settle. */
static double stc(double z){return z>1e-6?(1.0-cos(sqrt(z)))/z:z<-1e-6?(cosh(sqrt(-z))-1.0)/-z:0.5-z/24.0;}
static double sts(double z){double q;if(z>1e-6){q=sqrt(z);return (q-sin(q))/(q*q*q);}if(z<-1e-6){q=sqrt(-z);return (sinh(q)-q)/(q*q*q);}return 1.0/6.0-z/120.0;}
static int kepler(const double r0[3],const double v0[3],double mu,double dt,double r[3]){
 double rm=sqrt(r0[0]*r0[0]+r0[1]*r0[1]+r0[2]*r0[2]),rv=r0[0]*v0[0]+r0[1]*v0[1]+r0[2]*v0[2],sm=sqrt(mu);
 double alpha=2.0/rm-(v0[0]*v0[0]+v0[1]*v0[1]+v0[2]*v0[2])/mu,x=sm*fabs(alpha)*dt,z,f,g,F,dF;unsigned i;
 for(i=0;i<60u;++i){
  z=alpha*x*x;
  F=rv/sm*x*x*stc(z)+(1.0-alpha*rm)*x*x*x*sts(z)+rm*x-sm*dt;
  dF=rv/sm*x*(1.0-z*sts(z))+(1.0-alpha*rm)*x*x*stc(z)+rm;
  if(dF==0.0)return 0;
  x-=F/dF;
  if(fabs(F)<1e-9*sm*dt)break;
 }
 if(i==60u||x!=x)return 0;
 z=alpha*x*x;f=1.0-x*x/rm*stc(z);g=dt-x*x*x*sts(z)/sm;
 for(i=0;i<3u;++i)r[i]=f*r0[i]+g*v0[i];
 return 1;
}
void lm_conic_lambert_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x4c414d42)^((n/8u)*UINT32_C(0x85ebca6b));
 unsigned i,moon=(unsigned)((n/8u)&1u),mode=(unsigned)((n/256u)%8u);
 lm_conic_lambert_debris(m,io,n);
 for(i=0;i<8u;++i)(void)lm_conic_lambert_random(&seed);
 W[VTARGTAG]=(n/128u)%2u?0u:(agc_word)(lm_conic_lambert_random(&seed)&AGC_MASK);
 if((n/16u)%8u==7u){
  /* Raw edge words in the positions, time, sign, normal and guess. */
  for(i=0;i<6u;++i){W[R1VEC+i]=edge[lm_conic_lambert_random(&seed)%8u];W[R2VEC+i]=(agc_word)(lm_conic_lambert_random(&seed)&AGC_MASK);W[UN+i]=edge[lm_conic_lambert_random(&seed)%8u];}
  for(i=0;i<2u;++i){W[TDESIRED+i]=edge[lm_conic_lambert_random(&seed)%8u];W[COGA+i]=edge[lm_conic_lambert_random(&seed)%8u];}
  W[GEOMSGN]=edge[lm_conic_lambert_random(&seed)%8u];
  if(n&64u)for(i=0;i<6u;i+=2u)W[R2VEC+i]&=007777u;
  return;
 }
 {
  /* SI units: mu (m^3/cs^2), radius (m), speed (m/cs), time (cs). */
  double mu=moon?4.902778e8:3.986032e10,rs=moon?134217728.0:536870912.0;
  double r=moon?1738000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e7:uniform(&seed)*3.0e5):6378000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e8:uniform(&seed)*1.5e6);
  double vc=sqrt(mu/r),f,v,u[3],t[3],d,gam,en,per,dt,r1[3],v1[3],r2[3],h[3],sgn;
  for(;;){for(i=0;i<3u;++i)u[i]=uniform(&seed)*2.0-1.0;d=sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);if(d>0.1&&d<1.0)break;}
  for(i=0;i<3u;++i)u[i]/=d;
  for(;;){for(i=0;i<3u;++i)t[i]=uniform(&seed)*2.0-1.0;d=t[0]*u[0]+t[1]*u[1]+t[2]*u[2];for(i=0;i<3u;++i)t[i]-=d*u[i];d=sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]);if(d>0.1)break;}
  for(i=0;i<3u;++i)t[i]/=d;
  switch(mode){
  case 0:f=0.3+uniform(&seed)*1.1;break;
  case 1:f=1.45+uniform(&seed)*1.0;break;
  default:f=0.8+uniform(&seed)*0.4;break;
  }
  v=vc*f;gam=(uniform(&seed)-0.5)*1.0;
  for(i=0;i<3u;++i){r1[i]=r*u[i];v1[i]=v*(cos(gam)*t[i]+sin(gam)*u[i]);}
  h[0]=r1[1]*v1[2]-r1[2]*v1[1];h[1]=r1[2]*v1[0]-r1[0]*v1[2];h[2]=r1[0]*v1[1]-r1[1]*v1[0];
  en=v*v/2.0-mu/r;per=en<0.0?6.283185307179586*sqrt(pow(-mu/(2.0*en),3.0)/mu):6.283185307179586*sqrt(r*r*r/mu);
  /* The transfer time: within one revolution, near half of one, near a
   * whole one (the 360-degree edge), or short. */
  switch((n/2048u)%8u){
  case 0:dt=per*(0.48+uniform(&seed)*0.04);break;
  case 1:dt=per*(0.97+uniform(&seed)*0.025);break;
  case 2:dt=per*uniform(&seed)*0.05;break;
  default:dt=per*(0.02+uniform(&seed)*0.9);break;
  }
  if(dt*0.5>=268435456.0||!kepler(r1,v1,mu,dt,r2)){dt=uniform(&seed)*2.0e7;for(i=0;i<3u;++i)r2[i]=r1[i]*(1.0+uniform(&seed));}
  /* Sometimes the time does not belong to the positions, or the target is
   * the same direction (360LAMB). */
  if((n/32768u)&1u){
   if((n/4096u)%4u==0u)dt*=0.2+uniform(&seed)*3.0;
   else if((n/4096u)%4u==1u)for(i=0;i<3u;++i)r2[i]=r1[i]*(1.0+uniform(&seed)*0.5);
  }
  for(i=0;i<3u;++i){put_dp(m,R1VEC+2u*i,r1[i]/rs);put_dp(m,R2VEC+2u*i,r2[i]/rs);}
  put_dp(m,TDESIRED,dt/268435456.0);
  /* GEOMSGN: the sense of travel (+ when r1 x r2 lies along h), or either. */
  d=(r1[1]*r2[2]-r1[2]*r2[1])*h[0]+(r1[2]*r2[0]-r1[0]*r2[2])*h[1]+(r1[0]*r2[1]-r1[1]*r2[0])*h[2];
  sgn=(n/16u)%4u==3u?(uniform(&seed)<0.5?-1.0:1.0):d<0.0?-1.0:1.0;
  W[GEOMSGN]=sgn<0.0?(agc_word)(lm_conic_lambert_random(&seed)|040000u)&AGC_MASK:(agc_word)(lm_conic_lambert_random(&seed)&037777u);
  /* UN (B1): the orbit normal, as INITVEL's UNIT(RI) X VI leaves it. */
  d=sqrt(h[0]*h[0]+h[1]*h[1]+h[2]*h[2]);
  for(i=0;i<3u;++i)put_dp(m,UN+2u*i,h[i]/d/2.0);
  /* COGA (B5): the cotangent of the flight-path angle from the vertical,
   * about right or anywhere in range. */
  d=(r1[0]*v1[0]+r1[1]*v1[1]+r1[2]*v1[2])/(r*v*cos(gam));
  put_dp(m,COGA,(n/64u)%4u==0u?(uniform(&seed)-0.5)*1.9:d*(1.0+(uniform(&seed)-0.5)*0.2)/32.0);
 }
 if((n&2047u)==1717u){
  /* One case in 2048: a short transfer whose COGAMAX lands in
   * [COGUPLIM, 1) without overflow, so DSU BMN COGUPLIM falls through to
   * UPLIM (12,3301). The geometries (r1 along x, r2 in the xy plane, |r2| =
   * k|r1|, angle th) were found by searching with the native producer;
   * the oracle stream checks every one. GEOMSGN is positive. */
  static const double g[9][3]={{0.099184178495779637,0.78600198375061159,6.1501704494379084},
   {0.16514534384943544,2.9984175774268804,-0.098563667573349639},{0.1835265287477523,1.7913997971452773,6.1739943436019544},
   {0.50825697647407653,1.8899496045894921,6.1752416505234722},{0.43425944049842657,0.33158781398087739,6.1121048886674449},
   {0.26117806530557575,2.9566918222233651,6.1843507193900749},{0.15334549055434762,0.41077288817614316,-0.16000916917276187},
   {0.1735789668560028,2.3240143211558459,0.10347717865223108},{0.45832602724432947,1.5614628933370114,-0.11249529466818825}};
  const double *q=g[(n>>11)%9u];
  put_dp(m,R1VEC,q[0]);put_dp(m,R1VEC+2u,0.0);put_dp(m,R1VEC+4u,0.0);
  put_dp(m,R2VEC,q[0]*q[1]*cos(q[2]));put_dp(m,R2VEC+2u,q[0]*q[1]*sin(q[2]));put_dp(m,R2VEC+4u,0.0);
  W[GEOMSGN]=(agc_word)(W[GEOMSGN]&037777u);
 }
 if((n&2047u)==717u){
  /* One case in 2048: r1 and r2 almost parallel (|th| between 2^-13 and
   * 2^-19 rad). CSTH comes out one lsb above .5, STORE writes 1-CSTH as -0,
   * but ROUND carries the third word so BZE 360LAMB is not taken; LAMBLOOP
   * divides the stored -0 and BZE takes NEGP (12,3343). Found and checked
   * with the native producer; the oracle stream checks every one. */
  static const double z[8][3]={
   {0.056,0.69,-0.0001312255859375}, {0.07,0.8999999999999999,7.62939453125e-05},
   {0.096,1.29,-4.8065185546875e-05}, {0.112,1.5299999999999998,-2.70843505859375e-05},
   {0.08600000000000001,1.1400000000000001,1.10626220703125e-05}, {0.094,1.2599999999999998,5.91278076171875e-06},
   {0.062,0.78,2.193450927734375e-06}, {0.12200000000000001,1.6800000000000002,0.000115966796875}};
  const double *q=z[(n>>11)%8u];
  put_dp(m,R1VEC,q[0]);put_dp(m,R1VEC+2u,0.0);put_dp(m,R1VEC+4u,0.0);
  put_dp(m,R2VEC,q[0]*q[1]*cos(q[2]));put_dp(m,R2VEC+2u,q[0]*q[1]*sin(q[2]));put_dp(m,R2VEC+4u,0.0);
 }
}
