/* SPDX-License-Identifier: MIT */
#include "lm_powered_constants.h"
unsigned lm_powered_constants_check(void){static const uint16_t original[5]={00736,4,2,0,4};unsigned i,f=0;uint16_t value;for(i=0;i<5u;++i)if(!lm_powered_constant_read((uint16_t)(20466u+i),&value)||value!=original[i])++f;if(!lm_powered_constant_read(8221,&value)||value!=02664)++f;value=012345;if(lm_powered_constant_read(20465,&value)||value!=012345||lm_powered_constant_read(20471,&value)||value!=012345||lm_powered_constant_read(8220,&value)||value!=012345||lm_powered_constant_read(20466,0))++f;return f;}
