/* SPDX-License-Identifier: GPL-2.0-or-later
 * The LM landing radar for tools/flight_host_run.c (host only, NEVER part
 * of the native port; docs/FLIGHT-STORE.md, Flight scenarios). Included
 * after the yaAGC state m and flight_radar.h (struct radar_drops).
 *
 * FLIGHT_LR_POS2=CS: the antenna. Channel 12 bit 13 (LRPOS2's command)
 * moves it to position 2 CS centiseconds later: channel 33 bit 7 (in
 * position 2) goes to 0 and bit 6 (in position 1) to 1, on both sides.
 * Start the scenario in position 1 (FLIGHT_CH33 bit 6 0, bit 7 1).
 *
 * FLIGHT_LR=CS[:DH] (native side, needs FLIGHT_SCALED): the radar itself,
 * powered from AGC centisecond CS, so P63-P66 take LR altitude and
 * velocity (SERVICER POSUPDAT, VELUPDAT; after V57 the updates move the
 * state vector). The replay rows carry its RNRAD and channel 33 writes to
 * the C64, as they do FLIGHT_RR's. DH (metres, default 0) raises the true
 * surface above the AGC's landing-site radius /LAND/, so the AGC starts
 * DH too high and the updates have that error to remove.
 * FLIGHT_LR_DROP=T0-T1[,T0-T1...]: both data-good bits off in those
 * windows (flight_radar.h's dropout list).
 *
 * The true trajectory. At the first sample point from CS with the AGC's
 * altitude HCALC set, the model takes the AGC's own state RN, VN
 * (reference axes at PIPTIME, into stable-member axes through REFSMMAT)
 * plus the PIPAs' velocity since PIPTIME, and from then on flies it
 * itself: central lunar gravity plus the same velocity changes the engine
 * model counts into the PIPAs (lrm_dv, at each interval's midpoint).
 * Without updates the AGC's navigation stays within a few metres of it.
 * Attitude is perfect: the CDUs are the true attitude.
 *
 * The readings, at a read's start (channel 13 bit 4 rising with bit 3, the
 * LR select): bits 1-2 pick velocity X (0), Y (1), Z (2) or range (3),
 * INITREAD's LRVELX/Y/Z and LRALT.
 * - Beams: SERVICER SETPOS's geometry. The antenna's X and Y axes and the
 *   range beam HBEAMANT (-.4687018041, 0, -.1741224271, a half unit) go
 *   into nav base through the antenna angles (LRALPHA/LRBETA1 in position
 *   1, LRALPHA2/LRBETA2 in position 2: channel 33 bit 7 clear), velocity
 *   Z = X cross Y, then into stable-member axes through the CDUs.
 * - Range: the slant distance along the range beam to the true surface (a
 *   sphere), in 1.079 ft counts, less SERVICER's Doppler correction (V
 *   along the beam times RADSKAL, times SKALSKAL on low scale), so
 *   POSUPDAT's slant range is the true one. High scale (channel 33 bit 9
 *   set) above HISCALIM's 2,481.7 ft, 5.395 ft counts. The AGC turns slant
 *   range into altitude along the local vertical, so DELTAH carries the
 *   lunar curvature over the beam's footprint (about 20 m at 15 km), as in
 *   flight.
 * - Velocity: the velocity over the surface (true V plus the AGC's DELVS,
 *   the lunar-rotation term SERVICER adds) along the beam, in VXSCAL,
 *   VYSCAL, VZSCAL counts (-.644, 1.212, .8668 ft/s), biased 12288
 *   (LVELBIAS).
 * - Data good, channel 33 bit 5 (range) and bit 8 (velocity), active low,
 *   set at every sample point: on while the range beam meets the surface
 *   within 50,000 ft and the LM is more than 10 ft up, outside the dropout
 *   windows and before touchdown (FLIGHT_TOUCHDOWN).
 * The radar cycle itself (the activity bit reset and RADARUPT) is the
 * resident timer's, as for FLIGHT_RR. */
#ifndef FLIGHT_LR_H
#define FLIGHT_LR_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "agc_transfer.h"

