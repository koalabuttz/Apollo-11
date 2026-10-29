/* SPDX-License-Identifier: MIT */
#include <cbm.h>
#include "c64_dsky_keyboard.h"
#include "dsky_keyboard.h"
enum c64_dsky_key_status c64_dsky_keyboard_poll(uint16_t *channel15,uint8_t *raw_character){uint8_t character=cbm_k_getin();if(raw_character!=0)*raw_character=character;if(character==0)return C64_DSKY_KEY_EMPTY;return dsky_keyboard_decode(character,channel15)!=0 ? C64_DSKY_KEY_READY : C64_DSKY_KEY_UNMAPPED;}
