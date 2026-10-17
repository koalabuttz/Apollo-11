# Thin resident Executive and Waitlist on the canonical image

Lane A (Resident kernel lane). Milestone `resident-kernel`.

## Why

The resident window is $0801-$6F00 (25.7 KB). The certified Executive and
Waitlist routines (`src/lm_exec_*.c`, `src/cm_exec_*.c` and their bodies)
measure 54-56 KB per mission at -Os, because each one gathers and commits
whole core-set arrays and refuses inputs outside its proven domain.
David picked the steward's "Thin kernel" option
(`steward/resident-memory-proposal.md`): one small kernel that works on the
canonical image directly, tested against the certified routines as oracle.

## What it is

`src/resident_kernel_body.inc` translates EXECUTIVE.agc and WAITLIST.agc
instruction by instruction onto the 2048-word image. Each basic instruction
is one `agc_image_ops` call (`ai_*`, the yaAGC-checked instruction effects),
so A, L, Q, the bank registers, the editing registers and every scratch word
end exactly as the original leaves them. Control flow is the source's: a CCS
outcome, BZF/BZMF, TC (Q = the next word) or an INDEXed jump goes to the C
label of its target, and every outcome the source can take is followed.

There is no gather/commit view, no domain refusal and no site table of its
own. A routine refuses (`EXEC_OUT_OF_SCOPE`) only when:

- an inline operand at Q has no word in the program's transfer-ABI site
  table (NOVAC, FINDVAC, WAITLIST, LONGCALL: two words; TWIDDLE, FIXDELAY:
  one), so a call from an unknown site changes nothing;
- the LM program has no `agc_hardware` record for a job switch or the idle
  loop (they write COMP ACTY in channel 11), or either mission has none for
  TASKOVER and the T3RUPT lead-in (RESUME clears InIsr).

One body serves both missions. `src/lm_kernel.c` and `src/cm_kernel.c` set
the mission, its device record (`p66_bias_io` or `cm_native_io`) and include
the label macros that `tools/resident_kernel_labels.py` generates from the
yaYUL listings (`src/lm_kernel_labels.inc`, `src/cm_kernel_labels.inc`;
`make test-resident-kernel` checks them when the listings are built). The
mission differences are `#if KERNEL_LM` blocks: Comanche's BAILOUT/POODOO
without LXCH EXECTEM1/CA Q, NO.CORES 7 or 6, Comanche's EJSCAN without the
eighth core set, WAITLIST's negative-delay check (WATLST0- or WAIT2), the
WTABORT alarm, and LNGCALL2's LONGPOOH check.

With `RK_FIXED_IMAGE` (the C64 builds) the kernel addresses the image at
$7000 (`APOLLO_SHARED_ADDRESS`) as a constant and refuses any other image.

### Routine tables

`include/resident_kernel.h` declares tables with the same keys as the
certified ones, so a program swaps them in place:

| Kernel table | Replaces | Entries |
|---|---|---|
| `lm_kernel_routines` | `lm_exec_routines` | NOVAC, FINDVAC, TWIDDLE, WAITLIST, LONGCALL; LNGCALL2, LONGCYCL, LONGRTRN, MUCHTIME+4, GETCADR |
| `lm_kernel_job_routines` | `lm_exec_job_routines` | SPVACIN, SPVAC, CHANG1, CHANG2, JOBSLEEP, JOBWAKE, PRIOCHNG, ENDOFJOB |
| `lm_kernel_task_routines` | `lm_exec_task_routines` | 04014 (T3RUPT), DLY2 -1 (DELAYJOB's `TCF DLY2 -1`), FIXDELAY, VARDELAY, TASKOVER |
| `lm_kernel_idle_routines` | `lm_exec_idle_routines` | NUCHANG2, DUMMYJOB, DUMMYJOB+2, ADVAN; SUPDXCHZ, SUPDXCHZ+1, SVCT3, SVCT3+015, SVCT3X+2 |
| `cm_kernel_routines` | `cm_exec_routines` | the LM services and tasks, in one table |
| `cm_kernel_job_routines` | `cm_exec_job_routines` | as LM |
| `cm_kernel_idle_routines` | `cm_exec_idle_routines` | as LM (SVCT3X+2 at 07721) |

A routine stops where the certified routine of the same entry stops: at the
selected job (its DTCB, or NEWOPS after the interpreter's INTRSM), at the
caller's return, at BAILOUT/POODOO with the alarm code at Q, at ADVAN when a
job switch finds nothing ready (DUMMYJOB, ADVAN and NUCHANG2 entered directly
run on to the job), at a Waitlist task, at TASKOVER, NOVAC or FIXDELAY from
SVCT3, at LONGCYCL or WAITLIST in the LONGCALL chain, or where RESUME ends
the interrupt (Z = ZRUPT - 1). A path that runs into data stops with Z
there, unmapped: DEC 7 after the LM core-set CCS (TC 7), COREINC (TC 014),
the CCSHOLE TCs, DPBIT14's two words, and Comanche's OCT 1201/1202/1203
alarm words (TC 1201...). Comanche's DEC 6 after the core-set CCS runs as
EXTEND, so a +0 priority word goes on to NEXTCORE through BZF.

## Source map

| AGC (EXECUTIVE.agc) | Kernel label |
|---|---|
| NOVAC, FINDVAC, SPVACIN, SPVAC | `NOVAC`, `FINDVAC`, `SPVACIN`, `SPVAC` |
| FINDVAC2, VACFOUND (and the 1201 bailout) | loop after `SPVACIN`, `VACFOUND` |
| NOVAC2, NOVAC3, CORFOUND, SPECTEST, SETLOC, ENDFIND, NEXTCORE | same names |
| CHANG1, CHANG2, CHANJOB (+1, +2), CHANJOB4 (the MPAC swap loop), ENDPRCHG | `CHANG1`, `CHANG2`, `CHANJOB_2`, `CHANJOB_1`, `CHANJOB`, `CHANJOB4`, `ENDPRCHG_1` |
| JOBSLEEP, JOBSLP1, JOBSLP2 | `JOBSLEEP`, `JOBSLP2` |
| EJSCAN, EJ1, EJ2 (one loop over the core sets) | `EJSCAN` |
| JOBWAKE, JOBWAKE2-4, WAKETEST | `JOBWAKE`, `JOBWAKE4`, `JOBWAKE3` |
| PRIOCHNG, PRIOCH2 | `PRIOCHNG`, `PRIOCH2`, `PRIOCH2_1` |
| ENDOFJOB, ENDJOB1 | `ENDOFJOB` |
| DUMMYJOB, ADVAN, NUDIRECT, SUPDXCHZ, NUCHANG2 | same names (`DUMMYJOB2` is DUMMYJOB+2) |

| AGC (WAITLIST.agc) | Kernel label |
|---|---|
| TWIDDLE, WAITLIST, DLY2, WAIT2 | `TWIDDLE`, `WAITLIST`, `DLY2` |
| WTLST5 (the eight LST1 tests), WTABORT, WTLST2, WTLST4 | `WTLST5`, `WTABORT`, `WTLST2`, `WTLST4` |
| FIXDELAY, VARDELAY, SVCT3 | same names |
| T3RUPT (and the 04014 lead-in), T3RUPT2, TASKOVER, RESUME | `T3RUPT`, `T3RUPT2`, `TASKOVER` |
| LONGCALL, LNGCALL2, LONGPOOH, LONGCYCL, MUCHTIME, LASTTIME, LONGRTRN, GETCADR | `LONGCALL`, `LNGCALL2`, `LONGPOOH`, `LONGCYCL`, `LONGRTRN`, `GETCADR` |

## Evidence

Hashes: `evidence/resident-kernel/SHA256.json` (all records are in git).

- Independent assembly comparison and host results
  (`resident-kernel-host.log`, `resident-kernel-proof.json`):
  `tests/test_resident_kernel.c` runs every case of the six certified service
  milestones (`tests/resident_kernel_cases.c` drives each milestone's own case
  generator and trampoline program with the kernel tables in place). For each
  case the digest of the outcome, all 2048 words and the device words must
  equal the milestone's frozen yaAGC vector of the original program, and the
  final image must equal the certified routine's word for word.

  | Mission | services | jobs | idle | Total compared |
  |---|---|---|---|---|
  | LM | 9,170 | 9,216 | 4,864 | 23,250 |
  | CM | 7,522 | 6,144 | 5,120 | 18,786 |

  None differ. The 38 LM and 126 CM services cases the certified routines
  refuse have no yaAGC result (the original runs into DPBIT14 or a TC 1203
  alarm word and the vectors record only the refusal), so they are not
  compared; the kernel runs them to the same data address. Strict
  (`-Wall -Wextra -Werror -Wconversion -pedantic`) and ASan/UBSan builds pass.
  A one-word mutation (CS TWO to CS THREE in DUMMYJOB) makes 340 checks fail.
- C64 execution results (`resident-kernel-vice.log`): `make vice-resident-kernel`
  runs eight programs (LM services, LM jobs in lm-exec-jobs' three cohort
  parts, LM idle, CM services, jobs and idle) built with `RK_FIXED_IMAGE`, so
  the kernel runs on the image at $7000. `platform/c64/resident_kernel.ld`
  keeps code below $7000 and data above $8000. The rows
  (`tests/{lm,cm}_kernel_c64_rows.h`, `make resident-kernel-rows`) are the 36
  LM and 37 CM cases that take every kernel branch arc the host corpus takes
  (gcov over the kernel's translation unit, `tools/c64_row_cover.py`), plus
  every 64th case with an oracle: 400 LM and 329 CM checks, all passing.

## Size

`make resident-kernel-size` links only the kernel tables (LLVM-MOS, LTO,
gc-sections, image at $7000) and subtracts an empty program:

| Mission | -Oz | -Os | -Oz without the operand reader |
|---|---|---|---|
| LM | 10,273 B | 11,702 B | 9,604 B |
| CM | 9,985 B | 11,311 B | 9,336 B |

The operand reader (`agc_transfer_operand`, 650-680 B) is shared transfer-ABI
code every resident program links. The certified routines measure 54-56 KB,
so this is about a fifth of that. It is above the ~8 KB target: each AGC
instruction stays a separate helper call so the image matches the original
in every register and scratch word, and that costs about 20 bytes of 6502
code per instruction.

## Limits

- The kernel is compared only where the certified milestones have yaAGC
  results; the refused services cases above are run but not compared.
- docs/RESIDENT-FLIGHT.md links it into the ABI 2 flight resident and
  measures that resident; the allocation plan (`tools/overlay_allocation.py`)
  takes the numbers separately.
