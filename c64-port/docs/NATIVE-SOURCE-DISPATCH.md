# Source-selected continuations and resident scheduler stages

The native Executive can adopt a continuation already selected by a translated
source service. `EXEC_DISPATCHED_BASIC` and `EXEC_DISPATCHED_INTERPRETER` do not
repeat an Executive exchange, NEWJOB test, lifecycle operation or post-callback
IRQ admission. They require a BASIC caller, a positive active priority and a
mapped positive source continuation. Interpreter adoption additionally requires
EDOP zero and INTBIT15 matching the bank. Source idle uses BASIC adoption with
`{0,0}`, NEWJOB negative zero and a nonpositive active priority. The next native
entry can admit queued native requests. These are explicit native actions;
there is no AGC instruction decoder.

`exec_native_prepare` and `exec_native_finish` share the existing step algorithm.
The resident trampoline owns the original two exclusion intervals, obtains a
resident callback from prepare, returns from that service, invokes the callback,
and only then calls finish. A service hook can load a native scheduler overlay
under the same guard. No native stack frame or function pointer in the replaced
slot survives a load. A loader failure stops dispatch before another callback or
lifecycle operation. The separate scheduler adapter owns the disk transport.

A compiled known-entry resolver can replace the contiguous mapping table. It
receives a positive address and explicit BASIC/interpreter kind, and returns a
resident callback or null. It never reads source instruction words. Initializers
clear resolver/service fields; ordinary mapping users retain their existing
behavior. Flight-specific resolvers must explicitly qualify their own address
sets and callback semantics.

Target-only `EXEC_NATIVE_FINISH_FAMILY` selects action families: 1 is yield and
interpreter entry/exit; 2 is complete, sleep and priority; 3 is source-selected
adoption. With the macro absent all actions remain present. The family selector
strips unused compiled branches and helpers; it changes no source algorithm.
The scheduler router selects a family from the explicit callback action.

Qualification uses original assembly snapshots, including every canonical word
and external I/O metadata in the source-service fixtures. The default linked
runtime passes 256 priority/sleep and 128 ENDOFJOB transitions under both guard
states, retaining a native IRQ request posted during each callback. Seven
interpreter/adoption and four BASIC rejection cases cover unsupported domains.
Stock PAL C64 ROM tests exercise 128 priority/sleep transitions and 128 real END
basic/interpreter/idle transitions, both native IRQ-mask states, zero additional
zero-page storage, and restoring exclusion. The target family-3 seam is the same
compiled family used by the corresponding scheduler overlay. Measured software
stack peaks are 1540 and 1527 bytes in the standalone 3840-byte test stack.

Existing mixed-context source comparisons (140 checkpoints of all 324 modeled
Executive words), the live guidance prefix (2944 source checkpoints), and real
P66 preemption (400 checkpoints) retain their published snapshot identities
under strict compilation and ASAN/UBSAN. These bounded proofs do not certify
untranslated flight programs or complete mission behavior. The exact artifacts,
source identities, dependencies and commands are recorded in the accompanying
proof manifest.
