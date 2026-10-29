/* SPDX-License-Identifier: MIT */
#include "rtb_unit.h"
#include "guidance_unit_phased.h"
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
enum rtb_unit_status rtb_unit_phase(apollo_overlay_state *image,p66_bias_io *io,enum rtb_mission mission,uint8_t phase)
{
 static const uint16_t entry[3]={03024,03031,03043};frame f={0};enum p66_unit_result status;unsigned i;uint16_t *w=image->words,base=w[0120];
 if((unsigned)mission>1u||phase>2u||w[5]!=entry[phase]||w[4]!=0||w[3]>03400u||(w[3]&0377u)||w[6]!=(w[3]>>8)||w[7]||w[0163]!=077776u||io->interrupt_enabled>1u||(io->superbank&~0160u))return RTB_UNIT_OUT_OF_SCOPE;
 if(base!=0401u&&base!=0455u&&base!=0531u&&base!=0605u&&base!=0661u)return RTB_UNIT_OUT_OF_SCOPE;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)*owned(&f,fields[i].offset)=w[fields[i].address];
 f.push.base=base;f.push.pushloc=w[0166];for(i=0;i<P66_VAC_WORDS;++i)f.push.words[i]=w[base+i];
 if(phase==0u){
  static const unsigned at[3]={0,3,5};
  for(i=0;i<3u;++i){f.n.vbuf[i*2u]=f.p.mpac[at[i]];f.n.vbuf[i*2u+1u]=f.p.mpac[at[i]+1u];}
  f.n.tem1=f.sc.ovfind;f.sc.ovfind=0;
  status=p66_length_square_phase(&f.p,&f.sc,&f.el,&f.d,&f.n,&f.push)==P66_NORM_OK?P66_UNIT_OK:P66_UNIT_OUT_OF_SCOPE;
  f.d.dvnormct=03031;
 }
 else if(phase==1u)status=p66_unit_root_phase(&f.p,&f.sc,&f.el,&f.d,&f.n,&f.push);
 else status=p66_unit_finish_phase(&f.p,&f.sc,&f.el,&f.d,&f.n,&f.push);
 if(status==P66_UNIT_OUT_OF_SCOPE)return RTB_UNIT_OUT_OF_SCOPE;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)w[fields[i].address]=*owned(&f,fields[i].offset);
 for(i=0;i<P66_VAC_WORDS;++i)w[base+i]=f.push.words[i];
 if(status==P66_UNIT_ALARM_1302){w[5]=03402;w[2]=03043;return RTB_UNIT_ABORT_1302;}
 if(status==P66_UNIT_OVERFLOW){w[5]=(uint16_t)(mission==RTB_LM?06061u:06030u);w[2]=02413;return RTB_UNIT_OVERFLOW;}
 w[5]=(uint16_t)(phase<2u?entry[phase+1u]:mission==RTB_LM?06061u:06030u);w[2]=(uint16_t)(phase==0u?0140u:3u);return RTB_UNIT_OK;
}

enum rtb_unit_status rtb_unit_begin(apollo_overlay_state *image,p66_bias_io *io,enum rtb_mission mission)
{
 uint16_t *w=image->words,base=w[0120];
 if((unsigned)mission>1u||w[5]!=03023u||w[4]!=0||w[3]>03400u||(w[3]&0377u)||w[6]!=(w[3]>>8)||w[7]||w[0163]!=077776u||io->interrupt_enabled>1u||(io->superbank&~0160u))return RTB_UNIT_OUT_OF_SCOPE;
 if(base!=0401u&&base!=0455u&&base!=0531u&&base!=0605u&&base!=0661u)return RTB_UNIT_OUT_OF_SCOPE;
 w[2]=03024;w[5]=03010;
 (void)rtb_vector(image,io,mission,RTB_VECAGREE);return RTB_UNIT_OK;
}
