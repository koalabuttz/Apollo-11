/* SPDX-License-Identifier: MIT */
#include "lm_powered_constants.h"
bool lm_powered_constant_read(uint16_t address,uint16_t *out){static const uint16_t words[5]={00736,4,2,0,4};if(out==0)return false;if(address==8221u){*out=02664;return true;}if(address<20466u||address>20470u)return false;*out=words[address-20466u];return true;}
