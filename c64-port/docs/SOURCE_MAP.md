# Source-to-translation mapping

All line numbers refer to upstream commit
`911e5c0283c629c50cb97666f34065e8c07d71a5`. Historical assembly is unchanged.

| Source / labels | C implementation | Verification boundary |
|---|---|---|
| EXECUTIVE:37–60 NOVAC, FINDVAC, SPVACIN | `exec_admit` argument adaptation and NEWPRIO/NEWLOC setup | Original public entry executed; no C bank-switch implementation |
| EXECUTIVE:131–153 FINDVAC2, VACFOUND | `exec_admit`, ordered VAC search/reservation | ENDFIND success or BAILOUT1 before alarm body |
| EXECUTIVE:155–170 NOVAC2, NOVAC3, NO.CORES | `exec_admit`, ordered eight-position -0 search | Counter's inclusive-zero behavior verified with eighth-slot tests |
| EXECUTIVE:167–199 CORFOUND, SPECTEST, SETLOC | `exec_admit`, priority, pointer, locations, NEWJOB | Saved state immediately before ENDFIND |
| EXECUTIVE:200–208 NEXTCORE / 1202 | `exec_admit`, capacity failure | BAILOUT1 entry; no rollback or restart |
| EXECUTIVE:371–385 ENDJOB1 | `exec_endjob_select`, current core/VAC release | End of following selection, before dispatch |
| EXECUTIVE:388–457 EJSCAN, EJ1, EJ2 | `exec_endjob_select`, ascending scan of positions 1..7 | CHANJOB-2 for a selected job; DUMMYJOB entry for idle |
| EXECUTIVE:72–75 CHANG1; 212–273 CHANJOB/ENDPRCHG | `exec_change_basic`, `exchange_basic` | Original CHANG1 through DTCB; saved words and actual target banks |
| EXECUTIVE:293–303 NUCHANG2; 483–486 NUDIRECT | `exec_start_basic` | Bounded idle start with explicit A=2/L=0 |
| EXECUTIVE:371–457 ENDJOB1/EJSCAN plus CHANJOB-2 | `exec_complete_basic` | Original release, scan, swap and basic DTCB |
| EXECUTIVE:90–93,281–291 JOBSLEEP/JOBSLP1/2 | `exec_sleep_basic` | Public JOBSLEEP through selected physical transfer or bounded idle |
| EXECUTIVE:97–103,310–353 JOBWAKE/JOBWAKE2/3/4/WAKETEST | `exec_wake_basic` | Public JOBWAKE through ENDFIND; caller L=0, complete saved state |
| EXECUTIVE:107–113,358–366 PRIOCHNG/PRIOCH2 | `exec_priority_basic` | Public PRIOCHNG through actual Q/BBANK/SUPERBNK transfer; CCS threshold retained |
| EXECUTIVE:464–465 DUMMYJOB initial stores | `exec_complete_basic` idle return | After NEWJOB=-0, before RELINT and idle activity |
| FRESH_START_AND_RESTART:507–529 resource initialization stores | `exec_init` initial priority/use words | Adapted test initializer; zeroing extra words is separate adaptation |
| ERASABLE_ASSIGNMENTS:376–383 | `executive.core[8][12]`, field constants | Matching logical layout, native physical addresses differ |
| ERASABLE_ASSIGNMENTS:434–443 | `executive.vac[5][44]` | Historical use addresses retained as word values |
| EXECUTIVE:126 FAKEPRET | `EXEC_FAKEPRET` (octal 0110) | Derived from MPAC=0154 minus 36 decimal, fixture checks MPAC |

Reference adapter addresses, all octal: NOVAC=5072; FINDVAC=5105;
ENDFIND=5160; BAILOUT1=5716; ENDJOB1=bank 01,3103;
CHANJOB-2=bank 01,2704; DUMMYJOB=bank 01,3206; CHANG1=5122;
ENDPRCHG=bank 01,2765; NUCHANG2=bank 01,3011; NUDIRECT=bank 01,3225.
JOBSLEEP=5133; JOBWAKE=5137; PRIOCHNG=5146; JOBSLP1=bank 01,2776;
JOBWAKE2=bank 01,3023; PRIOCH2=bank 01,3072; EJSCAN=bank 01,3120.
The setup helper asserts selected symbol addresses against the assembler listing.

## Task scheduler mapping

| Original source / labels | Native implementation | Verification boundary |
|---|---|---|
| WAITLIST:129–144 public WAITLIST/DLY2/LVWTLIST; 203–240 WAIT2 | `wl_schedule` | Public entry to LVWTLIST before hardware return; TIME3/list words |
| WAITLIST:245–271 WTLST4; 274–336 WTLST5/WTABORT/FILLED | `wl_schedule` | Ordered shifts/equal deadlines; original 1203 BAILOUT1 entry |
| WAITLIST:348–361 WTLST2 | `wl_schedule` | Raw interval split and CCS zero branches |
| INTERRUPT_LEAD_INS:47–50 T3 lead-in; WAITLIST:380–417 T3RUPT/T3RUPT2 | `wl_begin`, explicit native pending latch | Actual selected Z/BBANK/SUPERBNK; original BANKRUPT save checked |
| WAITLIST:424–439 TASKOVER/RESUME | `wl_taskover` | Due task transfer or restored banks before RELINT/hardware RESUME |
| WAITLIST:561–564 WAITPOOH/1204 | `wl_schedule` | POODOO1 entry before alarm body |
| FRESH_START_AND_RESTART:399–400,475–505 | `wl_init` resource markers | Deterministic native initializer; SVCT3 body excluded |
| yaAGC counter hardware model | `wl_advance` | Controlled PINCs against real CounterPINC, including positive overflow reset |

Checked fixture addresses, octal: WAITLIST=5203; LVWTLIST=5215;
WAIT2=bank 01,3231; ENDTASK=5236 (positive pair 5240/4063);
T3 lead-in=4014; T3RUPT=bank 01,3407; T3RUPT2=bank 01,3414;
TASKOVER=5261; original return stop=5275; LST1=E3,1400;
LST2=E3,1410; TIME3=0026; RUPTAGN=0734.
WAITLIST.md describes the excluded scratch aliases, native ABI and CIA adaptation.

## Deferred routines

