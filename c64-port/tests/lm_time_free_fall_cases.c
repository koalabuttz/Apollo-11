/* SPDX-License-Identifier: MIT */
#include "lm_time_free_fall_cases.h"
#define W m->words
/* Chains (case n % 8): entry LOCs in SR30.1 order. */
static const agc_word chains[8][4]={
 {03360,0,0,0},{03361,0,0,0},{03361,03427,0,0},{03361,03427,03465,0},
 {03361,03427,03470,0},{03361,03427,03465,03470},{03361,03470,0,0},{03360,03427,03465,03470}};
static const uint8_t lengths[8]={1,1,2,3,3,4,2,4};
static uint32_t isqrt64(uint64_t v){uint64_t r=0,b=(uint64_t)1<<62;while(b>v)b>>=2;while(b){if(v>=r+b){v-=r+b;r=(r>>1)+b;}else r>>=1;b>>=2;}return (uint32_t)r;}
/* A signed fraction in units of 2^-28 as an AGC DP pair. */
static void put_dp(apollo_overlay_state *m,unsigned at,int32_t q){
 uint32_t a=(uint32_t)(q<0?-q:q);agc_word hi=(agc_word)((a>>14)&037777u),lo=(agc_word)(a&037777u);
 if(q<0){hi=(agc_word)(~hi&AGC_MASK);lo=(agc_word)(~lo&AGC_MASK);}
 W[at]=hi;W[at+1u]=lo;
}
static int32_t get_dp(const apollo_overlay_state *m,unsigned at){
 int32_t hi=(int32_t)W[at],lo=(int32_t)W[at+1u];
 if(hi>=040000)hi-=077777;
 if(lo>=040000)lo-=077777;
 return hi*16384+lo;
}
/* A random direction scaled to length len (units of 2^-28), optionally made
 * perpendicular to the unit direction u (units of 2^-14). */
