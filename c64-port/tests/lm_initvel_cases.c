/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "lm_initvel_cases.h"
#define W m->words
/* Physical erasable words of the inputs (E5 and E7), and the work-area
 * offsets of RATT1 and VATT1 (14D, 20D). */
enum {RATT1=016,VATT1=024,FLAGWRD1=075,R1VEC=02654,R2VEC=02662,TDESIRED=02670,UN=02673,VTARGTAG=02701,
 COGA=02765,RINIT=02322,VINIT=02330,VIPRIME=02336,RTARG=03443,DELLT4=03451,RTARG1=03471,INTIME=03607,
 ITCTR=03616,COZY4=03666};
/* A signed fraction q (|q| < 1) as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,double q){
 int32_t v;uint32_t a;agc_word hi,lo;
 if(q>=1.0)q=0.99999999;
 if(q<=-1.0)q=-0.99999999;
 v=(int32_t)(q*268435456.0);a=(uint32_t)(v<0?-v:v);hi=(agc_word)((a>>14)&037777u);lo=(agc_word)(a&037777u);
 if(v<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static double uniform(uint32_t *seed){return (double)(lm_initvel_random(seed)>>8)/16777216.0;}
/* A vector of three DP components at words at, at+2 and at+4. */
static void put_vec(apollo_overlay_state *m,unsigned at,const double v[3],double scale){
 unsigned i;for(i=0;i<3u;++i)put_dp(m,at+2u*i,v[i]/scale);
}
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
void lm_initvel_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x56454c4f)^((n/8u)*UINT32_C(0x85ebca6b));
 unsigned i,kind=(unsigned)(n%8u),moon=(unsigned)((n/8u)&1u),vac,raw=(n/64u)%8u==5u,count;
 /* SI units: mu (m^3/cs^2), radius (m), speed (m/cs), time (cs). */
 double mu=moon?4.902778e8:3.986032e10,rs=536870912.0,vs=128.0,r,vc,f,v,u[3],t[3],w[3],d,gam,en,per,dt,r1[3],v1[3],v2[3],r2[3];
 lm_initvel_debris(m,io,n);
 vac=W[0120];
 for(i=0;i<8u;++i)(void)lm_initvel_random(&seed);
 r=moon?1738000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e7:uniform(&seed)*3.0e5):6378000.0+((n/128u)%4u==0u?uniform(&seed)*1.0e8:uniform(&seed)*1.5e6);
 vc=sqrt(mu/r);
 for(;;){for(i=0;i<3u;++i)u[i]=uniform(&seed)*2.0-1.0;d=sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);if(d>0.1&&d<1.0)break;}
 for(i=0;i<3u;++i)u[i]/=d;
 for(;;){for(i=0;i<3u;++i)t[i]=uniform(&seed)*2.0-1.0;d=t[0]*u[0]+t[1]*u[1]+t[2]*u[2];for(i=0;i<3u;++i)t[i]-=d*u[i];d=sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]);if(d>0.1)break;}
 for(i=0;i<3u;++i)t[i]/=d;
 w[0]=u[1]*t[2]-u[2]*t[1];w[1]=u[2]*t[0]-u[0]*t[2];w[2]=u[0]*t[1]-u[1]*t[0];
 f=(n/512u)%4u==0u?0.3+uniform(&seed)*1.1:0.85+uniform(&seed)*0.3;
 v=vc*f;gam=(uniform(&seed)-0.5)*0.6;
 for(i=0;i<3u;++i){r1[i]=r*u[i];v1[i]=v*(cos(gam)*t[i]+sin(gam)*u[i]);}
 en=v*v/2.0-mu/r;per=en<0.0?6.283185307179586*sqrt(pow(-mu/(2.0*en),3.0)/mu):6.283185307179586*sqrt(r*r*r/mu);
 /* The transfer time: near half a revolution (the 180-degree cone), a
  * short one, or anywhere within one revolution. */
 switch((n/16u)%4u){
 case 0:dt=per*(0.47+uniform(&seed)*0.06);break;
 case 1:dt=per*uniform(&seed)*0.05;break;
 default:dt=per*(0.05+uniform(&seed)*0.85);break;
 }
 if(dt>1.3e8)dt=uniform(&seed)*1.3e8;
 /* The target: where a slightly different velocity (a burn of a few per
  * cent, partly out of plane) reaches after dt. */
 d=(uniform(&seed)-0.5)*0.1;
 for(i=0;i<3u;++i)v2[i]=v1[i]*(1.0+d)+v*(uniform(&seed)-0.5)*0.02*w[i];
 if(!kepler(r1,v2,mu,dt,r2))for(i=0;i<3u;++i)r2[i]=r1[i]*(0.5+uniform(&seed));
 for(i=0;i<3u;++i)if(fabs(r2[i])>=rs)r2[i]=r1[i];
 if(kind>=5u&&moon){rs/=4.0;vs/=4.0;}
 put_vec(m,RINIT,r1,rs);put_vec(m,VINIT,v1,vs);put_vec(m,RTARG,r2,rs);
 put_dp(m,DELLT4,dt/268435456.0);put_dp(m,INTIME,uniform(&seed)*0.9);
 /* 0D: the number of integration passes (zero for a conic transfer); 2D:
  * the cone angle E4 (revolutions). */
 count=kind==3u?1u+(n/32u)%3u:kind>=5u?1u+(n/64u)%3u:0u;
 W[vac]=(agc_word)count;W[vac+1u]=(agc_word)(lm_initvel_random(&seed)&AGC_MASK);
 put_dp(m,vac+2u,(n/256u)%4u==1u?uniform(&seed)*0.99:uniform(&seed)*0.1);
 switch(kind){
 case 4:
  /* HAVEGUES: GUESSW clear and a COGA guess (B5). */
  W[FLAGWRD1]&=(agc_word)~02u;
  d=(r1[0]*v1[0]+r1[1]*v1[1]+r1[2]*v1[2])/(r*v*cos(gam));
  put_dp(m,COGA,(n/64u)%4u==0u?(uniform(&seed)-0.5)*1.9:d*(1.0+(uniform(&seed)-0.5)*0.2)/32.0);
  break;
 case 5:
  /* INTSTALL's return: VIPRIME from LAMBERT. */
  put_vec(m,VIPRIME,v2,vs);
  break;
 case 6:case 7:
  /* INTEGRVS' return: the integrated RATT1/VATT1, the iteration state
  * (ITCTR, VTARGTAG, COZY4) and LAMBERT's inputs R1VEC, R2VEC, TDESIRED,
  * UN and VIPRIME; the last pass (ITCTR one short) or another. */
  W[VTARGTAG]=(agc_word)count;W[ITCTR]=(agc_word)(kind==6u?count-1u:(count>=2u?count-2u:077776u));
  put_vec(m,vac+RATT1,r2,rs);put_vec(m,vac+VATT1,v2,vs);put_vec(m,RTARG1,r2,rs);put_vec(m,R2VEC,r2,rs);put_vec(m,R1VEC,r1,rs);
  put_vec(m,VIPRIME,v2,vs);put_dp(m,TDESIRED,dt/268435456.0);put_dp(m,COZY4,(uniform(&seed)-0.5)*0.9);
  d=sqrt(w[0]*w[0]+w[1]*w[1]+w[2]*w[2]);
  for(i=0;i<3u;++i)put_dp(m,UN+2u*i,w[i]/d/2.0);
  break;
 default:break;
 }
 if(raw){
  /* Raw edge words in the positions, velocity, time and angle. */
  for(i=0;i<6u;++i){W[RINIT+i]=edge[lm_initvel_random(&seed)%8u];W[RTARG+i]=(agc_word)(lm_initvel_random(&seed)&AGC_MASK);W[VINIT+i]=edge[lm_initvel_random(&seed)%8u];}
  W[DELLT4]=edge[lm_initvel_random(&seed)%8u];W[vac+2u]=edge[lm_initvel_random(&seed)%8u];
  if(n&128u)for(i=0;i<6u;i+=2u)W[RTARG+i]&=007777u;
 }
}
