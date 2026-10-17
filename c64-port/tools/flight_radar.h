/* SPDX-License-Identifier: GPL-2.0-or-later
 * FLIGHT_RR for tools/flight_host_run.c (host only, NEVER part of the
 * native port; docs/FLIGHT-STORE.md, Flight scenarios): a rendezvous radar
 * locked on the CSM, the same on both sides, so P20 designates, locks on
 * and reads range and range rate, and R22 takes marks.
 *
 * FLIGHT_RR=CS:LM:CSM, each of LM and CSM six numbers (rx,ry,rz,vx,vy,vz:
 * metres and metres per second, basic reference coordinates, moon centred)
 * at AGC centisecond CS: the true orbits, Kepler conics about the moon
 * (the scenario load starts the AGC's state vectors on the same ones).
 *
 * At each sample point (idle at ADVAN, after the pokes and the engine):
 * - Attitude: CDUX-Z take the DAP's desired angles CDUXD-ZD (perfect
 *   attitude control, as FLIGHT_ENGINE does), so R61 and R65 maneuvers end.
 * - RR CDUs: channel 12 bit 1 (zero RR CDU) zeroes CDUT and CDUS; channel
 *   14 bits 12 and 11 (RR drive) empty CDUTCMD and CDUSCMD into the error
 *   counters, as the hardware does. With channel 12 bit 2 (error counter
 *   enable) or bit 14 (auto tracker) on, the antenna points at the CSM: CDUT
 *   and CDUS take the true line of sight in nav base axes (mode 1, LEM
 *   GEOMETRY RRNB's (sin S cos T, -sin T, cos S cos T)), through REFSMMAT
 *   and the IMU CDUs (SM to NB: inner Y, middle Z, outer X).
 * - Data good: channel 33 bit 4 is low (good) while the tracker is on and
 *   the CSM is within 400 nautical miles, high otherwise.
 * - A read: once channel 13 bit 4 (activity) has been on 9 cs (yaAGC's
 *   radar gate, 9 TIME5 periods), RNRAD takes the reading the select bits
 *   name (1 range, high scale: channel 33 bit 3 stays high; 2 range rate,
 *   biased 17000), the activity bit drops and RADARUPT is requested. */
#ifndef FLIGHT_RADAR_H
#define FLIGHT_RADAR_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "agc_transfer.h"

static int rr_on;
static unsigned long rr_t0;
static double rr_lm[6],rr_csm[6];
static struct rr { unsigned long since; int active; } rr_nat;
enum { RR_CDUT=035, RR_CDUS=036, RR_RNRAD=046, RR_CDUTCMD=053, RR_REFSMMAT=01733, RR_CDUX=032, RR_CDUXD=03235 };
#define RR_MU 4.9028e12

static int rr_parse(const char *t){int n=0;
 if(!t)return 1;
 if(sscanf(t,"%lu:%lf,%lf,%lf,%lf,%lf,%lf:%lf,%lf,%lf,%lf,%lf,%lf%n",&rr_t0,&rr_lm[0],&rr_lm[1],&rr_lm[2],&rr_lm[3],&rr_lm[4],&rr_lm[5],
           &rr_csm[0],&rr_csm[1],&rr_csm[2],&rr_csm[3],&rr_csm[4],&rr_csm[5],&n)!=13||t[n])return 0;
 rr_on=1;return 1;}

/* The conic state dt seconds after s (Battin's f and g in the eccentric
 * anomaly difference). */
static void rr_kepler(const double *s,double dt,double *out){double r0=sqrt(s[0]*s[0]+s[1]*s[1]+s[2]*s[2]),
 v2=s[3]*s[3]+s[4]*s[4]+s[5]*s[5],a=1.0/(2.0/r0-v2/RR_MU),sg=(s[0]*s[3]+s[1]*s[4]+s[2]*s[5])/sqrt(RR_MU),
 n=sqrt(RR_MU/(a*a*a)),sa=sqrt(a),de=n*dt,r,f,g,fd,gd;unsigned i;
 for(i=0;i<50u;++i){double fn=de+sg/sa*(1.0-cos(de))-(1.0-r0/a)*sin(de)-n*dt,
  dfn=1.0+sg/sa*sin(de)-(1.0-r0/a)*cos(de);de-=fn/dfn;}
 r=a+(r0-a)*cos(de)+sg*sa*sin(de);
 f=1.0-a/r0*(1.0-cos(de));g=a*sg/sqrt(RR_MU)*(1.0-cos(de))+r0*sqrt(a/RR_MU)*sin(de);
 fd=-sqrt(RR_MU*a)/(r*r0)*sin(de);gd=1.0-a/r*(1.0-cos(de));
 for(i=0;i<3u;++i){out[i]=f*s[i]+g*s[i+3u];out[i+3u]=fd*s[i]+gd*s[i+3u];}}

