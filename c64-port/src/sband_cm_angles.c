/* SPDX-License-Identifier: MIT */
#include "sband_cm_angles.h"
#include "sband_dot.h"
#include "sband_scalar.h"
#include "inverse_trig.h"
#include "guidance_shift.h"
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
static void shift_one(apollo_overlay_state *s){p66_preparation p={0};p66_scale_scratch sc;unsigned i;for(i=0;i<8u;++i)p.mpac[i]=s->words[0154u+i];p.rate.registers.a=s->words[0];p.rate.registers.l=s->words[1];sc.mptemp=s->words[0135];sc.ovfind=s->words[0121];(void)p66_scalar_shift_left(&p,&sc,1,1);for(i=0;i<8u;++i)s->words[0154u+i]=p.mpac[i];s->words[0]=p.rate.registers.a;s->words[1]=p.rate.registers.l;s->words[0135]=sc.mptemp;s->words[0121]=sc.ovfind;s->words[021]=0;s->words[020]=024000;}
static enum sband_cm_angles_status inverse(apollo_overlay_state *s){enum inverse_trig_status t;t=inverse_trig_prepare(s,INVERSE_TRIG_CM,0);if(t==INVERSE_TRIG_OK)t=inverse_trig_root(s,INVERSE_TRIG_CM);if(t==INVERSE_TRIG_OK)t=inverse_trig_polynomial(s,INVERSE_TRIG_CM);if(t==INVERSE_TRIG_OK)t=inverse_trig_finish(s,INVERSE_TRIG_CM);if(t==INVERSE_TRIG_ALARM_1301)return SBAND_CM_ANGLES_ALARM1301;if(t==INVERSE_TRIG_ALARM_1302)return SBAND_CM_ANGLES_ALARM1302;return SBAND_CM_ANGLES_OK;}
static enum sband_cm_angles_status yaw(apollo_overlay_state *s,p66_bias_io *io){static const uint8_t pos[6]={0,1,3,4,5,6};static const agc_word ux[6]={020000,0,0,0,0,0},uy[6]={0,0,020000,0,0,0};unsigned i,base,stack;agc_word eb,low;enum sband_cm_angles_status t;int sign=0;
 if(!s||!io)return SBAND_CM_ANGLES_UNSUPPORTED;
 base=s->words[0120];eb=s->words[3];
 if(s->words[5]!=06037u||s->words[0164]!=03724u||s->words[023]||s->words[0163]!=077776u||s->words[4]!=064000u||eb>03400u||(eb&0377u)||s->words[6]!=(agc_word)(064000u|(eb>>8))||s->words[0165]!=s->words[6]||s->words[0115]!=040000u||s->words[067]||s->words[7]||base<0401u||base>0661u||(base-0401u)%054u||s->words[0166]!=base+016u||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return SBAND_CM_ANGLES_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return SBAND_CM_ANGLES_UNSUPPORTED;
 stack=base+016u;low=s->words[stack+5u];for(i=0;i<6u;++i)s->words[stack+i]=s->words[0154u+pos[i]];s->words[1]=agc_extend(low);s->words[0166]=(agc_word)(stack+6u);
 bank(s,eb,054000);s->words[5]=07300;s->words[0116]=03332;s->words[0164]=03725;s->words[020]=020027;s->words[023]=0;(void)sband_dot(s,APOLLO_FIXED_CM,ux);
 bank(s,eb,0);s->words[0164]=03726;s->words[023]=051;shift_one(s);
 s->words[5]=03611;s->words[0]=4;s->words[020]=040002;s->words[023]=0;t=inverse(s);if(t!=SBAND_CM_ANGLES_OK)return t;
 /* Actual DP PDVL pushes yaw, then reloads six saved URP words. */
 stack=base+024u;s->words[stack]=s->words[0154];s->words[stack+1u]=s->words[0155];low=s->words[0162];for(i=0;i<6u;++i)s->words[0154u+pos[i]]=s->words[base+016u+i];s->words[1]=agc_extend(low);s->words[0163]=077776;s->words[0166]=(agc_word)(stack+2u);
 bank(s,eb,054000);s->words[5]=07300;s->words[0116]=03330;s->words[0164]=03731;s->words[020]=020027;s->words[023]=0;(void)sband_dot(s,APOLLO_FIXED_CM,uy);
 bank(s,eb,0);s->words[0164]=03732;s->words[023]=0133;shift_one(s);
 for(i=0;i<3u;++i){sign=agc_to_int(s->words[0154u+i]);if(sign)break;}
 s->words[0117]=065737;s->words[2]=sign>0?3u:06710u;
 if(sign<0){agc_word old_low=s->words[0155],old_stack_low=s->words[base+025u];s->words[2]=06712;s->words[0154]=037777;s->words[0155]=037777;s->words[0156]=0;s->words[1]=agc_extend(old_low);s->words[0166]=(agc_word)(base+024u);bank(s,eb,064000);s->words[0116]=(agc_word)(base+024u);s->words[0164]=03735;s->words[023]=0;s->words[020]=020004;s->words[5]=06754;(void)sband_scalar(s,APOLLO_FIXED_CM,SBAND_DSU,0);s->words[base+024u]=s->words[0154];s->words[base+025u]=s->words[0155];s->words[1]=agc_extend(old_stack_low);s->words[0166]=(agc_word)(base+026u);s->words[0]=0;s->words[020]=040007;}
 else {s->words[0]=agc_extend(040000);s->words[020]=050005;}
 bank(s,eb,064000);s->words[5]=06037;s->words[0164]=03737;s->words[023]=0;return SBAND_CM_ANGLES_OK;
}

static enum sband_cm_angles_status pitch(apollo_overlay_state *s,p66_bias_io *io,uint8_t null_path){static const uint8_t pos[6]={0,1,3,4,5,6};static const agc_word uz[6]={0,0,0,0,020000,0},quarter[2]={010000,0};unsigned i,base;agc_word eb,low;enum sband_cm_angles_status t;
 if(!s||!io)return SBAND_CM_ANGLES_UNSUPPORTED;
 base=s->words[0120];eb=s->words[3];
 if(s->words[5]!=06037u||s->words[0164]!=03737u||s->words[023]||s->words[0163]!=(null_path?077776u:0u)||s->words[4]!=064000u||eb>03400u||(eb&0377u)||s->words[6]!=(agc_word)(064000u|(eb>>8))||s->words[0165]!=s->words[6]||s->words[0115]!=040000u||s->words[067]||s->words[7]||base<0401u||base>0661u||(base-0401u)%054u||s->words[0166]!=base+(null_path?016u:026u)||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return SBAND_CM_ANGLES_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return SBAND_CM_ANGLES_UNSUPPORTED;
 low=s->words[0162];for(i=0;i<6u;++i)s->words[0154u+pos[i]]=s->words[base+010u+i];s->words[1]=agc_extend(low);s->words[0163]=077776;
 bank(s,eb,054000);s->words[5]=07300;s->words[0116]=03326;s->words[0164]=03741;s->words[020]=020027;s->words[023]=0;(void)sband_dot(s,APOLLO_FIXED_CM,uz);
 bank(s,eb,0);s->words[0164]=03742;s->words[023]=051;shift_one(s);
 s->words[5]=03611;s->words[0]=4;s->words[020]=040002;s->words[023]=0;t=inverse(s);if(t!=SBAND_CM_ANGLES_OK)return t;
 bank(s,eb,054000);s->words[5]=06754;s->words[0116]=03324;s->words[0164]=03744;s->words[020]=020032;s->words[023]=0;(void)sband_scalar(s,APOLLO_FIXED_CM,SBAND_DSU,quarter);
 /* Original STODL RHOSB followed by DLOAD YAWANG, then STORE GAMMASB.
  * Both DP stores retain old destination minor debris; neither aliases VAC. */
 low=s->words[02323];s->words[02320]=s->words[0154];s->words[02321]=s->words[0155];s->words[0154]=s->words[base+024u];s->words[0155]=s->words[base+025u];s->words[0156]=0;s->words[02322]=s->words[0154];s->words[02323]=s->words[0155];s->words[1]=agc_extend(low);s->words[0]=0;s->words[2]=3;s->words[0116]=01722;s->words[020]=020006;s->words[023]=0;s->words[0164]=03750;bank(s,eb,064000);s->words[5]=06037;return SBAND_CM_ANGLES_OK;
}
enum sband_cm_angles_status sband_cm_angles(apollo_overlay_state *s,p66_bias_io *io){enum sband_cm_angles_status t=yaw(s,io);return t==SBAND_CM_ANGLES_OK?pitch(s,io,0):t;}
enum sband_cm_angles_status sband_cm_angles_null(apollo_overlay_state *s,p66_bias_io *io){return pitch(s,io,1);}
