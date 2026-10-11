# Fidelity register

"Verified" below refers to the stated domain and recorded evidence. It is not
a proof for arbitrary machine states or an assertion of timing equivalence.

| Component | Original behavior | Native behavior | Fidelity | Justification |
|---|---|---|---|---|
| 15-bit signed encoding / both zeros | One's complement, +0 and -0 distinct | `uint16_t` raw words; explicit conversions/complement | Verified Exact | All 32768 encodings checked analytically; sign-extension reference comparison |
| Sign extension and overflow correction | Duplicate sign in 16-bit A; bit16 moved to bit15 on correction | Explicit unsigned bit operations | Verified Exact | All 32768 word and 65536 register encodings match Virtual AGC helper API |
| Accumulator addition | 16-bit one's-complement end-around carry; two sign bits may differ | `uint32_t` intermediate, explicit carry and correction | Verified Behavioral Equivalent | 524288 boundary pairs match helper; full pair space not tested |
| CCS word classification / diminished absolute value | Four cases including both zeros | Explicit word class/magnitude functions | Verified Exact | Analytical exhaustive unary checks; source branch tracing and Executive scenarios |
| NOVAC/FINDVAC admission | Ordered VAC/core scan; packed priority; special core-zero paths | Ordered native arrays; same saved transitions | Verified Behavioral Equivalent | 26 original executions, complete saved-state comparison; valid-state domain only |
| Eight-core capacity | Inclusive CCS counter starting at 7 tests eight positions | Eight allocatable positions | Verified Behavioral Equivalent | Eighth-slot success and all-eight-occupied failures in original assembly |
| Priority handling | Compare entire packed priority, including VAC/FAKEPRET low bits; retain ties | Same uncorrected accumulator comparison | Verified Behavioral Equivalent | Lower/equal/higher, pending candidate, high boundary scenarios |
| ENDJOB1 / EJSCAN | Release position zero and its VAC; select first highest active context | Same release and scan; optional basic exchange/dispatch | Verified Behavioral Equivalent | Initial six selection scenarios plus completion/dispatch and sequential cases |
| Saved NEWLOC scratch contents | DXCH stores A/L debris; old context goes into registers | Explicit saved debris | Verified Behavioral Equivalent | Full-state differential tests caught and corrected first implementation |
| 2CADR / bank switching | Inline two words; Q return; FBANK/BBANK/SUPERBNK changes | Two explicit word arguments, stored metadata | Approximate | Hardware call mechanism replaced; basic target Z/BBANK/SUPERBNK verified independently, saved register debris compared; Q/A/L ABI remains adapted |
| Resource initialization | Fresh start marks cores free and use words with addresses | Same resource markers plus deterministic extra zeroing | Approximate | Test constructor is not Apollo fresh start |
| 1201/1202 detection | Transfer to BAILOUT1 with inline octal code | Return octal enum, latch native marker; retain pre-bailout resource state | Verified Behavioral Equivalent | Both original trigger boundaries tested; this row covers detection only |
| Alarm light / FAILREG / abort route | BAILOUT1 body, display state, WHIMPER/ENEMA | Bounded alarm recording; remaining abort glue excluded | Verified bounded stage | Separate recording oracle stops at WHIMPER; RESTART.md defines scope |
| Native bailout latch | Apollo exits admission into abort/recovery | Further admission blocked until explicit bounded recovery or caller reinitialization | Approximate | Explicit fail-stop boundary protects unsupported continuation; bounded recovery is a separate adaptation (RESTART.md) |
| Basic CHANJOB state exchange | Swaps twelve words, encodes overflow, updates FIXLOC and dispatch scratch | Same saved transitions and explicit returned 2CADR | Verified Behavioral Equivalent | 109 original-derived checkpoints, including all candidate positions and both zeros |
| Native basic execution | DTCB transfers Z/BBANK and selected SUPERBNK | Complete 2CADR lookup calls ordinary C continuations | Approximate | Hardware transfers checked in oracle; C stack/ABI replaced explicitly; eight actual native two-job sequences pass host/C64 |
| Interpreter dispatch | Negative saved LOC resumes through INTRSM | Rejected before core/resource mutation | Not Implemented | Basic-only domain is explicit |
| Basic sleep/wake / priority change | Dormant encoding, first matching CADR, 2CADR reconstruction, packed PRIOCH2/CCS scan | Same saved transitions; explicit callback sleep/priority actions and foreground wake | Verified Behavioral Equivalent | 81 cases / 115 complete original checkpoints, five native sequences; C ABI/entry scratch adapted as documented in LIFECYCLE.md |
| Interrupt exclusion / ordering | INHINT/RELINT, AGC hardware, polling/yield machinery | Restoring C64 IRQ guards; bounded publication then foreground admission; cooperative yield | Approximate | Real raster IRQs preserve compiler registers and interrupted C; not AGC interrupt/task timing |
| IRQ admission buffer | Requests can originate in interrupt/task routines | Four FIFO requests, overflow fail-stop separate from 1201/1202 | Approximate | Explicit platform adaptation; host wrap/fault tests and real C64 overflow; no generic ready-job queue |
| WAITLIST list / task completion | TIME3, nine task slots, differential intervals, T3RUPT2/TASKOVER | Same persistent list/counter state, ordered insertion and basic task transfers | Verified Behavioral Equivalent | 196 cases / 306 original checkpoints, ten native sequences; scratch/ABI boundary in WAITLIST.md |
| CIA timer / native task timing | Centisecond counter, interrupt-task entry with I inhibited | CIA1 B publishes two-centisecond samples; foreground mapped callbacks run excluded | Approximate | Six actual CIA IRQs, register/decimal preservation, periodic sample and task/job bridge; SVCT3 body and hardware RESUME excluded |
| Bounded alarm/recovery stages | Alarm records, resource cleanup, protected phase/job demands | Explicit alarm/STARTSB1/Type C job translations and composed native recovery | Verified stages; composed adaptation | 190 cases / 226 full modeled-state checkpoints; remaining routes/glue excluded in RESTART.md |
| MP / DAS normal memory arithmetic | Signed zeros, DP product, low carry, independent signs, returned A/L overflow | Explicit raw-word/native integer arithmetic | Verified Behavioral Equivalent | 12,856 numerical/rate checkpoints; register aliases/edit registers excluded (GUIDANCE.md) |
| P66 RODCOMP initial rate update | Consume switch count, multiply scale, update VDGVERT | Direct native translation with six retained words | Verified bounded calculation | Original bank 32,3275..3302; PIPA/interpreter continuation excluded |
| Guidance INHINT / scratch / physical input | Original inhibition and A/L persist into later guidance | Owned native scratch, caller exclusion and restoring target guards | Approximate | Original INHINT checked independently; no live switch source or full P66 context |
| RODCOMP PIPA/time snapshots | DCA/DXCH and CA/XCH retain prior inputs and time-register debris | Direct owned-state exchanges | Verified bounded stage | Complete snapshots and A/L at 32,3315 match original; asynchronous inputs held |
| RODCOMP vector-input preparation | Raw AD/TS stores, six skip effects, MODE alias and retained scratch | Native accumulator operations and conditional next loads/CAF | Verified bounded stage | 3396 checkpoints; all 64 skip masks in both sign families; final boundary before TC INTPRET |
| P66 VXSC KPIP1 | DMPSUB partial products, VROUND, MPAC/MPTEMP/OVFIND/A/L | Native fixed-scalar vector arithmetic | Verified bounded operation | 3364 complete 41-word checkpoints, exact-half/carry cases, 292 C64 states; interpreter control and RELINT adaptation in SCALING.md |
| P66 PDDL THISTPIP | Six-word vector pushdown, timestamp load, MODE/PUSHLOC/MPAC/A/L | Owned 43-word VAC with explicit FIXLOC/PUSHLOC | Verified bounded operation with native scope check | 3496 complete 86-word checkpoints, all five VACs/38 offsets, 200 C64 states; invalid-domain and interpreter-control adaptations in PUSHDOWN.md |
| Native preparation return mask / context ownership | Original state and PC encode skips; MPAC is Executive scratch | Six-bit observation and separate explicit scratch state | Approximate | No new historical word/alarm; saved Executive context is not implicitly updated |
| Real-time timing | AGC machine timing and task deadlines | Native code executes at C64 rate | Approximate | Console/IRQ/lifecycle/WAITLIST/restart observations plus six numerical/rate samples; no proved worst-case or flight deadline |

