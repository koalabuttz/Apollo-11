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
| Alarm light / FAILREG / abort route | BAILOUT1 body, display state, WHIMPER/ENEMA | No implementation | Not Implemented | Tests stop before BAILOUT1 executes |
| Native bailout latch | Apollo exits admission into abort/recovery | Further admission blocked until caller reinitializes | Approximate | Explicit fail-stop boundary protects unsupported continuation; no Apollo restart claim |
| Basic CHANJOB state exchange | Swaps twelve words, encodes overflow, updates FIXLOC and dispatch scratch | Same saved transitions and explicit returned 2CADR | Verified Behavioral Equivalent | 109 original-derived checkpoints, including all candidate positions and both zeros |
| Native basic execution | DTCB transfers Z/BBANK and selected SUPERBNK | Complete 2CADR lookup calls ordinary C continuations | Approximate | Hardware transfers checked in oracle; C stack/ABI replaced explicitly; eight actual native two-job sequences pass host/C64 |
| Interpreter dispatch | Negative saved LOC resumes through INTRSM | Rejected before core/resource mutation | Not Implemented | Basic-only domain is explicit |
| Basic sleep/wake / priority change | Dormant encoding, first matching CADR, 2CADR reconstruction, packed PRIOCH2/CCS scan | Same saved transitions; explicit callback sleep/priority actions and foreground wake | Verified Behavioral Equivalent | 81 cases / 115 complete original checkpoints, five native sequences; C ABI/entry scratch adapted as documented in LIFECYCLE.md |
| Interrupt exclusion / ordering | INHINT/RELINT, AGC hardware, polling/yield machinery | Restoring C64 IRQ guards; bounded publication then foreground admission; cooperative yield | Approximate | Real raster IRQs preserve compiler registers and interrupted C; not AGC interrupt/task timing |
| IRQ admission buffer | Requests can originate in interrupt/task routines | Four FIFO requests, overflow fail-stop separate from 1201/1202 | Approximate | Explicit platform adaptation; host wrap/fault tests and real C64 overflow; no generic ready-job queue |
| WAITLIST list / task completion | TIME3, nine task slots, differential intervals, T3RUPT2/TASKOVER | Same persistent list/counter state, ordered insertion and basic task transfers | Verified Behavioral Equivalent | 196 cases / 306 original checkpoints, ten native sequences; scratch/ABI boundary in WAITLIST.md |
| CIA timer / native task timing | Centisecond counter, interrupt-task entry with I inhibited | CIA1 B publishes two-centisecond samples; foreground mapped callbacks run excluded | Approximate | Six actual CIA IRQs, register/decimal preservation, periodic sample and task/job bridge; SVCT3 body and hardware RESUME excluded |
| Restart/recovery | Phase groups and protected jobs rebuilt after abort | No implementation | Not Implemented | Separate future validation milestone |
| Fixed point / multiword math / special registers | Guidance-specific scales and register/instruction effects | No implementation | Not Implemented | Only initial Executive arithmetic is provided |
| Real-time timing | AGC machine timing and task deadlines | Native code executes at C64 rate | Approximate | Four console, 74 IRQ, 76 lifecycle and 156 WAITLIST operation samples plus IRQ/timer/latency observations; no proved worst-case or flight deadline |

No Executive ready-job selection is replaced with a generic queue, dynamic allocator,
host thread, or AGC emulator. Native test fingerprinting is diagnostic overhead
and does not participate in Apollo scheduling.

Domain exclusions: malformed raw words, zero packed priority, +0 in a core
PRIORITY, corrupted VAC addresses, arbitrary register debris at public entry,
and arbitrary concurrency are not equivalence claims. The documented one-IRQ /
foreground publication contract is separately tested. The fixture initializes these states
deliberately. No test purports to reproduce the Apollo 11 landing alarms in their
mission context.
