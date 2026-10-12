# Native control-transfer ABI

Lane A owns this interface (strategy doc: "Lane A owns the interpreter and
executive ABI"). Header: `include/agc_transfer.h`; code: `src/agc_transfer.c`.

## The problem it closes

The original program moves control through addresses held as data: the CADR
after `TC BANKCALL`, a return address in Q or BUF2, a 2CADR in a restart table,
an interpretive LOC. Several certified files stopped at that point: the native
code set Z/FB to the destination and returned, and nothing in the port ever
called the destination (auditor 2026-10-11, finding 4). Others recovered the
word after a caller's `TC` by reading a copy of the original rope at runtime
(finding 1).

## The interface

- **Routines.** A translated routine is a C function
  `enum exec_status f(const agc_native_program *, apollo_overlay_state *, void *io)`
  registered at the physical address (`bank*1024+offset`, `agc_fixed_physical`)
  of its original entry. It runs from its entry state and leaves Z (and FB, Q,
  LOC...) where the original continues.
- **Programs.** An `agc_native_program` lists sorted routine tables for basic
  code (keyed by Z) and interpretive code (keyed by LOC under BANKSET whenever Z
  is the program's interpreter fetch point), the compiled operand table, an
  optional passive constant reader, and user data.
- **Trampoline.** `agc_transfer_run` calls the routine at the current point,
  then the routine at its continuation, until the point equals the caller's
  stop address, a routine refuses, or a continuation has no routine
  (`EXEC_UNMAPPED`, image left at that continuation). Each routine returns
  before the next starts, so C stack depth is constant. Job switches are
  transfers too: ENDOFJOB, JOBSLEEP, PRIOCHNG and CHANG1/CHANG2 end at the
  selected job's entry (or NEWOPS, or the ADVAN idle loop), and TASKOVER and
  the T3 vector end at the next task or the interrupted program.
- **Hardware.** `program->hardware` (`agc_hardware`) carries the CPU state a
  routine needs beyond the image and the mission's device record: output
  channel 11 (the Executive's idle loop turns COMP ACTY off), the
  interrupt-in-progress flag (tasks run inside T3RUPT; TASKOVER's RESUME ends
  it), and the LM RCS jet channels 5 and 6 and channel 13 (TIME6 enable) that
  the T6RUPT programs drive (`docs/LM-T6RUPT.md`), and input channel 15, the
  DSKY key code KEYRUPT1 reads (`docs/KEYRUPT-UPRUPT.md`), and output
  channel 10 (the last DSKY relay word) and input channel 32 (the PRO key) that
  T4RUPT uses (`docs/T4RUPT-CORE.md`). It also holds output channels 12 and
  14, input channels 30, 31 and 33, and the scaler channels 3 and 4, for the
  IMU monitor, the DAP and compiled READ/RXOR orders. Input channels hold
  what READ returns, output channels the last word written, and channel 13
  only the CPU-writable bits (043777). This is the one channel record for
  every lane; new channels are appended by Lane A, not kept in mission I/O
  records. Zero-initialize it with `agc_hardware hw = {0};`, because a
  partial positional list fails `-Wmissing-field-initializers` whenever a
  field is added. A routine that needs it refuses when it is NULL.
- **Erasable continuations.** A few files store a `TC` into erasable memory and
  run it later (RESTARTS writes the service call at GOLOC-1 and its own
  continuation at GOLOC+2). Such a point is keyed
  `AGC_TRANSFER_ERASABLE_BASE + physical erasable address`
  (`agc_erasable_point`). The file that stores those words registers a routine
  there that accepts exactly the words it stores and refuses any other with
  `EXEC_OUT_OF_SCOPE`. Nothing decodes an arbitrary erasable word.
- **Pending right-hand orders.** An RTB in the left half of an order pair
  returns through DANZIG with the right order still in EDOP. With the
  mission's marker table (`agc_lm_danzig_marker`, `agc_cm_danzig_marker`) in
  a program's interpretive tables, Z at DANZIG with EDOP positive is an
  interpretive point keyed by LOC under BANKSET, the RTB's operand word. The
  file owning the pair registers a routine there that accepts exactly its
  pair's EDOP code. See `docs/DANZIG-PENDING.md`. The RTB itself enters
  INTER-BANK at `SWCALL -1` (BANKCALL's `INCR Q`), which both missions'
  interbank tables register (`docs/INTERBANK-Q.md`).
- **Inline operands.** The word a caller places after its `TC` (alarm codes,
  BANKCALL/POSTJUMP CADRs) is the caller's compiled constant. A
  `agc_site_operand` table holds it for every genuine call site, keyed by the
  operand's physical address. `tools/agc_sites_generate.py` builds the table
  from the unmodified source listing and checks each word against the source
  text and the assembled rope; `--check` keeps the committed `.inc` current.
  A fixed Q that is no genuine site is `EXEC_OUT_OF_SCOPE`: no original
  producer supplies it. Erasable inline words are still read from the image
  after the original stores. Flight code never links `cm_fixed_compat` or
  `lm_fixed_compat` to obtain operands.

Passive constant reads that the original performs as data (SUPDACAL, fixed
memory display, self-check sums) go through the program's `fixed_data` reader.
They never select code.

## What "complete" means for a transfer-of-control file

A file whose code computes a destination (INTER-BANK, the standard lead-ins,
restart tables and their consumers, Executive/Waitlist entry points, interpreter
dispatch) is complete only when:

1. every entry it defines is a registered routine at its original address;
2. every transfer it performs ends in the trampoline or the Executive registry,
   never in a bare address handed back to an unspecified caller;
3. every inline operand it consumes comes from a compiled site table or the
   erasable image, not from the rope;
4. its evidence includes a round trip through the trampoline into a native
   callee and back, compared with the composition of the file's certified
   steps, on the host and on the C64.

A destination in a file that is not yet translated resolves to
`EXEC_UNMAPPED`. That is the destination file's open work, not a defect of the
transferring file.

## Applied so far

| File | Routines | Operands | Evidence |
|---|---|---|---|
| Comanche055 ALARM_AND_ABORT | entries through `cm_alarm_site_call`, `cm_alarm2_site_call`, `cm_abort_site_call` | `cm_alarm_sites` (62 sites: ALARM, ALARM2, BAILOUT, POODOO) | `cm-alarm-sites` |
| Luminary099 INTER-BANK_COMMUNICATION | 13 entries in `lm_interbank_routines` | `lm_interbank_sites` (448 sites: BANKCALL, IBNKCALL, POSTJUMP, USPRCADR) | `lm-interbank-transfer` |
| Luminary099 SYSTEM_TEST_STANDARD_LEAD_INS | 7 basic entries, E/CALL's EXIT, and the INTPRET/INTPRET+2 forms they reach | test caller only: the flight program has no E/BKCALL or E/CALL caller | `lm-standard-lead-transfer` |
| Comanche055 INTER-BANK_COMMUNICATION | 13 entries in `cm_interbank_routines` | `cm_interbank_sites` (354 sites: BANKCALL, IBNKCALL, POSTJUMP, USPRCADR) | `cm-interbank-transfer` |
| Comanche055 SYSTEM_TEST_STANDARD_LEAD_INS | 7 basic entries, E/CALL's EXIT, and the INTPRET/INTPRET+2 forms they reach | test caller only, as in Luminary | `cm-standard-lead-transfer` |
| Luminary099 EXECUTIVE (NOVAC, FINDVAC) and WAITLIST (WAITLIST, TWIDDLE, the LONGCALL chain) | 10 entries in `lm_exec_routines` | `lm_exec_sites` (146 sites, 268 words) | `lm-exec-services` |
| Luminary099 EXECUTIVE (SPVACIN, SPVAC, CHANG1, CHANG2, JOBSLEEP, JOBWAKE, PRIOCHNG, ENDOFJOB), WAITLIST (FIXDELAY, VARDELAY, TASKOVER) and the INTERRUPT_LEAD_INS T3 vector | 8 entries in `lm_exec_job_routines`, 4 in `lm_exec_task_routines` | `lm_exec_sites` (adds the 16 FIXDELAY sites: 284 words) | `lm-exec-jobs` |
| Luminary099 and Comanche055 IMU_COMPENSATION_PACKAGE | 11 Luminary and 9 Comanche entries in `lm_`/`cm_imu_compensation_routines` (1/PIPA, the NOVAC and BANKCALL returns, NBDONLY, GCOMPZER...) | BANKCALL/IBNKCALL CADRs in `lm_interbank_sites`/`cm_interbank_sites`; Luminary 1/GYRO 2CADR in `lm_exec_sites` | `imu-compensation` |
| Luminary099 RESTARTS_ROUTINE and RESTART_TABLES | RESTARTS, GETPART2, PHSPART2, the display return and the GOLOC-1/GOLOC+2 erasable words in `lm_restart_routines` | table words compiled from the source (`src/lm_restart_table_data.inc`); INITDSP 2CADR in `lm_restart_operands` | `lm-restart-select` |
| Luminary099 and Comanche055 DISPLAY_INTERFACE_ROUTINES | 165 LM / 164 CM entries (every label and every callee return) in `lanec_di_lm_routines` / `lanec_di_cm_routines`, one table per module for subsets | through `lm_interbank_sites`, `cm_interbank_sites` and `lm_exec_sites`; CM NOVAC/FINDVAC operands wait for `cm_exec_sites` | `lanec-di-transfer` |
| Luminary099 and Comanche055 EXTENDED_VERBS | 313 LM / 241 CM basic entries in `lanec_ev_lm_routines` / `lanec_ev_cm_routines`, 15 LM interpretive entries (keyed by LOC) in `lanec_ev_lm_interpretive`, INTPRET for the file's own callers in `lanec_ev_lm_intpret`; io is `lm_dntm_io` / `cm_dntm_io` (channels 12, 14, 30-33) | `lm_interbank_sites`, `cm_interbank_sites`, `lm_exec_sites`; CM interpretive words not yet translated | `lanec-ev-transfer` |

Luminary099 RESTART_TABLES and RESTARTS_ROUTINE carry a pending complete
certificate for the auditor's review.
