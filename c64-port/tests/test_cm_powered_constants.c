/* SPDX-License-Identifier: MIT */
#include "cm_powered_constants.h"
unsigned cm_powered_constants_check(void){static const uint16_t original[5]={00736,4,2,0,4};unsigned i,f=0;uint16_t value;for(i=0;i<5u;++i)if(!cm_powered_constant_read((uint16_t)(20445u+i),&value)||value!=original[i])++f;value=012345;if(cm_powered_constant_read(20444,&value)||value!=012345||cm_powered_constant_read(20450,&value)||value!=012345||cm_powered_constant_read(20445,0))++f;return f;}