SPVAC, CHANG2, interpretive ENDPRCHG/INTRSM, NUCHANG2 race/activity handling,
full DUMMYJOB/ADVAN polling/self-check, general SUPDXCHZ, ENDFIND's return,
TWIDDLE/FIXDELAY/VARDELAY/LONGCALL wrappers, SVCT3's body, hardware interrupt
RESUME remains untranslated. Separate bounded alarm/cleanup/phase/job stages
are mapped below and documented in RESTART.md. `exec_endjob_select` is not ENDOFJOB's complete calling
convention, and a selected index alone does not execute a routine. `exec_native` now runs
explicitly mapped native demonstration jobs; see DISPATCH.md for the adapter
boundaries and the separate interpreter exclusion.

## Arithmetic mapping

| Relevant AGC semantics | Native primitive | Evidence |
|---|---|---|
| 15-bit complement / CS on a data word | `agc_negate` | Exhaustive involution and signed conversion |
| Sign-extended A/L representation | `agc_extend` | Exhaustive all 32768 words against yaAGC helper |
| Erasable sign-bit overflow correction | `agc_correct` | Exhaustive all 65536 register patterns against helper |
| AD with end-around carry in 16-bit accumulator | `agc_add_register` | All word encodings against 16 boundary operands |
| CCS's word class and diminished absolute value | `agc_classify`, `agc_diminished_abs` | Exhaustive analytical checks, Executive execution comparisons |
| BZMF accumulator sign / both zeros | Explicit `exec_admit` branch test | Tested packed-priority comparisons |
| Word-sized one's-complement addition | `agc_add_word` | Separate analytical modular-arithmetic oracle; not a substitute for AD/TS |
| MP with an ordinary memory operand | `agc_multiply` | Signed zeros, corrected A and DP product; actual opcode comparisons |
| DAS to an ordinary erasable pair | `agc_double_add` | Low carry, independent word signs and returned overflow; actual opcode comparisons |

Register CCS with overflow, register-target TS, edit registers, division
and interpreter operations are not implemented. MP/DAS are bounded to normal
memory operands; guidance scales remain raw words (GUIDANCE.md).

## Alarm and protected Type C job stages

| Original source/labels | Native entry | Oracle boundary |
|---|---|---|
| ALARM_AND_ABORT: ALARM1/ALMNCADR/BAILOUT1/POODOO1, BOTHABRT, CHKFAIL1/2, FAIL3, PROGLARM, MULTFAIL/MULTEXIT | `rs_record` | Q+1, WHIMPER, or OCT77770+1 before phase writes |
| FRESH_START_AND_RESTART: STARTSB1/STARTSB2/DSPOFF | `rs_cleanup` | TC Q return, retained payloads/device readbacks/trap latches |
| FRESH_START_AND_RESTART: GOPROG3/PCLOOP | `rs_validate` | MMDSPLAY or PTBAD entry |
| RESTARTS_ROUTINE: RESTARTS/ITSAVAR/ITSAJOB/CHKNOVAC/ITSNOVAC, FINDVAC/NOVAC | `rs_job` | ENDFIND or BAILOUT1, with original MPAC/VAC5 GOLOC scratch |
| Bounded native composition of these stages | `rs_recover` | Integration adaptation; not complete original ENEMA execution |

RESTART_TABLES and PHASE_TABLE_MAINTENANCE are traced and included unchanged in
the fixture; no table route or phase-update primitive is translated. RESTART.md
records caller inputs, valid Type C encodings, exclusions and fixture addresses.

## P66 rate adjustment

`LUNAR_LANDING_GUIDANCE_EQUATIONS.agc:948` maps RODCOMP's INHINT/CAF/XCH/EXTEND/
MP/DAS prefix to `p66_rate_update`, entering bank 32,3275 and stopping at
32,3303 before PIPA work. RODCOUNT/RODSCAL1/VDGVERT are E7,1746/E7,1756/E7,1644.
STARTP66/STRTP66A and DESCBITS are traced for scale/count provenance, not ported.
GUIDANCE.md defines full numerical domains, original register results and the
restoring native interrupt adaptation. The subsequent interpreter is excluded.

| Original RODCOMP instructions | Native entry | Checkpoint |
|---|---|---|
| Lines 955..966: EXTEND/DCA PIPAX, DXCH OLDPIPAX/RUPTREG1, CA/XCH PIPAZ/OLDPIPAZ/RUPTREG3, EXTEND/DCA TIME2, DXCH THISTPIP | `p66_snapshot` | 32,3315 before CA OLDPIPAX; prior time in A/L |
| Lines 968..999: CA/AD/TS MPAC components, CS/AD/AD/TS DELVROD components, CAF/TS low/TEM stores, CS ONE/TS MODE | `p66_vector_inputs` | 32,3353 before TC INTPRET; raw A/L and all six skips |
| Full basic RODCOMP prefix from line 948 | `p66_prepare` | 32,3353; composition of rate, snapshot and vector-input stages |

PREPARATION.md records all physical addresses, MODE's alias, retained words and
overflow effects. The original instructions execute independently in the host
oracle; direct native assignments fold only unobservable exchange intermediates.

## First P66 interpretive vector operation

| Historical source | Native translation | Boundary |
|---|---|---|
| LUNAR_LANDING_GUIDANCE_EQUATIONS:1001..1003, ITRPNT1 VXSC PDDL / KPIP1 | `p66_scale_velocity` after `p66_prepare` | First arithmetic operation only, before PDDL |
| CONTROLLED_CONSTANTS:175, KPIP1 | Fixed DP pair `{00321,26706}` | 33,2021; asserted original rope bytes |
| INTERPRETER:902..926, DMPSUB | Private component partial-product arithmetic using MP/DAS | All original product ordering and corrections |
| INTERPRETER:936..949, VROUND / SETOVF2 | Private component signed rounding | TS skip selection, low carry and A/L debris |
| INTERPRETER:1190..1212, VXSC / VVXSC / VROTATEX | Three native component calls | DANZIG 6061 before its first instruction |

LOC/BANKSET/ADDRWD/CYR/EDOP/Q/bank selectors are explicitly outside the native
41-word arithmetic contract. Genuine integration cases execute the original
interpreter entry; see SCALING.md for dispatch and interrupt adaptations.

## PDDL THISTPIP

| Historical source | Native translation | Boundary |
|---|---|---|
| LUNAR_LANDING_GUIDANCE_EQUATIONS:1001..1004, pending PDDL / THISTPIP | `p66_push_timestamp` after scaling | DANZIG 6061 before DSU PIPTIME |
| INTERPRETER:550..575, PDDL / ENDVPUSH | Six-word save, DP timestamp load, explicit exchange debris | Entry PDDL 6523, MODE=-1; exit MODE=+0 |
| ERASABLE_ASSIGNMENTS:434 onward, VAC payloads; FIXLOC/PUSHLOC | `p66_pushdown` with 43 words and two pointer/base words | Five supported VAC mappings, six available words required |

