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