No Executive ready-job selection is replaced with a generic queue, dynamic allocator,
host thread, or AGC emulator. Native test fingerprinting is diagnostic overhead
and does not participate in Apollo scheduling.

Domain exclusions: malformed raw words, zero packed priority, +0 in a core
PRIORITY, corrupted VAC addresses, arbitrary register debris at public entry,
and arbitrary concurrency are not equivalence claims. The documented one-IRQ /
foreground publication contract is separately tested. The fixture initializes these states
deliberately. No test purports to reproduce the Apollo 11 landing alarms in their
mission context.

## Alarm/recovery milestone classification

| Behavior | Classification | Evidence/limit |
|---|---|---|
| Bounded ALARM1/BAILOUT1/POODOO1 recording | Faithful within stated entry/state domain | FAILREG/ALMCADR/lamp memory and full retained model match original; negative first/second FAILREG excluded |
| STARTSB1 cleanup | Faithful modeled state | All modeled resource/display/service/device writes and retained payloads match original; no physical Apollo devices |
| Channel-13 trap latches/readback | Faithful modeled device semantics | Real original instructions and Virtual AGC CPU I/O API, explicit separate latches |
| Protected-pair loop and Type C variable job route | Faithful bounded stages | Original scratch, signed priority, VAC reservation and admission failures compared |
| Composed `rs_recover` and runtime reinitialization | Adaptation | Omitted ENEMA/display/flag/bank-return glue; native callbacks restart from mappings |
| POODOO phase/service cleanup, table/Type B/display/timed/longcall routes, phase updates and hardware GOJAM | Unimplemented | Unsupported forms rejected; phase corruption stops before PTBAD body |

