/* SPDX-License-Identifier: MIT */
#ifndef P66_DESCENT_VIEW_H
#define P66_DESCENT_VIEW_H
#include "guidance_horizontal.h"
#include "guidance_feedback_pairs.h"
#include "guidance_throttle_service.h"
#include <stddef.h>
typedef struct {
 p66_preparation p;p66_scale_scratch s;p66_elapsed_state e;
 p66_divide_scratch d;p66_norm_scratch n;p66_pushdown push;
 agc_dp hdot,vhoriz;agc_word delvs[6],position[6];
 p66_throttle_feedback feedback;p66_throttle_inputs inputs;agc_word bias[6];
 p66_feedback_pair_control control;p66_throttle_service_state service;
} descent_frame;
typedef struct {uint16_t address,offset;} field;
#define F(a,m) {a,(uint16_t)offsetof(descent_frame,m)}
/* Only transient physical views; source address aliases have one owner. */
static const field fields[]={
 F(03746,p.rate.count),F(03756,p.rate.scale),F(03644,p.rate.desired.high),F(03645,p.rate.desired.low),F(0,p.rate.registers.a),F(1,p.rate.registers.l),
 F(037,p.pipa[0]),F(040,p.pipa[1]),F(041,p.pipa[2]),F(024,p.time.high),F(025,p.time.low),
 F(03763,p.old_pipa[0]),F(03764,p.old_pipa[1]),F(03765,p.old_pipa[2]),F(070,p.ruptreg[0]),F(071,p.ruptreg[1]),F(072,p.ruptreg[2]),F(03761,p.this_piptime.high),F(03762,p.this_piptime.low),
 F(01160,p.pipatmp[0]),F(01161,p.pipatmp[1]),F(01162,p.pipatmp[2]),F(01254,p.tem[0]),F(01255,p.tem[1]),F(01256,p.tem[2]),
 F(0154,p.mpac[0]),F(0155,p.mpac[1]),F(0156,p.mpac[2]),F(0157,p.mpac[3]),F(0160,p.mpac[4]),F(0161,p.mpac[5]),F(0162,p.mpac[6]),F(0163,p.mpac[7]),
 F(03766,p.delvrod[0]),F(03767,p.delvrod[1]),F(03770,p.delvrod[2]),F(03771,p.delvrod[3]),F(03772,p.delvrod[4]),F(03773,p.delvrod[5]),F(0135,s.mptemp),F(0121,s.ovfind),
 F(01234,e.piptime.high),F(01235,e.piptime.low),F(0130,e.buf),F(0131,d.buf_low),F(0136,d.dvsign),F(0137,d.dvnormct),F(0140,d.maxdvsw),
 F(021,n.sr),F(022,n.cyl),F(0116,n.addrwd),F(0132,n.buf_third),F(0141,n.tem1),
 F(0122,n.vbuf[0]),F(0123,n.vbuf[1]),F(0124,n.vbuf[2]),F(0125,n.vbuf[3]),F(0126,n.vbuf[4]),F(0127,n.vbuf[5]),
 F(03473,hdot.high),F(03474,hdot.low),F(02262,vhoriz.high),F(02263,vhoriz.low),
 F(03733,delvs[0]),F(03734,delvs[1]),F(03735,delvs[2]),F(03736,delvs[3]),F(03737,delvs[4]),F(03740,delvs[5]),
 F(03520,position[0]),F(03521,position[1]),F(03522,position[2]),F(03523,position[3]),F(03524,position[4]),F(03525,position[5]),
 F(03463,feedback.afc.high),F(03464,feedback.afc.low),F(03610,feedback.fweight.high),F(03611,feedback.fweight.low),F(03757,feedback.lasttpip.high),F(03760,feedback.lasttpip.low),
 F(01244,inputs.mass.high),F(01245,inputs.mass.low),F(02542,inputs.lag_tau.high),F(02543,inputs.lag_tau.low),F(02546,inputs.maxforce.high),F(02547,inputs.maxforce.low),F(02544,inputs.minforce.high),F(02545,inputs.minforce.low),
 F(02630,bias[0]),F(02631,bias[1]),F(02632,bias[2]),F(02633,bias[3]),F(02634,bias[4]),F(02635,bias[5]),
 F(5,control.pc),F(2,control.q),F(4,control.fb),F(3,control.eb),F(6,control.bb),F(0164,control.loc),F(0165,control.bankset),F(020,control.cyr),F(023,control.edop),F(0115,control.intbit15),F(0117,control.polish),
 F(03465,service.fcodd.high),F(03466,service.fcodd.low),F(03467,service.fp.high),F(03470,service.fp.low),F(03612,service.pif.high),F(03613,service.pif.low),
 F(03615,service.fc),F(03620,service.fcold),F(03606,service.pifpset),F(03614,service.pseudo55),F(03617,service.tthrot),F(03607,service.rtnhold),F(0055,service.thrust),
 F(02506,service.lowcrit),F(02507,service.highcrit),F(01011,service.modreg),F(01442,service.phase3[0]),F(01443,service.phase3[1])
};
#undef F
static uint16_t *word(descent_frame *f,uint16_t offset){return (uint16_t *)((unsigned char *)f+offset);}

#endif
