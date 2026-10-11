/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_DISPATCH_H
#define NATIVE_DISPATCH_H
#include "executive.h"
enum exec_action { EXEC_YIELD, EXEC_FINISH, EXEC_SLEEP, EXEC_REPRIORITIZE,
                   EXEC_ENTER_INTERPRETER, EXEC_EXIT_INTERPRETER,
                   EXEC_DISPATCHED_BASIC, EXEC_DISPATCHED_INTERPRETER };
enum exec_context_kind { EXEC_CONTEXT_BASIC=0, EXEC_CONTEXT_INTERPRETIVE=1 };
/* A callback returns to this trampoline; locals on its C stack do not survive
 * a context change. Store persistent job data in MPAC/VAC. resume is an explicit 2CADR. */
typedef enum exec_action (*exec_continuation)(executive *, void *, exec_address *);
typedef struct { exec_address address; exec_continuation call; } exec_mapping;
typedef struct { exec_address address; enum exec_context_kind kind;
                 exec_continuation call; } exec_context_mapping;
struct exec_irq_queue;
struct exec_exclusion;
struct exec_native_services;
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
    const struct exec_native_services *services;
    void *service_user;
    exec_continuation (*resolve)(void *,exec_address,enum exec_context_kind);
    void *registry_user;
} exec_native;
typedef struct exec_native_services {
 enum exec_status (*prepare)(void *,exec_native *,exec_continuation *);
 enum exec_status (*finish)(void *,exec_native *,enum exec_action,exec_address);
} exec_native_services;
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
/* A compiled known-entry resolver is an alternative to a contiguous registry.
 * It receives a positive source continuation and explicit context kind, never
 * an AGC opcode. It returns a resident C callback or NULL. Resolver/callbacks
 * must remain resident across every service/math load. No source state changes
 * occur on initialization or lookup. The caller owns resolver domain/table
 * uniqueness; missing targets retain the ordinary UNMAPPED fault behavior. */
enum exec_status exec_native_resolver_init(exec_native *,executive *,
 exec_continuation (*)(void *,exec_address,enum exec_context_kind),void *registry_user,void *callback_user);
int exec_native_is_mapped(const exec_native *runtime, exec_address address,
                          enum exec_context_kind kind);
enum exec_status exec_native_start(exec_native *runtime);
/* Executes one native continuation, then performs its returned lifecycle action.
 * A yield with NEWJOB=+0 resumes directly, like the basic NEWJOB poll around
 * CHANG1. An unmapped address/error latches a fault and blocks further calls.
 * Bound IRQ requests are admitted at guarded safe points before callback entry
 * and after callback return. Admission does not preempt a callback. */
enum exec_status exec_native_step(exec_native *runtime);
/* Source-exact guarded stages shared by linked and disk service builds. Caller
 * owns exclusion across each call. Prepare returns a resident callback; finish
 * runs only after its C frame returns. No service-slot pointer may survive.
 * Native service hooks must load their module under that same guard and return
 * before a resident continuation can replace the slot. Bind before running. */
enum exec_status exec_native_prepare(exec_native *,exec_continuation *);
enum exec_status exec_native_finish(exec_native *,enum exec_action,exec_address);
enum exec_status exec_native_bind_services(exec_native *,const exec_native_services *,void *);
/* DISPATCHED_BASIC is for a source-qualified canonical basic body that has
 * already completed its source return or Executive service/exchange. The
 * callback imports the resulting Executive and returns its actual selected
 * basic pair in resume, or {0,0} for the source idle route. The trampoline only
 * adopts that metadata: no lifecycle operation, NEWJOB poll or post-return IRQ
 * admission is repeated. The next explicit native entry may admit IRQ requests.
 * Positive active priority and an existing basic mapping are required; idle
 * requires NEWJOB=-0 and a nonpositive active priority. */
/* DISPATCHED_INTERPRETER has the same already-completed source-service
 * contract, for a BASIC caller whose verified INTRSM body selected an
 * interpretive pair. The callback imports INTBIT15/EDOP explicitly; positive
 * active priority, a mapped interpretive pair, EDOP0 and matching bank bit are
 * required. It never performs another exchange, poll or IRQ admission. */
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
