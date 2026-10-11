/* SPDX-License-Identifier: MIT */
#include "p66_descent_prefix_control.h"
uint8_t p66_descent_prefix_control(apollo_overlay_state *m,agc_word completed,agc_word ddv_q){agc_word cyr=0;uint8_t retain=0;
 if(completed==03353u){m->words[5]=03353;return 1;}
 if(m->words[023]!=0)return 0;
 switch(completed){
 case 03356:cyr=020012;m->words[2]=3;break;
 case 03360:cyr=020032;break;
 case 03361:retain=1;break;
 case 03364:cyr=020014;m->words[2]=ddv_q;break;
 case 03366:cyr=020003;break;
 case 03370:cyr=020024;break;
 case 03373:cyr=020000;break;
 case 03374:cyr=5;break;
 case 03375:retain=1;break;
 case 03377:cyr=024000;break;
 case 03401:cyr=020006;break;
 case 03404:cyr=020036;break;
 case 03407:cyr=020032;break;
 case 03411:cyr=020006;break;
 case 03414:cyr=020021;break;
 case 03416:cyr=040005;break;
 case 03420:cyr=074000;break;
 case 03421:retain=1;break;
 case 03422:cyr=020034;break;
 case 03427:cyr=020027;break;
 case 03430:retain=1;break;
 case 03433:cyr=020000;break;
 default:return 0;
 }
 m->words[5]=06061;if(!retain)m->words[020]=cyr;return 1;
}
