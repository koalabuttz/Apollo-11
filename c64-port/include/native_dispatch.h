/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_DISPATCH_H
#define NATIVE_DISPATCH_H
#include "executive.h"
enum exec_action { EXEC_YIELD, EXEC_FINISH, EXEC_SLEEP, EXEC_REPRIORITIZE,
                   EXEC_ENTER_INTERPRETER, EXEC_EXIT_INTERPRETER };
enum exec_context_kind { EXEC_CONTEXT_BASIC=0, EXEC_CONTEXT_INTERPRETIVE=1 };
/* A callback returns to this trampoline; locals on its C stack do not survive
 * a context change. Store persistent job data in MPAC/VAC. resume is an explicit 2CADR. */
typedef enum exec_action (*exec_continuation)(executive *, void *, exec_address *);
typedef struct { exec_address address; exec_continuation call; } exec_mapping;
typedef struct { exec_address address; enum exec_context_kind kind;
                 exec_continuation call; } exec_context_mapping;
struct exec_irq_queue;
struct exec_exclusion;
typedef struct {
    executive *state;
    const exec_mapping *mapping;
    void *user;
    exec_address current;
    uint8_t count, running;
    enum exec_status fault;
    struct exec_irq_queue *requests;
    const struct exec_exclusion *exclusion;
    agc_word requested_priority;
    uint8_t in_callback;
    const exec_context_mapping *context_mapping;
    enum exec_context_kind current_kind;
    exec_interpreter interpreter_control;
} exec_native;
/* The table and user storage must outlive the runtime. Duplicate addresses,
 * null callbacks and non-basic locations are rejected. No pointer truncation. */
enum exec_status exec_native_init(exec_native *runtime, executive *state,
                                 const exec_mapping *mapping, uint8_t count, void *user);
/* Mixed tables use positive, actual basic/interpretive continuation addresses.
 * Admission remains basic: INTPRET is an explicit native action at a basic
 * entry. Interpreter YIELD supplies the next operation-pair address and requires
 * EDOP=+0. ENTER/EXIT cross the original INTPRET/EXIT boundaries. Interpretive
 * sleep/completion require EXIT into a mapped basic JOBSLEEP/ENDOFJOB body. */
enum exec_status exec_native_context_init(exec_native *runtime, executive *state,
                                         const exec_context_mapping *mapping,
                                         uint8_t count, void *user);
int exec_native_is_mapped(const exec_native *runtime, exec_address address,
                          enum exec_context_kind kind);
enum exec_status exec_native_start(exec_native *runtime);
/* Executes one native continuation, then performs its returned lifecycle action.
 * A yield with NEWJOB=+0 resumes directly, like the basic NEWJOB poll around
 * CHANG1. An unmapped address/error latches a fault and blocks further calls.
 * Bound IRQ requests are admitted at guarded safe points before callback entry
 * and after callback return. Admission does not preempt a callback. */
enum exec_status exec_native_step(exec_native *runtime);
/* Foreground helpers for a callback's own admission or explicit safe-point poll.
 * With an IRQ binding these restore the platform caller's previous IRQ state.
 * Errors latch a native fault; neither helper invokes a continuation. */
enum exec_status exec_native_admit(exec_native *runtime, agc_word priority,
                                  exec_address address, int needs_vac);
enum exec_status exec_native_poll(exec_native *runtime);
/* Guarded foreground wake by one-word CADR. NOT_FOUND is nonfatal. No dispatch;
 * a woken idle job starts on the next start/step. Unsupported mappings fault. */
enum exec_status exec_native_wake(exec_native *runtime, agc_word wake_cadr);
/* Stage a priority inside a basic callback, then return REPRIORITIZE with
 * resume. The actual mutation/exchange occurs only after the C frame returns.
 * Staged priority is cleared at each callback entry and must be paired with
 * REPRIORITIZE on that return. SLEEP supplies a mapped switched-window resume
 * (02000..03777); its EBANK/SUPERBNK must match current, because JOBSLEEP saves those current flags. */
enum exec_status exec_native_request_priority(exec_native *runtime, agc_word priority);
#endif
