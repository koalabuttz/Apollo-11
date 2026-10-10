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
| EXECUTIVE:291–303 NUCHANG2; 483–486 NUDIRECT | `exec_start_basic` | Bounded idle start with explicit A=2/L=0 |
| EXECUTIVE:371–457 ENDJOB1/EJSCAN plus CHANJOB-2 | `exec_complete_basic` | Original release, scan, swap and basic DTCB |
| EXECUTIVE:464–465 DUMMYJOB initial stores | `exec_complete_basic` idle return | After NEWJOB=-0, before RELINT and idle activity |
| FRESH_START_AND_RESTART:507–529 resource initialization stores | `exec_init` initial priority/use words | Adapted test initializer; zeroing extra words is separate adaptation |
| ERASABLE_ASSIGNMENTS:376–383 | `executive.core[8][12]`, field constants | Matching logical layout, native physical addresses differ |
| ERASABLE_ASSIGNMENTS:434–443 | `executive.vac[5][44]` | Historical use addresses retained as word values |
| EXECUTIVE:126 FAKEPRET | `EXEC_FAKEPRET` (octal 0110) | Derived from MPAC=0154 minus 36 decimal, fixture checks MPAC |

Reference adapter addresses, all octal: NOVAC=5072; FINDVAC=5105;
ENDFIND=5160; BAILOUT1=5716; ENDJOB1=bank 01,3103;
CHANJOB-2=bank 01,2704; DUMMYJOB=bank 01,3206; CHANG1=5122;
ENDPRCHG=bank 01,2765; NUCHANG2=bank 01,3011; NUDIRECT=bank 01,3225.
The setup helper asserts selected symbol addresses against the assembler listing.

## Deferred routines

SPVAC, CHANG2, interpretive ENDPRCHG/INTRSM, JOBSLEEP/JOBSLP1/2,
JOBWAKE/JOBWAKE2/3/4/WAKETEST, PRIOCHNG/PRIOCH2, NUCHANG2 race/activity handling,
full DUMMYJOB/ADVAN polling/self-check, general SUPDXCHZ, ENDFIND's return and all alarm/restart code
remain untranslated. `exec_endjob_select` is not ENDOFJOB's complete calling
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

Register CCS with overflow, TS skip effects, edit registers, multiword
arithmetic, fixed-point guidance scales, multiplication/division and interpreter
operations are not implemented. Normal bounded admission does not require them.
