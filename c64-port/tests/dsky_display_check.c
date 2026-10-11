/* SPDX-License-Identifier: MIT */
#include "dsky_display_check.h"
#include "p66_mode_display.h"
#include "dsky_relay.h"
static apollo_overlay_state source;
static dsky_display_state device;
uint16_t dsky_display_check(volatile uint8_t *screen,volatile uint8_t *color){
 uint16_t i,failures=0;p66_bias_io basic={0100,0};dsky_relay_io relay={{0,0},0,0};
 for(i=0;i<APOLLO_SHARED_WORDS;++i)source.words[i]=0;
 source.words[3]=01000;source.words[4]=060000;source.words[6]=060002;source.words[01011]=66;
 for(i=0;i<11;++i)source.words[01023+i]=04000;
 if(p66_mode_display(&source,&basic)!=P66_BIAS_OK || source.words[01016]!=1)++failures;
 source.words[0]=0;source.words[2]=02070;source.words[4]=014000;source.words[5]=02024;source.words[6]=014002;source.words[0776]=10;
 if(dsky_relay_drain(&source,&relay)!=P66_BIAS_OK || relay.wrote_channel10!=1 || relay.channel10!=055634)++failures;
 dsky_display_reset(&device);
 if(dsky_display_flash_visible(&device)!=0)++failures;
 dsky_display_advance(&device,31);if(dsky_display_flash_visible(&device)!=0)++failures;
 dsky_display_advance(&device,1);if(dsky_display_flash_visible(&device)!=1)++failures;
 dsky_display_advance(&device,96);if(dsky_display_flash_visible(&device)!=0)++failures;
 dsky_display_advance(&device,65535);if(dsky_display_flash_visible(&device)!=1 || device.flash_phase!=127)++failures;
 dsky_display_output(&device,010,relay.channel10);dsky_display_render(&device,screen,color,1);
 if(screen[143]!=54 || screen[144]!=54 || screen[256]!=32 || screen[421]!=32)++failures;
 /* Known relay wiring: selector8 low digitR1D1=1;selector7 R1D2/3=2/3 +;
  * selector6 R1D4/5=4/5 -,positive sign wins bothwiredsigns. */
 dsky_display_output(&device,010,040003);dsky_display_output(&device,010,037473);dsky_display_output(&device,010,032776);
 dsky_display_output(&device,010,051243); /* VERB01 */
 dsky_display_output(&device,010,045177); /* NOUN79 */
 dsky_display_output(&device,010,060450); /* NO ATT/GIMBAL LOCK/PROG */
 dsky_display_output(&device,011,0336); /* COMP/UPLINK/TEMP/KEY/OPERR/RESTART */
 dsky_display_render(&device,screen,color,1);
 if(screen[421]!=49 || screen[422]!=50 || screen[423]!=51 || screen[424]!=52 || screen[425]!=53 || screen[418]!=43)++failures;
 if(screen[256]!=48 || screen[257]!=49 || screen[269]!=55 || screen[270]!=57)++failures;
 if((color[122]&15u)!=7 || (color[202]&15u)!=7 || (color[282]&15u)!=7 || (color[362]&15u)!=7 || (color[442]&15u)!=7 || (color[522]&15u)!=7 || (color[602]&15u)!=7 || (color[682]&15u)!=7)++failures;
 dsky_display_output(&device,011,0160);dsky_display_render(&device,screen,color,0);
 if(screen[256]!=32 || screen[257]!=32 || screen[269]!=32 || screen[270]!=32 || screen[143]!=54)++failures;
 if((color[362]&15u)!=11 || (color[442]&15u)!=11)++failures;
 dsky_display_output(&device,011,040);
 dsky_display_output(&device,010,0);dsky_display_render(&device,screen,color,1);
 if(screen[143]!=54 || screen[256]!=48 || screen[269]!=55)++failures;
 dsky_display_output(&device,010,054000);dsky_display_render(&device,screen,color,1);
 if(screen[143]!=32 || screen[144]!=32)++failures;
 return failures;
}