Native bounds rejection is an explicit scope adaptation, not an original alarm.
PUSHDOWN.md lists register/scratch preservation and excluded interpreter control.

## DSU PIPTIME

| Historical source | Native translation | Boundary |
|---|---|---|
| LUNAR_LANDING_GUIDANCE_EQUATIONS:1004..1006, DSU / PIPTIME | `p66_elapsed_time` after PDDL | DSU at 32,3357; stop DANZIG before STORE 30D at 32,3361 |
| INTERPRETER:800..815, DSU / ENDVXV / SETOVF | Word complements plus verified DP addition | DSU 7005, ENDVXV 7000, SETOVF 7003 |
| INTERPRETER:825..839,947..949, OVERFLOW / SETOVF2 | Exact BUF/sign realignment and conditional OVFIND write | OVERFLOW 7017; actual TS 7 skips at 7026/7033 |
| FIXED_FIXED_CONSTANT_POOL, LIMITS / POSMAX / NEGMAX | Same-sign maxima 037777/040000 | Assembled constants 04733/04735 independently asserted |

All 89 modeled words and observed paths match 4664 original checkpoints;
ELAPSED.md records the banked PIPTIME operand and excluded interpreter controls.

## STORE 30D

| Historical source | Native translation | Boundary |
|---|---|---|
| LUNAR_LANDING_GUIDANCE_EQUATIONS:1006, STORE 30D | `p66_store_elapsed` after DSU | STORE at 32,3361; stop DANZIG before DDV/PDVL at 32,3362 |
| INTERPRETER:404..420,453..475, DOSTORE / STORJUMP / STORE / STARTSTO | Fixed relative offset, DP write, old-word L and clear A | Genuine dispatcher and store at 6362/6372/6422/6434 |

The oracle asserts operand FIXLOC+30, MODE, LOC, BBANK and Q, reading all 89
modeled words before/after the store. STORE.md describes omitted control state.

## DDV 4SEC(28)

| Historical source | Native translation | Boundary |
|---|---|---|
| LUNAR_LANDING_GUIDANCE_EQUATIONS:1007..1009, DDV/PDVL / 4SEC(28) | `p66_divide_four_seconds` after STORE | DDV at 32,3362; stop DANZIG before PDVL GDT/2 |
| INTERPRETER:1359..1371,1783..1905 | Fixed divisor load, BUFZERO/sign agreement, overflow and normalization | DDV 7577; bank 00 DDV/BDDV 2353, BUFZERO 2363, DVOVF 2405, DVNORM 2505 |
| INTERPRETER:1068..1095,1976..2030 | TPAGREE word effects and two exact quotient stages | TPAGREE 7257; GENDDV 00,2570; FINALDV 00,2625 |
| SERVICER:1095, 4SEC(28) | Raw constant {0,00620} | Original declaration/source hash and assembled 33,3152 pair verified separately |

All 93 modeled words/results match 8098 assembly checkpoints using pinned
hardware DV. DIVIDE.md defines the one-line host-oracle selection and audit;
no claim of equality to the pinned fast-DV shortcut is made.

## PDVL GDT/2

INTERPRETER:589..602 and531..543 -> src/guidance_gravity.c.
PDVL6557/ENDVLOAD6511/VMODE6521 -> DANZIG6061 beforeVSU; operand physical1236
becomesADDRWD1636/EBANK01000. All99 modeled words independently verified.

## Verified native VAD/VSU

Reusable vector arithmetic now matches 13,600 original-assembly checkpoints.
128 complete states pass host, sanitizer and C64 diagnostics. See [VECTOR.md](VECTOR.md).

## Verified SPVAC and CHANG2

Staged VAC admission and interpretive context exchange match 768 complete
original-assembly checkpoints. Host/sanitizer tests and all 768 C64 state
fingerprints pass. Native mixed-context execution is a separate integration
stage. See [EXECUTIVE_SERVICES.md](EXECUTIVE_SERVICES.md).

## Translate native single and double phase-table changes

All 49,152 complete original-assembly states match; 128 host/sanitizer/C64 states pass. See [PHASE.md](PHASE.md).

## Translate arbitrary-factor vector scaling and unrounded DP products

All 8,192 complete assembly checkpoints and 128 host/sanitizer/C64 states pass. See [PRODUCT.md](PRODUCT.md).

## Translate scalar and vector shifts with original rounding and scratch effects

All 26,880 complete assembly states and 168 host/sanitizer/C64 states pass. See [SHIFT.md](SHIFT.md).

## Translate general double-precision DDV and backward division

All 163,840 original-assembly comparisons and 573 host/sanitizer/C64 states pass using the hardware DV model. See [GENERAL_DIVIDE.md](GENERAL_DIVIDE.md).

## Translate complete P66 updated-velocity calculation through STOVL and position load

All 4,648 complete assembly checkpoints pass; all 156 C64 states pass across two RAM-safe images. See [VELOCITY.md](VELOCITY.md).

## Translate scalar square root and normalized root with source error boundary

All 132,096 assembly comparisons and 637 host/sanitizer/C64 states pass; negative inputs stop at the declared pre-1302 boundary. See [NORM.md](NORM.md).

## Translate abort flags and selected restart-group cleanup paths

All 49,152 complete assembly comparisons and 128 host/sanitizer/C64 states pass. See [RECOVERY_CLEANUP.md](RECOVERY_CLEANUP.md).

## Translate unrounded triple-precision dot product and overflow accumulation

All 10,272 complete assembly comparisons and 96 host/sanitizer/C64 states pass. See [DOT.md](DOT.md).

## Translate scalar sums and backward vector subtraction with exact overflow effects

All 27,200 complete assembly comparisons and 256 host/sanitizer/C64 states pass. See [SUM.md](SUM.md).

## Execute native basic and interpretive continuations through original context exchanges

Native C callbacks now enter and exit interpretation, yield at completed operation pairs, restore interpreter payloads, and resume through the original Executive lifecycle. All 140 full original-assembly checkpoints agree; host and sanitizer tests and the PAL C64 diagnostic pass, including masked repeats and an actual raster IRQ admission. This milestone supplies scheduling plumbing; remaining guidance bodies still need translation and binding. See [INTERPRETER_NATIVE.md](INTERPRETER_NATIVE.md).

## Translate ENEMA through restart validation and source-defined exit boundaries

The native restart entry composes the verified cleanup and phase validation routines through the original MMDSPLAY, PTBAD, and fresh-start request boundaries. All 4,096 full 457-word original-assembly comparisons agree, and 128 complete C64 target states pass with host and sanitizer verification. Subsequent display and restart execution remain to be completed. See [ENEMA.md](ENEMA.md).

