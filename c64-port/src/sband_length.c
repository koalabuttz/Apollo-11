/* SPDX-License-Identifier: MIT */
#include "sband_length.h"
#include "guidance_length_phased.h"
#include <stddef.h>
typedef struct {p66_preparation p;p66_scale_scratch sc;p66_elapsed_state el;p66_divide_scratch d;p66_norm_scratch n;p66_pushdown push;} frame;
typedef struct {uint16_t address,offset;} field;
#define F(a,m) {a,(uint16_t)offsetof(frame,m)}
static const field fields[]={
 F(0,p.rate.registers.a),F(1,p.rate.registers.l),
 F(0154,p.mpac[0]),F(0155,p.mpac[1]),F(0156,p.mpac[2]),F(0157,p.mpac[3]),F(0160,p.mpac[4]),F(0161,p.mpac[5]),F(0162,p.mpac[6]),F(0163,p.mpac[7]),
 F(0135,sc.mptemp),F(0121,sc.ovfind),F(0130,el.buf),F(0131,d.buf_low),F(0136,d.dvsign),F(0137,d.dvnormct),F(0140,d.maxdvsw),
 F(021,n.sr),F(022,n.cyl),F(0116,n.addrwd),F(0132,n.buf_third),F(0141,n.tem1),
 F(0122,n.vbuf[0]),F(0123,n.vbuf[1]),F(0124,n.vbuf[2]),F(0125,n.vbuf[3]),F(0126,n.vbuf[4]),F(0127,n.vbuf[5])};
#undef F
static uint16_t *owned(frame *f,uint16_t at){return (uint16_t *)((unsigned char *)f+at);}

enum sband_length_status sband_length(apollo_overlay_state *image,p66_bias_io *io,enum apollo_fixed_mission mission){frame f={0};enum p66_norm_result status;unsigned i;uint16_t *w,base,shift;
 if(!image||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return SBAND_LENGTH_UNSUPPORTED;
 w=image->words;base=w[0120];
 if(w[5]!=03201u||w[4]||w[3]>03400u||(w[3]&0377u)||w[6]!=(w[3]>>8)||w[7]||w[0163]!=077776u||(base!=0401u&&base!=0455u&&base!=0531u&&base!=0605u&&base!=0661u)||io->interrupt_enabled>1u||(io->superbank&~0160u))return SBAND_LENGTH_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return SBAND_LENGTH_UNSUPPORTED;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)*owned(&f,fields[i].offset)=w[fields[i].address];
 f.push.base=base;f.push.pushloc=w[0166];for(i=0;i<P66_VAC_WORDS;++i)f.push.words[i]=w[base+i];
 (void)p66_length_square_phase(&f.p,&f.sc,&f.el,&f.d,&f.n,&f.push);status=p66_length_root_phase(&f.p,&f.sc,&f.el,&f.d,&f.n,&f.push);
 shift=f.sc.mptemp;
 if(status==P66_NORM_OK)(void)p66_length_finish_phase(&f.p,&f.sc,&f.el,&f.d,&f.n,&f.push);
 for(i=0;i<sizeof fields/sizeof fields[0];++i)w[fields[i].address]=*owned(&f,fields[i].offset);
 for(i=0;i<P66_VAC_WORDS;++i)w[base+i]=f.push.words[i];
 /* A long unnormalize TC GENSCR+1 changes Q; count14 terminates
  * before that internal call. Both original missions share its return02314. */
 w[2]=status==P66_NORM_ALARM_1302?03210u:shift>14u?02314u:3u;w[5]=status==P66_NORM_ALARM_1302?03402u:mission==APOLLO_FIXED_LM?06061u:06030u;
 return status==P66_NORM_ALARM_1302?SBAND_LENGTH_ABORT_1302:SBAND_LENGTH_OK;
}
