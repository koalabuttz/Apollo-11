/* SPDX-License-Identifier: MIT */
#include "cm_powered_constants.h"
bool cm_powered_constant_read(uint16_t address,uint16_t *out){static const uint16_t words[5]={00736,00004,00002,00000,00004};if(out==0||address<20445u||address>20449u)return false;*out=words[address-20445u];return true;}