## Translate Comanche Executive admission and release with seven native core sets

A separate Comanche 055 Executive retains its seven core sets, five VACs, and source BAILOUT boundaries. All 1,536 source-derived assembly comparisons agree; host and sanitizer tests and the C64 run pass, including 40 complete-state samples. The generated fixture documents five indentation repairs and matches the pinned corrected full rope; historical repository files remain unchanged. CM dispatch and lifecycle are next. See [COMANCHE_EXECUTIVE.md](COMANCHE_EXECUTIVE.md).

## Translate absolute value, vector length and half-unit normalization with signed-zero fidelity

ABS, ABVAL and UNIT now preserve raw length, half-unit vector, overflow, and scratch effects. All 50,736 complete 104-word assembly comparisons agree, including sixteen signed-zero regressions found by P66 composition. Host and sanitizer tests and all 118 full-state C64 checks pass. Full application binding and remaining flight routines are still in progress. See [UNIT.md](UNIT.md).

## Compose native P66 radial velocity and predicted altitude from verified arithmetic

The P66 radial group now computes half-unit radius, radial velocity and predicted altitude with original VAC, signed-zero and scratch effects. All 4,952 complete 134-word original-assembly checkpoints agree; full-prefix host and sanitizer checks pass. Six C64 images verify 203 complete stage outputs from exact original pre-UNIT states. The full-prefix C64 diagnostic exceeds RAM, so whole-program overlay integration remains required. See [RADIAL.md](RADIAL.md).

## Translate P66 desired radial acceleration and gravity through the rotation boundary

The native P66 acceleration group preserves original VAC, scalar division, vector magnitude, gravity scaling and implicit stack pop effects through DANZIG before CDU rotation. All 16,384 full-stage assembly states and the 7,428-checkpoint composed host prefix agree. Strict host, sanitizer and 96 full-state C64 stage checks pass; whole-program overlay execution remains in progress. See [ACCELERATION.md](ACCELERATION.md).

## Execute bounded native disk overlays with canonical erasable storage on the C64

A resident Executive and loader execute separately linked native C payloads from a D64 image, swap and reload them after ordinary C returns, reset transient BSS, and preserve the 2,048-word canonical source image and platform mappings. Host bounds and error checks, sanitizers and the stock PAL C64 disk-loading proof pass. The demonstrated payloads test the ABI; full mission module allocation, media handling and guidance integration remain unfinished. See [OVERLAYS.md](OVERLAYS.md).

## Fix source-exact WAITLIST positive-zero control flow and final-slot insertion

WAITLIST now skips interval additions on early CCS positive zero and follows the final positive-zero insertion route instead of raising a false full-queue alarm. The 256-case full-word regression detects 688 previous field errors and passes after the fix; the original 196-case suite remains unchanged and passes. Host, sanitizer and all 256 C64 states agree with original assembly. See [WAITLIST_ZERO.md](WAITLIST_ZERO.md).

## Execute Comanche basic and interpretive jobs through source-exact lifecycle transitions

The separate seven-core CM runtime now dispatches native C callbacks, exchanges basic and interpreted contexts, sleeps and wakes jobs, and changes priorities through original Comanche semantics. All 876 complete source-derived checkpoints and twenty real native sequences agree; host and sanitizer checks and four stock C64 diagnostics pass. IRQ coordination remains caller-owned pending the dedicated adapter. See [COMANCHE_DISPATCH.md](COMANCHE_DISPATCH.md).

## Translate P66 operator counts, task admission and phase-timer entry paths

DESCBITS, RODTASK and P66VERTA now follow original operator arithmetic, priority-22 admission, phase termination, delayed task scheduling and immediate RODCOMP entry with exact shared scratch. All 73,728 original-entry checkpoints agree; host, sanitizer and 128 full-state C64 checks pass. STRTP66 bias initialization, display admission and later continuations are separate ongoing milestones. See [P66_STARTUP.md](P66_STARTUP.md).

## Port exact native trigonometric services

All 65,536 complete source states match CDULOGIC, SIN, COS and CDUTRIGS; all-state sanitizers and 128 frozen C64 cases pass. The native fixed-point coefficients and overflow behavior retain the original instruction semantics. See [TRIG.md](TRIG.md).

## Run live P66 continuations from native disk overlays

The canonical 2,048-word image drives actual compiled arithmetic callbacks through the mixed Executive. All 5,887 source checkpoints pass; the C64 disk proof passes 72 full-state fingerprints including two real P66 jobs, both IRQ states and SUPERBNK forms. The current driver ends before UNIT and retains its core/VAC for further translation. See [P66_NATIVE_DRIVER.md](P66_NATIVE_DRIVER.md).

## Port original native axis rotations

All 32,768 complete AX*SR*T source states match; strict and all-state sanitizer checks pass. C64 verification compares 128 full frozen states with source scratch aliases and both rotation directions preserved. See [ROTATION.md](ROTATION.md).

## Port live CDU projection and acceleration feedback

Live native trig and rotation feed DOT14 and AFC with original source scheduling boundaries. All 39,432 direct and full-prefix checkpoints match; entire-corpus sanitizers and all 144 frozen C64 checkpoints pass. See [PROJECTION.md](PROJECTION.md).

## Implement exact native hardware divide recurrence

The independently authored fixed-point recurrence matches 327,680 isolated original hardware-DV instruction states, including raw register overflow and signed zero. Full-state sanitizers and all 255 frozen C64 register-pair checks pass. See [HARDWARE-DIVIDE.md](HARDWARE-DIVIDE.md).

## Port source STOPRATE and FASTCHNG initialization services

All 1,024 complete source checkpoints match including return-register debris, physical banks and IRQ state. Strict and sanitizer checks and 128 complete frozen C64 states pass. See [P66_INITIALIZATION.md](P66_INITIALIZATION.md).

## Port original force feedback and throttle clamps

All 16,384 complete force-feedback states match unchanged assembly with all three clamp paths covered. Full-state sanitizers and 64 complete frozen C64 cases pass. See [THROTTLE_FEEDBACK.md](THROTTLE_FEEDBACK.md).

## Port complete bounded native THROTTLE service

THROTTLE+3 through THDUMP matches all 16,384 complete 147-word source states, including hardware-DV weighting, force saturation, critical branches, FASTCHNG and pulse commands. Strict and all-state sanitizer checks and 96 complete C64 states pass; the enclosing return bridge is a separate continuation. See [THROTTLE-SERVICE.md](THROTTLE-SERVICE.md).

