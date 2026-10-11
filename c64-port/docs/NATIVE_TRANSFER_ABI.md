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
  before the next starts, so C stack depth is constant. Executive jobs and
  Waitlist tasks keep using `exec_native`; this ABI covers transfers inside a
  job or task.
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

Luminary099 RESTART_TABLES is back to `partial` until the native RESTARTS
consumer selects its entries and transfers to their 2CADRs through this ABI.
