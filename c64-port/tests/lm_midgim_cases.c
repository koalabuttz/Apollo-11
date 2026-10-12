/* SPDX-License-Identifier: MIT */
#include <math.h>
#include "lm_midgim_cases.h"
#define W m->words
/* Physical erasable words of the inputs. */
enum {REFSMMAT=01733,RINIT=02322,VINIT=02330};
/* A signed fraction q (|q| < 1) as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,double q){
 int32_t v;uint32_t a;agc_word hi,lo;
 if(q>=1.0)q=0.99999999;
 if(q<=-1.0)q=-0.99999999;
 v=(int32_t)(q*268435456.0);a=(uint32_t)(v<0?-v:v);hi=(agc_word)((a>>14)&037777u);lo=(agc_word)(a&037777u);
 if(v<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static double uniform(uint32_t *seed){return (double)(lm_midgim_random(seed)>>8)/16777216.0;}
static void put_vec(apollo_overlay_state *m,unsigned at,const double v[3],double scale){
 unsigned i;for(i=0;i<3u;++i)put_dp(m,at+2u*i,v[i]/scale);
}
/* A random direction scaled to length r. */
static void direction(uint32_t *seed,double r,double u[3]){
 double d;unsigned i;
 for(;;){for(i=0;i<3u;++i)u[i]=uniform(seed)*2.0-1.0;d=sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);if(d>0.1&&d<1.0)break;}
 for(i=0;i<3u;++i)u[i]*=r/d;
}
void lm_midgim_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x4c564347)^((n/8u)*UINT32_C(0x85ebca6b));
 unsigned i,j,kind=(unsigned)(n%8u),vac,pd,raw=(n/64u)%8u==5u;
 double a[3],b[3],c[3],d,v[3],r[3];
 lm_midgim_debris(m,io,n);
 vac=W[0120];pd=W[0166];
 for(i=0;i<8u;++i)(void)lm_midgim_random(&seed);
 if(kind<3u){
  /* GET+MGA: the velocity pushed at PUSHLOC-6 (B7 m/cs) and REFSMMAT, a
   * rotation (B1), sometimes one whose Y row lies along the velocity. */
  direction(&seed,1.0,a);direction(&seed,1.0,b);
  d=a[0]*b[0]+a[1]*b[1]+a[2]*b[2];for(i=0;i<3u;++i)b[i]-=d*a[i];
  d=sqrt(b[0]*b[0]+b[1]*b[1]+b[2]*b[2]);for(i=0;i<3u;++i)b[i]/=d;
  c[0]=a[1]*b[2]-a[2]*b[1];c[1]=a[2]*b[0]-a[0]*b[2];c[2]=a[0]*b[1]-a[1]*b[0];
  put_vec(m,REFSMMAT,a,2.0);put_vec(m,REFSMMAT+6u,b,2.0);put_vec(m,REFSMMAT+12u,c,2.0);
  direction(&seed,100.0+uniform(&seed)*1000.0,v);
  if((n/16u)%8u==1u)for(i=0;i<3u;++i)v[i]=b[i]*(n&256u?-500.0:500.0);
  put_vec(m,pd-6u,v,128.0);
  if(raw)for(i=0;i<6u;++i){W[pd-6u+i]=edge[lm_midgim_random(&seed)%8u];W[REFSMMAT+6u+i]=(agc_word)(lm_midgim_random(&seed)&AGC_MASK);}
 }else if(kind<6u){
  /* GET.LVC: RINIT (B29 m), VINIT (B7 m/cs), delta V at 0D (B7). */
  direction(&seed,(n/16u)%2u?1.75e6+uniform(&seed)*2e5:6.4e6+uniform(&seed)*1e6,r);
  direction(&seed,(n/16u)%2u?160.0:760.0,v);
  put_vec(m,RINIT,r,536870912.0);put_vec(m,VINIT,v,128.0);
  direction(&seed,uniform(&seed)*30.0,a);put_vec(m,vac,a,128.0);
  if(raw)for(j=0;j<6u;++j){W[RINIT+j]=edge[lm_midgim_random(&seed)%8u];W[VINIT+j]=edge[lm_midgim_random(&seed)%8u];}
 }
}