## Port canonical whole-image P66 bias initialization

All 1,024 original STRTP66A entry cases match the complete 2,048-word erasable image plus physical SUPERBNK/IRQ. Strict and sanitizer ownership checks and 64 complete whole-image C64 states pass. See [P66_BIAS.md](P66_BIAS.md).

## Port canonical vertical-guidance startup through RODCOMP

All 1,024 complete original source images match through STOPRATE, flag updates, phase termination, timer admission and POSTJUMP. Strict and sanitizer checks and 64 whole-image C64 cases pass, including source resource bailouts. See [P66_VERTICAL_START.md](P66_VERTICAL_START.md).

## Port Comanche timer queue and native task execution

All 306 complete original WAITLIST checkpoints match, including zero-sign branches, source task transfers and 01203/01204 resource alarms. Strict, sanitizer and C64 checks cover all checkpoints, ten actual native callback sequences and ownership guards. See [COMANCHE_WAITLIST.md](COMANCHE_WAITLIST.md).

## Connect Comanche Executive to real native C64 interrupts

Exact source admission and callback states verify the CM runtime adaptation. C64 checks pass 512 admission fingerprints, seven real raster interrupts and 15 full callback states while preserving native registers, compiler storage, caller IRQ state and KERNAL coexistence. See [COMANCHE_IRQ.md](COMANCHE_IRQ.md).

## Port real P66 major-mode display job admission

NEWMODEX matches 1,024 complete original erasable images, admitting the actual DSPMMJB basic job. Strict, sanitizer and C64 checks pass 64 full images; deferred display formatting and consumption remain separate continuations. See [P66_MAJOR_MODE.md](P66_MAJOR_MODE.md).

## Compose complete native STARTP66 through RODCOMP

The complete source landing-start sequence matches 1,024 whole-memory entry states, with actual display-job and timer admission and both resource bailouts. Strict and sanitizer checks and 32 complete C64 images pass. Deferred display execution and the remaining landing cycle are separately tracked. See [P66_LAUNCH.md](P66_LAUNCH.md).

## Port complete native horizontal velocity arithmetic

All 65,536 original horizontal-velocity checkpoints match, preserving UNIT overflow continuations and physical bank effects. Strict and all-state sanitizer checks and all 256 complete frozen C64 states pass. See [HORIZONTAL.md](HORIZONTAL.md).

## Port actual P66 major-mode display formatting

DSPMMJB matches all 1,024 original whole-memory states across every decimal mode and relay dirty/clean branch. Strict and sanitizer checks and 64 complete C64 images pass through the actual ENDOFJOB entry; relay drain remains a separate service. See [P66_MODE_DISPLAY.md](P66_MODE_DISPLAY.md).

## Port original canonical throttle call and return bridges

The actual source EXIT/BANKCALL and THDUMP/SWRETURN/INTPRET paths match all 8,192 complete erasable images. Strict and all-state sanitizer comparisons and 64 whole-image C64 checks pass, with unsupported-boundary no-mutation guards. See [THROTTLE_BRIDGE.md](THROTTLE_BRIDGE.md).

## Port Comanche phase records and restart foundation

CM PHASCHNG phase handling, STARTSB2/STARTSUB cleanup, GOPROG3 and Type C restart admissions match all 50,432 complete source checkpoints. Strict, sanitizer and stock C64 checks pass; actual restart-selected execution is a separate continuation. See `docs/COMANCHE_PHASE.md` and `docs/COMANCHE_RESTART.md`.

## Execute Comanche restart-selected native callbacks

All 320 complete original restart/dispatch states match across 64 source sequences and 192 actual native callbacks. Strict, sanitizer and stock C64 checks preserve caller frames and channel11 readback. The documented test callback bodies establish runtime composition; arbitrary mission bodies still require translation. See `docs/COMANCHE_RESTART_NATIVE.md`.

## Bind Comanche runtime to the canonical erasable image

The sole 2,048-word image supplies CM Executive, WAITLIST, phase and restart views. Poison/regather tests preserve all 320 source checkpoints and execute 192 native callbacks; strict, sanitizer and stock C64 checks pass without a second persistent guidance image. See `docs/COMANCHE_NATIVE_IMAGE.md`.

## Port Comanche ENEMA error and fresh-start paths

All 4,096 complete original ENEMA states match across 439 owned words, including navigation/error combinations, counter startup and conditional TVCEXEC scheduling. Strict and sanitizer checks and stock C64 fingerprints plus full-state samples pass; downstream selected mission tasks remain separate translation work. See `docs/COMANCHE_ENEMA.md`.

## Split source-faithful guidance into returning native phases

All 97,558 complete UNIT/radial/projection source states and full sanitizer corpora match. All 480 C64 phase entry/exit cases pass with zero extra zero page. Internal C ABI returns retain ownership; scheduling occurs only at original source boundaries. See `docs/GUIDANCE_PHASED.md`.

## Port original DSKY relay drain and channel10 output

All 1,024 whole-memory DSPOUTSB source states and actual channel10 output events match. Strict and sanitizer checks and 64 complete C64 images pass; the native screen adapter and live application wiring remain separate integrations. See [DSKY_RELAY.md](DSKY_RELAY.md).

## Render actual native DSKY relay output on the C64

The source mode66 display job and channel10 relay drain now drive real C64 screen/color RAM. Host, sanitizer and target checks verify numeric/sign latches, lights, blanking and the128cs flash cycle; live application keyboard/device integration remains pending. See [DSKY_DISPLAY.md](DSKY_DISPLAY.md).

## Port original Comanche startup and cleanup entries

SLAP1/DOFSTART and MR.KLEAN/P00KLEAN/V37KLEAN match all640 complete source checkpoints, preserving2047 image words and15 external I/O fields with native Z boundary qualification. Strict and sanitizer checks and640 C64 fingerprints plus five full-state samples pass. See `docs/COMANCHE_STARTUP.md`.

## Split acceleration at source boundaries and preserve return control

All65,536 complete acceleration phase states and65,536 independent source Q observations match. Full-state sanitizers and128numerical/65control frozen C64 cases pass. Separate native phase APIs support genuine source scheduling boundaries. See `docs/ACCELERATION_PHASED.md`.

## Port genuine P66 velocity scheduling boundaries

Three native velocity groups stop at source LOC03366,03370 and03373 with EDOP zero. All13936 complete118-word source checkpoints match SHA977279d21f48ea76643d805554338b2d4fb8fb9b8da9f72a80952c9a2d6c912d; strict/sanitizer checks and96 actualC64 entry/exit states pass. LOC03371 retains pendingSTADR and cannot hand off. C64 images17922/17035/16086bytes use no additional zero page, at most20 hardware and47 software stack bytes. The owning canonical driver supplies scheduling/control state and producer exclusion. See `docs/P66-VELOCITY-PHASES.md`.

