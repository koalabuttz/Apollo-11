/* SPDX-License-Identifier: MIT */
#include "lm_interpreter_exchange_payload.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t pointer){lm_interpreter_exchange_arguments *args=(lm_interpreter_exchange_arguments *)(uintptr_t)pointer;if(!args)return EXEC_OUT_OF_SCOPE;return (uint16_t)lm_interpreter_change(image,args->io,args->transfer);}
