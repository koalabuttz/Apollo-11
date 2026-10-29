/* SPDX-License-Identifier: MIT */
#include "p66_launch_phased.h"
#include "p66_initialization.h"
enum p66_launch_status p66_launch_setup_phase(apollo_overlay_state *image,p66_bias_io *io){
 uint16_t *w,base;p66_basic_initialization basic={0};enum exec_status admitted;
 if(!image||!io)return P66_LAUNCH_OUT_OF_SCOPE;
 w=image->words;base=w[0120];
 if(w[5]!=02477||w[3]!=03400||w[4]!=062000||w[6]!=062007||w[067]!=0||
    base<0401||base>0661||(base-0401u)%054u!=0||w[0166]!=base||w[base-1u]!=0||
    (w[0167]&0777u)!=base||w[0167]<030000||w[0167]>=040000||w[026]>037777||
    io->superbank>060||(io->superbank&017u)||io->interrupt_enabled>1)return P66_LAUNCH_OUT_OF_SCOPE;
 basic.a=w[0];basic.l=w[1];basic.q=02500;basic.bbank=w[6];basic.superbank=io->superbank;basic.interrupt_enabled=io->interrupt_enabled;
 basic.phase3_address[0]=w[01442];basic.phase3_address[1]=w[01443];
 (void)p66_fastchng(&basic);
 w[01442]=basic.phase3_address[0];w[01443]=basic.phase3_address[1];w[0]=basic.a;w[1]=basic.l;w[2]=02501;
 admitted=p66_major_mode_start(image,io);
 if(admitted!=EXEC_OK)return P66_LAUNCH_NO_CORE;
 w[0]=agc_extend(w[03644]);w[1]=agc_extend(w[03645]);w[03644]=w[03473];w[03645]=w[03474];w[5]=02505;
 return P66_LAUNCH_OK;
}
enum p66_launch_status p66_launch_bias_phase(apollo_overlay_state *image,p66_bias_io *io){
 if(!image||!io||image->words[5]!=02505)return P66_LAUNCH_OUT_OF_SCOPE;
 return p66_bias_start(image,io)==P66_BIAS_OK?P66_LAUNCH_OK:P66_LAUNCH_OUT_OF_SCOPE;
}