The original primitive bailout markers still do not imply recovery by themselves.
Explicit bounded recovery and runtime ownership requirements are in RESTART.md.

## DSU PIPTIME bounded fidelity

The arithmetic and modeled memory/register effects are verified for the declared
DP raw-word domain: 4664 complete 89-word checkpoints and 172 C64 final states.
Signed zeros, low carry, independent word signs, BUF, conditional OVFIND writes
and raw A/L match actual original execution. Both signs of initial overflow
resolved without OVFIND writes are explicitly covered. A returned path mask,
owned scratch/stable PIPTIME and restoring platform guard are native adaptations.
Interpreter bank/control transfers are asserted by the oracle but are not native
resume state. STORE 30D, DDV, live producers and full scheduling are outside this
operation. See ELAPSED.md.

## STORE 30D bounded fidelity

The two stored raw words, retained MPAC/scratch/VAC state, clear A and old-word L
match 5044 complete original-assembly checkpoints and 176 C64 expectations.
All five VACs and legal push positions include deliberate overlaps at slots
30/31. Unsupported MODE/FIXLOC rejection, object ownership and restoring I are
native adaptations. PUSHLOC is unused. Interpreter dispatch/bank/control state
is oracle-only; later DDV/PDVL and live flight scheduling are excluded (STORE.md).

## Fixed-divisor DDV fidelity and oracle qualification

TPAGREE, normalization, zero/ordinary/overflow exits and modeled register/scratch
state are verified over 8098 complete checkpoints using the pinned hardware-DV
model. That oracle uses unchanged assembly and a generated engine copy with one
branch selecting existing SimulateDV for every DV. The pinned fast mathematical
DV shortcut mishandles sign-extended L=-0; the audit records 798 differing rows,
only quotient words and L. The pinned checkout and earlier oracles are unchanged.
Native specialization for {0,400}, owned scratch/MODE rejection, path enum and
platform restoring I are explicit adaptations. General divisors, interpreter
resume, pending PDVL and physical Apollo hardware validation are outside scope.
The twelfth C64 PRG passes 156 full 93-word states; see DIVIDE.md.
