/* SPDX-License-Identifier: MIT */
#ifndef CM_NATIVE_DISPATCH_H
#define CM_NATIVE_DISPATCH_H
#include "cm_dispatch.h"
enum cm_exec_action { CM_EXEC_YIELD, CM_EXEC_FINISH, CM_EXEC_SLEEP,
 CM_EXEC_REPRIORITIZE, CM_EXEC_ENTER_INTERPRETER, CM_EXEC_EXIT_INTERPRETER };
enum cm_exec_context_kind { CM_EXEC_CONTEXT_BASIC=0, CM_EXEC_CONTEXT_INTERPRETIVE=1 };
typedef enum cm_exec_action (*cm_exec_continuation)(cm_executive *,void *,exec_address *);
typedef struct { exec_address address; cm_exec_continuation call; } cm_exec_mapping;
typedef struct { exec_address address; enum cm_exec_context_kind kind;
 cm_exec_continuation call; } cm_exec_context_mapping;
typedef struct {
 cm_executive *state; const cm_exec_mapping *mapping; void *user;
 exec_address current; uint8_t count,running; enum exec_status fault;
 agc_word requested_priority; uint8_t in_callback;
 const cm_exec_context_mapping *context_mapping;
 enum cm_exec_context_kind current_kind; exec_interpreter interpreter_control;
} cm_exec_native;
/* Foreground CM trampoline. Caller owns exclusive contexts/IRQ coordination;
 * no IRQ request queue or hardware exclusion adapter is implied. Callback C
 * frames RETURN before any scheduler context exchange. Persistent data lives
 * in core/VAC fields. Typed tables use actual positive continuation addresses.
 * Interpreter yields require EDOP=0 and explicit next-pair addresses; sleeping,
 * finishing, and reprioritizing require explicit EXIT into a basic body. */
enum exec_status cm_exec_native_init(cm_exec_native *,cm_executive *,const cm_exec_mapping *,uint8_t,void *);
enum exec_status cm_exec_native_context_init(cm_exec_native *,cm_executive *,const cm_exec_context_mapping *,uint8_t,void *);
int cm_exec_native_is_mapped(const cm_exec_native *,exec_address,enum cm_exec_context_kind);
enum exec_status cm_exec_native_start(cm_exec_native *);
enum exec_status cm_exec_native_step(cm_exec_native *);
enum exec_status cm_exec_native_admit(cm_exec_native *,agc_word,exec_address,int);
/* Foreground safe-boundary pending-alarm check; does not deliver IRQ requests. */
enum exec_status cm_exec_native_poll(cm_exec_native *);
enum exec_status cm_exec_native_wake(cm_exec_native *,agc_word);
/* Only within a basic callback; paired with REPRIORITIZE on its return. */
enum exec_status cm_exec_native_request_priority(cm_exec_native *,agc_word);
#endif