static unsigned long lr_pos2,lr_ref,lr_nat;
static void lr_due(unsigned long t,unsigned long *at,agc_hardware *hw){agc_word c12=hw?hw->channel12:(agc_word)m.InputChannel[012];
 if(!lr_pos2||*at==1ul)return;
 if(!*at){if(c12&010000u)*at=t+lr_pos2;return;}
 if(t<*at)return;
 *at=1ul;
 if(hw)hw->channel33=(agc_word)((hw->channel33&~0100u)|040u);else m.InputChannel[033]=(int16_t)((m.InputChannel[033]&~0100)|040);}

#if RK_LM
enum { LR_CDUX=032, LR_PIPAX=037, LR_RNRAD=046, LR_RN=01220, LR_VN=01226, LR_PIPTIME=01234, LR_RADSKAL=01354,
       LR_SKALSKAL=01356, LR_REFSMMAT=01733, LR_LAND=02332, LR_ALPHA=02522, LR_HCALC=03534, LR_DELVS=03733 };
#define LR_MU 4.9028e12          /* m^3/s^2, as MOON_MU */
#define LR_FT 0.3048
#define LR_HSCAL 0.3288792       /* metres a low-scale count (HSCAL) */

static struct lrm_state {
 int on, started, active, landed;
 unsigned long t0, t;
 double dh, rs, r[3], v[3];
 struct radar_drops drops;
} lrm;

static int lrm_parse(const char *t){int n=0;
 if(!t)return 1;
 if(sscanf(t,"%lu%n",&lrm.t0,&n)!=1)return 0;
 t+=n;
 if(*t==':'){if(sscanf(t+1,"%lf%n",&lrm.dh,&n)!=1)return 0;t+=1+n;}
 if(*t)return 0;
 lrm.on=1;return 1;}

/* The channel 33 bits the LR models own (chan_due's mask). */
static agc_word lr_ch33_bits(void){return (agc_word)((lrm.on?0720u:0u)|(lr_pos2?0140u:0u));}

static long lr_ones(agc_word v){return (v&040000u)?-(long)((~v)&037777u):(long)v;}
static double lr_dp(const agc_word *w,unsigned a,int scale){
 return ((double)lr_ones(w[a])*16384.0+(double)lr_ones(w[a+1u]))*ldexp(1.0,scale-28);}
static double lr_angle(agc_word v){return (double)(v&077777u)*(6.283185307179586/32768.0);}
/* Centiseconds from PIPTIME to now on the AGC clock (TIME2, TIME1). */
static double lr_since_pip(const agc_word *w){
 double now=(double)(w[024]&037777u)*16384.0+(double)(w[025]&037777u),
  pip=(double)lr_ones(w[LR_PIPTIME])*16384.0+(double)lr_ones(w[LR_PIPTIME+1u]),d=now-pip;
 while(d<0.0)d+=268435456.0;
 return d;}

/* Stable member to nav base through gimbal angles inner ig (Y), middle mg
 * (Z), outer og (X), as *SMNB*; lr_nbsm is its inverse. */
static void lr_smnb(double ig,double mg,double og,const double *a,double *b){
 double x1=a[0]*cos(ig)-a[2]*sin(ig),z1=a[0]*sin(ig)+a[2]*cos(ig),y1=a[1],
  x2=x1*cos(mg)+y1*sin(mg),y2=-x1*sin(mg)+y1*cos(mg);
 b[0]=x2;b[1]=y2*cos(og)+z1*sin(og);b[2]=-y2*sin(og)+z1*cos(og);}
static void lr_nbsm(double ig,double mg,double og,const double *b,double *a){
 double y2=b[1]*cos(og)-b[2]*sin(og),z1=b[1]*sin(og)+b[2]*cos(og),
  x1=b[0]*cos(mg)-y2*sin(mg),y1=b[0]*sin(mg)+y2*cos(mg);
 a[0]=x1*cos(ig)+z1*sin(ig);a[1]=y1;a[2]=-x1*sin(ig)+z1*cos(ig);}

