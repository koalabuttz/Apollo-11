/* SPDX-License-Identifier: MIT
 * Direct original DSPOUTSB/DSPSCAN/DSPLAY source composition. */
#include "dsky_relay.h"
#include "agc_word.h"
enum p66_bias_status dsky_relay_drain(apollo_overlay_state *s,dsky_relay_io *io){
 static const uint16_t codes[]={04000,010000,014000,020000,024000,030000,034000,040000,044000,050000,054000};
 uint16_t *w=s->words,cnt=w[0776];uint8_t i,second=0;
 if(w[2]!=02070 || w[3]!=01000 || w[4]!=014000 || w[6]!=014002 || w[0]>037777 || (cnt>10 && cnt!=077777) || io->control.superbank>0160 || (io->control.superbank&017u)!=0 || io->control.interrupt_enabled>1)return P66_BIAS_OUT_OF_SCOPE;
 for(i=0;i<11;++i)if(w[01023+i]==0 || w[01023+i]==077777)return P66_BIAS_OUT_OF_SCOPE;
 io->wrote_channel10=0;w[01016]=agc_correct(w[0]);w[073]=077777;w[0776]=cnt==0 ? 077777 : cnt;
 for(;;){uint16_t p=cnt==077777 ? 0 : cnt,old=w[01023+p];
  if((old&040000u)!=0){uint16_t mag=agc_negate(old);w[01023+p]=mag;w[073]=(uint16_t)(mag&03777u);w[0]=agc_add_register(agc_extend(codes[p]),agc_extend(w[073]));io->channel10=agc_correct(w[0]);io->wrote_channel10=1;w[2]=06741;w[5]=02071;break;}
  if(cnt!=0 && cnt!=077777){--cnt;if(cnt==0)cnt=077777;w[0776]=cnt;continue;}
  if(second!=0){w[01016]=0;w[073]=0;w[0]=0;w[2]=3;w[5]=02070;break;}
  second=1;w[073]=0;cnt=10;w[0776]=10;
 }
 w[7]=0;return P66_BIAS_OK;
}