## Port P66 feedback through six source-aligned overlays

All98304 complete124-word original feedback checkpoints match SHA2681746eff8644b4b4ff08531cc7cc9d4d1eddd11d8cc943ce30d98a7c76bd76. Strict192 source entry/exit states and196602 invalidMODE checks, full-source sanitizers andsix actualC64 images pass. Source LOC03437 still has pendingPDDL inEDOP00052 and retains job ownership; canonical scheduling applies only at genuine completed source boundaries. Equivalent raw-sign predicates preserve the complete earlier monolithic source corpus. See `docs/P66-PHASED-FEEDBACK.md`.

## Port P66 horizontal-exit display control

All8192 complete2048-word source images plusSUPER/IRQ match SHA b77721d0f877bf7d71452a34837d05cc72914aadc78fbc8dc22759d0a251571b. Native EXIT/BANKCALL/DISPEXIT+3 follows suppression to actualENDOFJOB or enabled output to originalREGODSPR, preserving NV/BUF2 and source registers. Strict64 states/14 invalid states, full-source sanitizers andactualC64 64 states pass; PRG4178bytes, zero additional zero page, hardware14 andsoftware12 stack bytes. See `docs/P66-DISPLAY-CONTROL.md`.

## Port actual P66 REGODSPR display-job admission

All8192 complete2048-word source images plusSUPER/IRQ match SHA d1d8bd7dea68056cc34531153360729db463ee886ee7999014e2b61a8ee62fba. Native REGODSPR/NOVAC/SAVELOCR/BANKJUMP copies NV/MPAC2/CADR/flags into originalMAKEPLAY job storage and reaches actualENDOFJOB;1024 no-core cases reach originalBAILOUT1. Strict64 states/18 invalid states, full-source sanitizers andactualC64 64 states pass; PRG11904bytes, zero additional zero page, hardware18/software678 stack bytes. Display formatting follows in separate native source services. See `docs/P66-REGODSPR.md`.

## Preserve separate P66 STORE20 and acceleration-add handoffs

The separate STORE20 and DAD groups stop at genuine source LOC03421 and03422. All 81,920 complete 112-word original checkpoints match SHA d0b4c030a35450fb4ea75dbc906600ad163b041b004060473d4430fab002788d; strict 64 source states, 65,534 invalid MODE cases and full-source sanitizers pass. Two actual C64 images pass 32 original states each, with zero additional zero page and at most 14 hardware / 371 software stack bytes. The earlier combined sum API and retained numerical evidence remain unchanged. See `docs/P66-ACCELERATION-STORE.md`.

## Port original P66 noun-verb entry dispatch

Native NVMONOPT reaches UPDATVB-1 or the actual busy caller return. All 8,192 complete 2,048-word source images plus SUPER/IRQ match SHA 188f356b2c434205fc424a1eca0b54097d83576087b8209ea8f253276155c8fe. Strict 64 source states / 13 invalid entries, full-source sanitizers and 64 actual C64 source states pass. The 4,367-byte PRG uses no additional zero page and 14 hardware / 12 software stack bytes. Formatting and output continue in separately verified services. See `docs/P66-NV-ENTRY.md`.

## Port source P66 verb and noun update services

Original UPDATVB and UPDATNN for V06N60 match all 8,192 complete 2,048-word source images plus SUPER/IRQ, SHA 29c140f45d7a4d5ff1c2d3f2b18759bdc6e584c1370d196e9b6f071af3b4ece9. Strict 64 source states and 20 invalid entries, full-source sanitizers and 64 actual C64 source states pass. The 11,091-byte PRG uses no additional zero page and 33 hardware / 47 software stack bytes. This milestone preserves source continuation boundaries for later data formatting. See `docs/P66-NV-WORDS.md`.

## Port original DSKY relay character and blanking services

Native DSPIN,11DSPIN and2BLANK match all 4,096 complete 2,048-word original-source images plus SUPER/IRQ, SHA 65097a2bedb65f7a5ebf81f42b101107b2e51ced4ae5aed81fd510e05bef0180. The entry domain includes all eight retained erasable banks, ordinary caller returns and source character/sign/blank counts; original clean-positive-zero CCS holes remain explicitly excluded. Strict and sanitizer checks and 64 actual C64 source images pass. The 9,859-byte PRG uses zero additional zero page and 10 hardware / 9 software stack bytes. Earlier EB2-only proof artifacts are retained. See `docs/DSKY-PINBALL.md`.

## Port source five-digit signed DSKY decimal formatting

Original DSPDECWD and DSPDECNR match all 4,096 complete 2,048-word source images plus SUPER/IRQ across all eight retained erasable banks, SHA c10d492637f9b2fe2cc0fd4dd5a6513c7921ce92e442bdb09da09fea0476ccc2. Native formatting preserves rounded and unrounded sign handling and all five decimal characters through the actual caller return. Strict checks, the entire sanitizer snapshot stream and 64 actual C64 source images pass. The 16,051-byte PRG uses zero additional zero page and 13 hardware / 33 software stack bytes. Earlier EB2-only proof artifacts remain immutable. See `docs/DSKY-DECIMAL.md`.

## Port source DSKY double-precision output scaling

Original DP1,DP2 andDP3OUTSF match all 4,096 complete 2,048-word source images plus SUPER/IRQ, SHA 5f1679fb963ea1e13e4286e247b0f8bbb7ed4f1197b22c6af9489cc897ea472b. Native fresh READLO, mixed-sign TP agreement, DMP and rescaling reach actual DSPDCEND bank41. The documented domain retains all eight initial erasable banks and normal/mixed noun addressing. Strict and sanitizer checks and 64 actual C64 source images pass; PRG14,508 bytes, zero additional zero page, hardware13/software122 stack bytes. See `docs/DSKY-SCALE.md`.

## Port original DSKY two-digit noun-verb formatter

Native DSPDECVN matches all 4,096 complete 2,048-word source images plus SUPER/IRQ, SHA 284719c9194b0c106ff4c0e79ed0000adbb28450d928f9297c606afb51fdc924. Original SHORTMP consumes retained MPAC+2, formats values0..99 into adjacent source display characters and returns through the actual caller, with all eight retained erasable banks covered. Strict and sanitizer checks and 64 actual C64 source images pass. The 13,289-byte PRG uses zero additional zero page and 13 hardware / 40 software stack bytes. See `docs/DSKY-TWO-DIGIT.md`.

