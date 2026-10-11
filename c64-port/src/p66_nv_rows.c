/* SPDX-License-Identifier: MIT */
#include "p66_nv_rows.h"
#include "agc_word.h"
enum p66_bias_status p66_nv_data_prepare(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words;uint8_t i;static const uint16_t addresses[3]={02262,03473,03534};
 if(w[5]!=03604u||w[3]!=0||w[4]!=062000u||w[6]!=062000u||w[7]!=0||io->superbank!=0100u||io->interrupt_enabled>1u||w[01001]!=6u||w[01002]!=60u)return P66_BIAS_OUT_OF_SCOPE;
 w[01013]=w[01014]=w[01015]=w[01000]=0;w[0156]=w[0161];w[01041]=077771;w[0777]=077754;
 w[0146]=024074;w[0147]=060512;w[0140]=2;w[0153]=010347;
 for(i=0;i<3u;++i){w[0150u+i]=addresses[i];w[0125u+i]=w[addresses[i]+1u];w[01003u+i]=(uint16_t)(w[0125u+i]^AGC_MASK);}
 w[0114]=02243;w[0117]=2;w[0122]=0;w[0145]=0125;w[020]=077770;w[022]=077765;
 w[0]=0;w[1]=03025;w[2]=02537;w[3]=02000;w[4]=062000;w[6]=062004;w[5]=02537;return P66_BIAS_OK;
}