static void direction(uint32_t *seed,int32_t out[3],const int32_t *perp){
 int32_t w[3];int64_t len2,dot;unsigned i;
 for(;;){
  for(i=0;i<3u;++i)w[i]=(int32_t)(lm_time_free_fall_random(seed)%8193u)-4096;
  if(perp){
   dot=((int64_t)w[0]*perp[0]+(int64_t)w[1]*perp[1]+(int64_t)w[2]*perp[2])/16384;
   for(i=0;i<3u;++i)w[i]-=(int32_t)(dot*perp[i]/16384);
  }
  len2=(int64_t)w[0]*w[0]+(int64_t)w[1]*w[1]+(int64_t)w[2]*w[2];
  if(len2>=(int64_t)1<<20)break;
 }
 for(i=0;i<3u;++i)out[i]=(int32_t)((int64_t)w[i]*16384/(int64_t)isqrt64((uint64_t)len2));
}
uint8_t lm_time_free_fall_calls(uint32_t n){return lengths[n%8u];}
void lm_time_free_fall_call(apollo_overlay_state *m,uint32_t n,uint8_t k){
 /* CALL leaves NEWOPS with LOC at the entry, FBANK 27 (BANKSET, BBANK,
  * INTBIT15 and EDOP from it) and QPRET at the word after the CALL. */
 agc_word loc=chains[n%8u][k];unsigned vac=W[0120];
 W[5]=06070;W[4]=056000;W[6]=(agc_word)(056000u|(W[3]>>8));W[0165]=W[6];W[0115]=040000;W[023]=040000;W[0164]=loc;W[7]=0;W[067]=0;
 /* Luminary has no TFFCONIC caller; a CALL from bank 22 leaves EDOP
  * 040000 (BIT15 of BBANK), which sends TFFCONIC's first DANZIG into
  * CHANG2. TFFCONIC is sampled from the EDOP +0 state instead. */
 if(loc==03360u)W[023]=0;
 W[vac+052u]=loc<=03361u?045567:loc==03427u?045571:loc==03465u?045625:045633;
 if(loc==03465u){
  /* DOTPER DLOAD CALL RPER CALCTPER. */
  W[0154]=W[vac+016u];W[0155]=W[vac+017u];W[0156]=0;W[0163]=0;
 }else if(loc==03470u){
  /* DLOAD HPERMIN; DAD CALL RPADTEM CALCTFF. */
  put_dp(m,0154,get_dp(m,02200)+get_dp(m,02202));W[0156]=0;W[0163]=0;
 }
}
void lm_time_free_fall_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word mu[2][2]={{025004,06702},{027533,07571}}; /* 1/RTMUE, 1/RTMUM */
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,020000,057777};
 uint32_t seed=UINT32_C(0x4f524254)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac,moon=(unsigned)((n/8u)&1u);
 lm_time_free_fall_debris(m,io,n);vac=W[0120];
 /* TFF/RTMU (and MPAC for TFFCONIC), HPERMIN and RPADTEM. */
 W[vac+036u]=mu[moon][0];W[vac+037u]=mu[moon][1];W[0154]=mu[moon][0];W[0155]=mu[moon][1];
 put_dp(m,02200,moon?(int32_t)(21336u+lm_time_free_fall_random(&seed)%4096u):(int32_t)(45720u+lm_time_free_fall_random(&seed)%16384u));
 put_dp(m,02202,moon?(int32_t)(3476000u+lm_time_free_fall_random(&seed)%8192u):(int32_t)(3186669u+lm_time_free_fall_random(&seed)%8192u));
 if((n/16u)%8u==7u){
  /* Raw edge words in RONE and VONE. */
  for(i=0;i<6u;++i){W[02206u+i]=edge[lm_time_free_fall_random(&seed)%8u];W[02214u+i]=(agc_word)(lm_time_free_fall_random(&seed)&AGC_MASK);}
  if(n&64u)for(i=0;i<6u;i+=2u)W[02214u+i]&=017777u;
  if((n&0700u)==0700u)for(i=0;i<6u;++i)W[02206u+i]=(n&04000u)?0u:077777u;
 }else{
  int32_t u[3],t[3],uq[3];int64_t kk,kkr,v2;uint32_t r,v,f;int32_t sn,cs;
  uint32_t alt=lm_time_free_fall_random(&seed);
  /* Radius in units of 2^-28 of the scale (Earth 2^29 m, Moon 2^27 m). */
  if(moon)r=(1738000u+((n/128u)%4u==0u?alt%10000000u:alt%300000u))*2u;
  else r=(6378000u+((n/128u)%4u==0u?alt%100000000u:alt%1500000u))/2u;
  {int32_t k=(int32_t)W[vac+036u]*16384+(int32_t)W[vac+037u];kk=(int64_t)k*k>>28;}
  kkr=kk*(int64_t)r>>28;if(kkr<1)kkr=1;
  v2=((int64_t)1<<47)/kkr;
  v=isqrt64((uint64_t)v2<<28);
  /* Speed: circular speed times 0.5-1.5, 0-1.6 or 1.4-2.5 (hyperbolic),
   * or escape speed within 2^-16 (near-parabolic). */
  switch((n/256u)%8u){
  case 0:f=lm_time_free_fall_random(&seed)%26215u;break;
  case 1:f=0;v=isqrt64((uint64_t)v2<<29);v=(uint32_t)((int64_t)v+(int64_t)v*((int32_t)(lm_time_free_fall_random(&seed)%65u)-32)/2097152);break;
  case 2:f=23000u+lm_time_free_fall_random(&seed)%18000u;break;
  case 3:f=12000u+lm_time_free_fall_random(&seed)%20000u;break;
  default:f=8192u+lm_time_free_fall_random(&seed)%16385u;break;
  }
  if(f)v=(uint32_t)((uint64_t)v*f>>14);
  if(v>0x0fffffffu)v=0x0fffffffu;
  sn=(int32_t)(lm_time_free_fall_random(&seed)%19661u)-9830;cs=(int32_t)isqrt64((uint64_t)((int64_t)16384*16384-(int64_t)sn*sn));
  if((n/256u)%8u==3u){
   /* Axis-aligned apse: radial speed of a few units of 2^-28 at most, so
    * that R.V is exactly or nearly zero (perigee or apogee). */
   unsigned a=lm_time_free_fall_random(&seed)%3u;int32_t sr=(lm_time_free_fall_random(&seed)&1u)?-1:1;
   for(i=0;i<6u;++i){W[02206u+i]=0;W[02214u+i]=0;}
   put_dp(m,02206u+2u*a,sr*(int32_t)r);
   put_dp(m,02214u+2u*((a+1u)%3u),(lm_time_free_fall_random(&seed)&1u)?-(int32_t)v:(int32_t)v);
   put_dp(m,02214u+2u*a,sr*((int32_t)(lm_time_free_fall_random(&seed)%7u)-3));
  }else{
   direction(&seed,u,0);for(i=0;i<3u;++i)uq[i]=u[i];
   direction(&seed,t,uq);
   for(i=0;i<3u;++i){
    put_dp(m,02206u+2u*i,(int32_t)((int64_t)r*u[i]/16384));
    put_dp(m,02214u+2u*i,(int32_t)(((int64_t)v*cs/16384*t[i]+(int64_t)v*sn/16384*u[i])/16384));
   }
  }
 }
 if((n/256u)%8u==3u&&(n/8u)%16u==0u&&(n%8u==0u||n%8u==7u)){
  /* Exact parabolas for TFFCONIC (which takes 1/SQRT(MU) in MPAC): RONE,
   * VONE and 1/SQRT(MU) powers of two with ALFA exactly zero, for the BZE
   * paths of TFFCONIC and TFFRP/RA. */
  static const uint8_t ex[3][3]={{10,13,13},{12,12,13},{12,13,12}};const uint8_t *e=ex[(n/2048u)%3u];
  for(i=0;i<6u;++i){W[02206u+i]=0;W[02214u+i]=0;}
  W[02206]=(agc_word)(1u<<e[0]);W[02216]=(agc_word)(1u<<e[1]);W[0154]=(agc_word)(1u<<e[2]);W[0155]=0;
 }
 lm_time_free_fall_call(m,n,0);
}