## Qualify shared native arithmetic against original Comanche assembly

Original Comanche SQRT,DDV andUNIT match 132,096 / 163,840 / 50,736 complete source states. Their unchanged native kernels and existing frozen vectors pass host/sanitizer checks and 1,328 complete C64 vectors. CM dispatch addresses, signed-zero hardware-DV oracle qualification and indentation-only original-source assembly repairs are audited separately; whole normalized-original CM rope matches the independently pinned assembly SHA2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79. This proves bounded arithmetic reuse; surrounding CM program coverage remains pending. See `docs/COMANCHE_NUMERICS.md`.

## Port Comanche-specific AX*SR*T rotation scratch semantics

Original Comanche TERM1TMP aliases BUF2 rather than Luminary MPAC+3/+4. A distinct native CM rotation preserves the original scratch and retained MPAC words. All 32,768 complete 131-word source states match, with strict host/sanitizer and 128 complete actual C64 states passing. The 45,659-byte diagnostic remains below ROM, uses no additional zero page and 23 hardware / 570 software stack bytes. Original CM and LM kernels retain separate verified semantics. See `docs/COMANCHE_ROTATION.md`.

## Port original Comanche RECTIFY and MINIRECT conic reset

Twelve explicit native groups reproduce original RECTIFY/MINIRECT position and velocity deviation shifts, conic stores, clears and RVQ return. All 24,064 complete 2,048-word source images match SHA 7b93fc96fd7e845b9fec26c64cbfd48e96ba80dc472e1990cd21590381faee23, with all-source sanitizer equality and 384 complete actual C64 phase states passing. Source DANZIG boundaries permit Executive handoffs; final RVQ returns to NEWOPS without introducing a job check. All five VACs, eight initial erasable banks, fourteen return fixed banks and Earth/Moon/raw corners are covered. Maximum target image ends0557C, with hardware20/software170 stack bytes and zero additional zero page. See `docs/COMANCHE-RECTIFY.md`.

## Run the live P66 Executive through the acceleration command

The live native Executive now runs RODCOMP through completed AFC/DELVROD at LOC03433 using 23 source-aligned callbacks and 27 disk-loaded arithmetic modules. All 192 C64 source fingerprints pass, including real higher-P66 preemption, both SUPER forms and enabled/masked native interrupts. The resident ends at $6fb9; overlays fit16KB with zero BSS/noinit, the canonical image remains authoritative and measured software stack is736/2048bytes. Independent original-source, sanitizer and first-legal pending NEWJOB comparisons pass at all22 handoffs. Actual resident PRG and D81 media are retained. Feedback, throttle, horizontal and final display lifecycle remain subsequent work. See `docs/P66-LIVE-GUIDANCE.md`.

## Port Comanche DIFEQ Nystrom numerical phases

Original DIFEQ+0/+1 with DIFEQCOM through STORE ALPHAV, and DIFEQ+2 through STORE ZV, run as explicit native phases. All 19,840 complete 2,048-word original states from 1,792 calls match SHA 9b8b544ee7eac53ad9ac77fa6f27e4d980a23b0e3dcf22d7843e9ff4607a3ecf, including raw index overflow corners. Full-source sanitizer equality and 960 original phase entry/exit C64 states pass across 30 images. Genuine DANZIG returns preserve scheduling semantics; STCALL-to-NEWOPS internal transitions retain ownership. Actual PRGs, maps, mailboxes and stack dumps are retained; maximum image ends05F36, hardware20/software177 stack bytes, zero additional zero page. Integrator control surrounding these stores remains subsequent work. See `docs/COMANCHE-DIFEQ.md`.

## Port Comanche DIFEQ0 initialization and source JSWITCH routing

Native orbital initialization and JSWITCH BON/GOTO routing reproduce all 5,120 complete 2,048-word original images plus IRQ from 1,024 calls, SHA 5a0688f511a9c6097a5aeafe9b8ac628c6ff9598345b029c40e0471e0d1cb37c. Strict and full-source sanitizer checks plus four actual C64 images containing 128 original phase entry/exit states pass. Phases0..2 reach genuine DANZIG; phase3 restores IRQ and returns to NEWOPS at original DOW/ACCOMP without inserting a job check. Maximum target image ends0409B, hardware20/software44 stack bytes, zero additional zero page. Actual PRGs and all original-source proof artifacts are retained. See `docs/COMANCHE-ORBITAL-INITIAL.md`.

## Correct DSKY positive-zero blanking and NOUT increment overflow

Original 2BLANK skips INCR NOUT for positive zero, and original INCR overflow-corrects register addition at037777. Generic Pinball and private P66 mode display now preserve both source behaviors. New independent full-image corpora cover4,096 Pinball and1,024 mode cases, with complete sanitizer streams and128 actualC64 source states passing. Prior published source corpus identities rerun unchanged; earlier constructors did not establish these edges. Source arithmetic and historical files remain unchanged outside the native corrections. Current dependent basic diagnostic also passes its original source images, task admission and real screen output after the fixes. See `docs/DSKY-ZERO-CORRECTIONS.md`.

## Preserve full-width P66 callback alarms

Native callback status now retains full16-bit alarm and fault values instead of the target short enum. Dedicated strict/sanitizer and10 actualC64 enabled/masked-interrupt transport checks preserve01302,01202,8003,ffff and native failure4, commit the qualified source boundary and stop without callback retry. Existing numerical, preemption and pending-request source hashes remain unchanged; this proves status transport rather than additional alarm arithmetic. See `docs/P66-STATUS.md`.

## Port source V06N60 display-data preparation

Original NVSUB through ENTPAS0, table reload, MIXNOUN and DECDSP reaches source DSPDCPUT. All 4,096 complete 2,048-word source images plus SUPER/IRQ match SHA a6f0742ccbc774a0c5eb3e66b62289b6832efcbdfd1c3f6424aeffe53bfd0089. Strict64 source states/10 invalid entries, full-source sanitizers and64 actualC64 states pass. Original noun60 physical data pairs and source continuation state remain authoritative; targetPRG12274bytes, hardware29/software43 stack bytes and zero additional zero page. See `docs/P66-NV-ROWS.md`.

## Port original V06N60 scaling and three-row display consumer