static long rr_ones(agc_word v){return (v&040000u)?-(long)((~v)&037777u):(long)v;}
static double rr_dp(const agc_word *w,unsigned a,int scale){
 return ((double)rr_ones(w[a])*16384.0+(double)rr_ones(w[a+1u]))*ldexp(1.0,scale-28);}
static double rr_cdu(agc_word v){return (double)(v&077777u)*(6.283185307179586/32768.0);}
static agc_word rr_angle(double a){long c=lround(a/6.283185307179586*32768.0);return (agc_word)((unsigned long)c&077777ul);}

/* One sample point at time t: w the erasable image, the channels, and the
 * interrupt requests (*radarupt set to request RADARUPT). */
static void rr_due(unsigned long t,struct rr *e,agc_word *w,agc_word *ch12,agc_word *ch13,const agc_word *ch14,agc_word *ch33,int *radarupt){
 double l[6],c[6],rel[6],sm[3],nb[3],range,rdot,ig,mg,og,x1,y1,z1,x2,y2;unsigned k,j;
 if(!rr_on)return;
 for(k=0;k<3u;++k)w[RR_CDUX+k]=w[RR_CDUXD+k];
 if(*ch12&1u)w[RR_CDUT]=w[RR_CDUS]=0u;
 for(k=0;k<2u;++k)if(*ch14&(04000u>>k))w[RR_CDUTCMD+k]=0u;
 rr_kepler(rr_lm,((double)t-(double)rr_t0)*0.01,l);rr_kepler(rr_csm,((double)t-(double)rr_t0)*0.01,c);
 for(k=0;k<6u;++k)rel[k]=c[k]-l[k];
 range=sqrt(rel[0]*rel[0]+rel[1]*rel[1]+rel[2]*rel[2]);
 rdot=(rel[0]*rel[3]+rel[1]*rel[4]+rel[2]*rel[5])/range;
 if(*ch12&020002u){
  for(j=0;j<3u;++j){sm[j]=0.0;for(k=0;k<3u;++k)sm[j]+=rr_dp(w,RR_REFSMMAT+6u*j+2u*k,1)*rel[k]/range;}
  ig=rr_cdu(w[RR_CDUX+1u]);mg=rr_cdu(w[RR_CDUX+2u]);og=rr_cdu(w[RR_CDUX]);
  x1=sm[0]*cos(ig)-sm[2]*sin(ig);z1=sm[0]*sin(ig)+sm[2]*cos(ig);y1=sm[1];
  x2=x1*cos(mg)+y1*sin(mg);y2=-x1*sin(mg)+y1*cos(mg);
  nb[0]=x2;nb[1]=y2*cos(og)+z1*sin(og);nb[2]=-y2*sin(og)+z1*cos(og);
  w[RR_CDUT]=rr_angle(asin(-nb[1]));w[RR_CDUS]=rr_angle(atan2(nb[0],nb[2]));}
 if((*ch12&020000u)&&range<400.0*1852.0)*ch33&=(agc_word)~010u;else *ch33|=010u;
 *ch33|=04u;
 if(!(*ch13&010u)){e->active=0;return;}
 if(!e->active){e->active=1;e->since=t;}
 if(t<e->since+9u)return;
 e->active=0;
 if((*ch13&7u)==1u)w[RR_RNRAD]=(agc_word)lround(range/(2.859024*8.0))&077777u;
 else if((*ch13&7u)==2u)w[RR_RNRAD]=(agc_word)(17000l-lround(rdot/0.19135344))&077777u;
 *ch13&=(agc_word)~010u;*radarupt=1;}
#endif
