/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERPRETER_EXCHANGE_NATIVE_H
#define LM_INTERPRETER_EXCHANGE_NATIVE_H
#include "lm_interpreter_change.h"
#include "native_dispatch.h"
typedef struct {exec_native *runtime;apollo_overlay_state *image;p66_bias_io *io;
 exec_transfer transfer;uint16_t callbacks,status;
 uint16_t (*invoke_change)(void *,apollo_overlay_state *,p66_bias_io *,exec_transfer *);
 void *change_modules;} lm_interpreter_exchange_binding;
exec_continuation lm_interpreter_exchange_resolve(void *,exec_address,enum exec_context_kind);
enum exec_action lm_interpreter_exchange_callback(executive *,void *,exec_address *);
#endif