Native scale selection, DSPDCEND BANKCALL/decimal/SWRETURN, row-loop and NVSUBEND/SUPDXCHZ preserve the actual source caller. All4,096 scale-selection states and12,288 complete2050-word three-row checkpoints match original assembly; full-row SHA 2ecaa7528c4f5ae8281952543f146c7390eb9a281d610fbdef49fbc25f819a58. Strict64 states/30 invalid entries, full-source sanitizers andfour actualC64 sixteen-state images pass. PRGs8843/11922/12296/9067bytes use no additional zero page and at most29 hardware/153 software stack bytes. MAKEPLAY/GOPLAY admission and surrounding scheduler control are separate services. See `docs/P66-NV-RENDER.md`.

## Port original GOPLAY display completion and flag services

The original GOPLAY consumer now carries V06N60 through all three display rows and the original return, flash, copy, flag and ENDJOB paths. Source comparisons retain 8192 direct and 18432 full-consumer states, sanitizer streams and the actual C64 PRG, mailbox and stack measurements. See `docs/P66-GOPLAY.md`.

## Port original LM BASIC PRIOCHNG service

The original PRIOCHNG service preserves all source registers, banks and scheduling effects across 8192 complete original states, host and sanitizer checks, and 64 C64 cases. This is a scheduling service milestone; complete LM application acceptance remains pending. See `docs/LM-BASIC-PRIORITY.md`.

## Share the resident Executive lifecycle code

The shared Executive native step is explicitly outlined, reducing the resident to $6e6b while preserving its algorithms. The new PRG and D81 pass 192 original-source C64 checkpoint comparisons with measured software stack 736 bytes; previous artifacts remain retained. See `docs/P66-RESIDENT-OUTLINING.md`.

## Port source P66 entry and completed-pair runtime bridges

Native source-entry bridges preserve the original NEWOPS, EXIT, THDUMP and basic REGODSPR paths without adding polling points. Full original-state comparisons cover 16384 cases with sanitizer and 64 C64 cases. See `docs/P66-RUNTIME-BRIDGES.md`.

## Original immutable fixed data

Both programs now retain 2871 independently assembled immutable words with exact C64 readback. Nine reviewed data-only original files have complete source coverage certificates; mixed noun-reader and Comanche restart files remain partial. Original CM ALTVAR01357 and R53FLBIT05010 layout distinctions are explicit. See `docs/FIXED_DATA.md`.

## Port genuine P66 feedback handoffs and horizontal internal phases

All 492557 original-source numerical and control checkpoints match the native fine phases and sanitizer streams. Thirty-seven actual C64 PRGs pass 1184 frozen entry and exit states. Every completed feedback pair remains an explicit scheduling boundary; pending UNIT and length operations retain ownership until their genuine source return. See `docs/P66-FINE-PHASES.md`.

## Compose real P66 basic jobs, task admission and display output

The native basic binding executes DSPMMJB and ENDJOB, admits RODTASK at its actual 100-centisecond WAITLIST interval, invokes the mapped RODCOMP job and routes DSPOUT to the C64 screen. The RODTASK source comparison covers 2048 complete states, sanitizer results and 64 C64 states; the final resident diagnostic passes eight complete source images with the corrected zero, alarm-width and shared-Executive dependencies. Full descent and interrupt composition remain separate milestones. See `docs/P66-BASIC-RUNTIME.md`.

## Port original Luminary VERB and NOUN keyboard entries

The real VERB and NOUN source entries preserve 4096 complete erasable and interrupt states, including signed-zero blanking, with matching sanitizer streams and 64 actual C64 cases. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port source CHARIN verb and noun admission effects

The original CHARIN path preserves display locks, key-release channel events and real VERB or NOUN execution. All 4096 complete source states and sanitizer streams match; the native C64 diagnostic passes 64 cases. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port source KEYCOM admission of real CHARIN jobs

KEYCOM admits the actual historical CHARIN priority and address pair into all eight Executive contexts, retaining source bailout and RESUME entry boundaries. Original-source and sanitizer comparisons cover 4096 complete states; 64 C64 cases pass. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port original KEYRUPT register saves and time sampling

The real KEYRUPT1 path preserves raw overflow, interrupted Q and banks, double-word time sampling and channel15 key input before actual KEYCOM admission. All 4096 complete original states, sanitizer streams and 64 C64 cases agree; full interrupt entry and resume composition remain separately qualified. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port original Comanche TIG-5 task and phase scheduling

All 1536 complete original-source states agree on the host and under sanitizers; the actual C64 PRG passes 192 representative whole-image fingerprints and a full retained state. Native TIG-5 selects the original TIG-0 delay, phase change, S40.13 priority and TASKOVER route. Resident mission-task composition remains a separate milestone. See `docs/COMANCHE_TIG5.md`.

## Port actual Comanche SETJTAG task and native callback

The original SETJTAG body saves elapsed time, sets JETAG, changes phase and returns through TASKOVER. All 1024 complete source states and real native callback sequences pass host, sanitizer and C64 checks; the initial harness failure and corrected proof remain retained. See `docs/COMANCHE_ENTRY_TASKS.md`.

## Preserve pending NEWJOB during real REGODSPR admission

Actual REGODSPR NOVAC admission preserves or replaces the original pending priority request across 8192 complete source states, matching sanitizers and 64 C64 cases. No synthetic polling point or job-completion placeholder is introduced. See `docs/P66-RUNTIME-REGODSPR.md`.

## Port actual throttle after-poll continuation fetch

The original NEWOPS and LOC03507/03510 fetch routes resume THROTTLE+3 with exact CYR and control debris. All 8192 complete source states and sanitizer snapshots match; 64 native C64 cases pass. See `docs/P66-RUNTIME-FETCH.md`.

## Port canonical LM JOBWAKE source effects

The original JOBWAKE service preserves raw L, bank and scheduling effects in 8192 complete source states, including first wake, no match and both signed zeros. Strict host, sanitizer and 64 C64 comparisons pass. See `docs/LM-BASIC-WAKE.md`.

## Port real MAKEPLAY priority and wake prelude

Native MAKEPLAY follows the actual PRIOCHNG, JOBWAKE, GOSLEEPS, GOPLAY and duplicate ENDOFJOB routes with source XCHTOEND replacement. Original comparisons cover 12288 complete states, matching sanitizer streams and 64 C64 cases; the sleep and resumed lifecycle remains separately qualified. See `docs/P66-MAKEPLAY.md`.

## Port canonical Comanche restart timing services

Complete original-image FINDTIME and ITSLGCL2 source effects; independent source, sanitizer and 192 C64 state checks pass. See `docs/COMANCHE_RESTART_TIMING.md`.

## Port canonical Comanche long-call bank transfers

Original LONGCALL, LONGCYCL, LONGRTRN and GETCADR transitions pass complete-image source and sanitizer comparisons plus C64 checks. See `docs/COMANCHE_LONGCALL.md`.