/* Gravity only, from lrm.t to t (cs), in steps of at most 10 cs. */
static void lr_fly(unsigned long t){
 while(lrm.t<t){unsigned long s=t-lrm.t>10ul?10ul:t-lrm.t;double dt=(double)s*0.01,rm;unsigned k;
  rm=sqrt(lrm.r[0]*lrm.r[0]+lrm.r[1]*lrm.r[1]+lrm.r[2]*lrm.r[2]);
  for(k=0;k<3u;++k){lrm.v[k]-=LR_MU/(rm*rm*rm)*lrm.r[k]*dt*0.5;lrm.r[k]+=lrm.v[k]*dt;}
  rm=sqrt(lrm.r[0]*lrm.r[0]+lrm.r[1]*lrm.r[1]+lrm.r[2]*lrm.r[2]);
  for(k=0;k<3u;++k)lrm.v[k]-=LR_MU/(rm*rm*rm)*lrm.r[k]*dt*0.5;
  lrm.t+=s;}}

/* The engine model's velocity change dv (m/s, stable-member axes) over
 * the dt seconds to AGC centisecond t, taken at the interval's midpoint. */
static void lrm_dv(const double *dv,unsigned long t,double dt){unsigned k;unsigned long mid=t-(unsigned long)lround(dt*50.0);
 if(!lrm.started)return;
 if(mid>lrm.t)lr_fly(mid);
 for(k=0;k<3u;++k)lrm.v[k]+=dv[k];}

/* The four beams in stable-member axes: 0 velocity X, 1 Y, 2 Z, 3 range. */
static void lr_beams(const agc_word *w,agc_word ch33,double b[4][3]){
 static const double ux[3]={1.0,0.0,0.0},uy[3]={0.0,1.0,0.0},hb[3]={-.4687018041*2.0,0.0,-.1741224271*2.0};
 unsigned p=(ch33&0100u)?0u:2u,k;double nb[4][3],ig,mg,og,al=lr_angle(w[LR_ALPHA+p]),be=lr_angle(w[LR_ALPHA+p+1u]);
 lr_smnb(be,0.0,al,ux,nb[0]);lr_smnb(be,0.0,al,uy,nb[1]);lr_smnb(be,0.0,al,hb,nb[3]);
 nb[2][0]=nb[0][1]*nb[1][2]-nb[0][2]*nb[1][1];nb[2][1]=nb[0][2]*nb[1][0]-nb[0][0]*nb[1][2];nb[2][2]=nb[0][0]*nb[1][1]-nb[0][1]*nb[1][0];
 ig=lr_angle(w[LR_CDUX+1u]);mg=lr_angle(w[LR_CDUX+2u]);og=lr_angle(w[LR_CDUX]);
 for(k=0;k<4u;++k)lr_nbsm(ig,mg,og,nb[k],b[k]);}

/* The slant range (m) along unit u from r to the sphere of radius rs, or
 * -1 if the beam misses it. */
static double lr_slant(const double *r,const double *u,double rs){
 double ru=r[0]*u[0]+r[1]*u[1]+r[2]*u[2],rr=r[0]*r[0]+r[1]*r[1]+r[2]*r[2],d=ru*ru-(rr-rs*rs);
 if(d<0.0)return -1.0;
 d=-ru-sqrt(d);return d>0.0?d:-1.0;}

static void lrm_start(unsigned long t,const agc_word *w){unsigned j,k;double rm=0.0,rn[3],vn[3];
 if(!w[LR_HCALC]&&!w[LR_HCALC+1u])return;  /* no descent navigation yet */
 for(k=0;k<3u;++k){rn[k]=lr_dp(w,LR_RN+2u*k,29);vn[k]=lr_dp(w,LR_VN+2u*k,7)*100.0;}
 for(j=0;j<3u;++j){lrm.r[j]=lrm.v[j]=0.0;
  for(k=0;k<3u;++k){double m1=lr_dp(w,LR_REFSMMAT+6u*j+2u*k,1);lrm.r[j]+=m1*rn[k];lrm.v[j]+=m1*vn[k];}
  rm+=lrm.r[j]*lrm.r[j];}
 if(rm<1e12)return;
 for(k=0;k<3u;++k)lrm.v[k]+=(double)lr_ones(w[LR_PIPAX+k])*0.01;
 lrm.t=t-(unsigned long)lr_since_pip(w);
 lrm.rs=lr_dp(w,LR_LAND,24)+lrm.dh;
 lrm.started=1;
 lr_fly(t);
 if(getenv("FLIGHT_LR_TRACE"))printf("LR start %lu cs: |R| %.1f m, surface %.1f m\n",t,sqrt(rm),lrm.rs);}

/* A sample point, or a read's start (channel 13 bit 4 rising with bit 3). */
static void lrm_due(unsigned long t,unsigned long touchdown,agc_word *w,const agc_word *ch13,agc_word *ch33){
 double b[4][3],slant,alt,rm,vr[3],c;unsigned k,sel;
 if(!lrm.on||t<lrm.t0)return;
 if(!lrm.started){lrm_start(t,w);if(!lrm.started)return;}
 if(touchdown&&t>=touchdown)lrm.landed=1;
 lr_fly(t);
 lr_beams(w,*ch33,b);
 rm=sqrt(lrm.r[0]*lrm.r[0]+lrm.r[1]*lrm.r[1]+lrm.r[2]*lrm.r[2]);alt=rm-lrm.rs;
 slant=lr_slant(lrm.r,b[3],lrm.rs);
 if(!lrm.landed&&slant>0.0&&slant<50000.0*LR_FT&&alt>10.0*LR_FT&&!in_drop(&lrm.drops,t))*ch33&=(agc_word)~0220u;else *ch33|=0220u;
 if(slant>0.0)*ch33=(agc_word)(slant>2481.7*LR_FT?*ch33|0400u:*ch33&~0400u);
 if(!(*ch13&010u)){lrm.active=0;return;}
 if(lrm.active||!(*ch13&04u))return;
 lrm.active=1;
 for(k=0;k<3u;++k)vr[k]=lrm.v[k]+lr_dp(w,LR_DELVS+2u*k,5)*100.0;
 sel=*ch13&3u;
 if(sel==3u){int hi=(*ch33&0400u)!=0;double vd=(vr[0]*b[3][0]+vr[1]*b[3][1]+vr[2]*b[3][2])*0.01,d;
  if(slant<0.0)return;
  /* POSUPDAT: slant = (HMEAS + D) HSCAL, D = (V . HBEAM) RADSKAL, times
   * SKALSKAL on low scale; V in m/cs at 2(7), D in counts at 2(28). */
  d=vd/128.0*lr_dp(w,LR_RADSKAL,0)*268435456.0;
  if(!hi)d*=(double)lr_ones(w[LR_SKALSKAL])/16384.0;
  c=slant/LR_HSCAL-d;
  if(hi)c/=5.0;
  if(getenv("FLIGHT_LR_TRACE"))printf("LR %lu cs: range %.1f m slant, %.1f m up, LGC HCALC %.1f m (%s scale)\n",t,slant,alt,lr_dp(w,LR_HCALC,24),hi?"high":"low");}
 else{static const double scale[3]={-.644*LR_FT,1.212*LR_FT,.8668*LR_FT};
  double vb=vr[0]*b[sel][0]+vr[1]*b[sel][1]+vr[2]*b[sel][2];
  c=12288.0+vb/scale[sel];
  if(getenv("FLIGHT_LR_TRACE"))printf("LR %lu cs: velocity %c %.2f m/s\n",t,"XYZ"[sel],vb);}
 w[LR_RNRAD]=(agc_word)((unsigned long)lround(c<0.0?0.0:c>32767.0?32767.0:c)&077777ul);}
#else
/* The CM has no landing radar: the hooks do nothing. */
static struct lrm_state { int on, active; struct radar_drops drops; } lrm;
static void lrm_dv(const double *dv,unsigned long t,double dt){(void)dv;(void)t;(void)dt;}
static void lrm_due(unsigned long t,unsigned long touchdown,agc_word *w,const agc_word *ch13,agc_word *ch33){(void)t;(void)touchdown;(void)w;(void)ch13;(void)ch33;}
#endif
#endif
