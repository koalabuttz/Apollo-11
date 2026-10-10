# Bounded alarm recording and protected job recovery

The sixth milestone translates three explicit stages from unchanged Luminary099
source: ALARM1/BAILOUT1/POODOO1 alarm recording, STARTSB1 software cleanup, and
GOPROG3 phase-pair validation followed by RESTARTS Type C variable job admission.
`src/restart.c` contains native C control flow, with no AGC instruction decoder.
`rs_recover` composes a bounded BAILOUT1 recovery for pending 1201, 1202 or 1203.
It does not implement the complete Apollo abort/restart program.

## State and source boundaries

| Native entry | Original execution | Stop boundary |
|---|---|---|
| `rs_record(..., RS_ALARM1, ...)` | ALARM1, ALMNCADR, LARMENT, CHKFAIL1/2, FAIL3, PROGLARM/MULTFAIL, MULTEXIT | Caller Q+1 after RELINT |
| `rs_record(..., RS_BAILOUT1, ...)` | BAILOUT1, BOTHABRT and shared alarm stores | WHIMPER entry, before RESUME |
| `rs_record(..., RS_POODOO1, ...)` | POODOO1, BOTHABRT and shared alarm stores | OCT77770+1, before phase-4 replacement |
| `rs_cleanup` | STARTSB1 through STARTSB2 and DSPOFF | TC Q return |
| `rs_validate` | GOPROG3 / PCLOOP | MMDSPLAY entry on success; PTBAD entry on disagreement |
| `rs_job` | RESTARTS / ITSAVAR / ITSAJOB / CHKNOVAC / ITSNOVAC, then FINDVAC or NOVAC | ENDFIND or BAILOUT1 entry |

The caller supplies an explicit historical return 2CADR, alarm code, modeled
peripheral words/channels and six protected phase pairs. `restart_state` is 130
bytes on the C64: FAILREG, ALMCADR, twelve DSPTAB words, 41 named cleanup words,
four channel readbacks and three hardware trap latches. `restart_phases` is 60
bytes. These are native structures, not an emulated AGC address space. Test-only
`restart_addresses.h` maps fields to original erasable addresses for the oracle.
No native entry interprets an instruction word or executes a stored address.

Alarm recording preserves the first two alarms and stores the third. A later
alarm replaces the third with BIT15 set. A BIT15-only third register has zero
magnitude and is available, matching FAIL3's POSMAX mask. The first alarm sets
DSPTAB+11's 040400 bits. Existing program-alarm lamp bits are otherwise retained.
ALMCADR receives the explicit caller pair. The bounded API accepts nonnegative
15-bit alarm codes and first/second FAILREG values; negative first/second records
are rejected before mutation. Codes including +0 and POSMAX are verified.
This stage models display memory; it does not operate a physical DSKY.

Cleanup sets TIME3/4/5, disables the T6 continuation, selects the original
DAPIDLER T5 pair, and applies the original RCS, DAP, radar, telemetry and gyro
masks to explicit device/flag inputs. Channel 13 trap-enable bits latch in
hardware and are absent from channel readback; the model retains Trap31A,
Trap31B and Trap32 separately. WOR BIT12 enables Trap31A. Device effects match
Virtual AGC's real CPU channel-write semantics and original instructions; these
fields are modeled Apollo devices, not C64 hardware control registers.

Cleanup frees all eight priorities and five VAC use words, fills WAITLIST with
ENDTASK pairs/dummy intervals, and resets NEWJOB and DSRUPTSW to -0. It retains
core/VAC payloads except MPAC+0's DSPOFF scratch result, retains RUPTAGN and the
protected phase/name/priority records, and preserves FAILREG/ALMCADR/DSPTAB+11.
Display/service continuations are cleared as in STARTSB1. Native alarm markers
are cleared separately; they have no original erasable address. Cleanup is not
`exec_init`: it must not erase the retained payloads or alarm history.

## Protected re-entry and native runtime contract

Phase validation scans groups 6 down to 1 and requires phase XOR complement=-0.
It retains the original MPAC+5 scan index and sets MPAC+6=+0 only after success.
Disagreement returns 1107 before PTBAD's alarm/fresh-start body; that body remains
untranslated. Do not dispatch after this failure.

A supported active demand is a Type C variable JOB phase with
`(phase & 01407)==00402` and phase <=01777. The native group argument is zero
based. PHSNAME supplies the job pair; a positive PHSPRDT selects FINDVAC and a
negative PHSPRDT selects NOVAC with its complemented priority. Zero priorities,
unsupported phase forms and non-basic target addresses are rejected before
mutation. The implementation retains original MPAC scratch and VAC5 GOLOC
workspace writes, then uses the verified Executive admission path. A free
core-zero with NEWJOB other than -0 would invoke CCSHOLE; this entry is rejected.

`rs_recover` preflights all demands, records the pending BAILOUT1 alarm, cleans up,
validates all pairs, and admits supported active groups in descending order.
Inactive groups have phase +0. Unsupported inputs and simultaneous pending
Executive/WAITLIST alarms are rejected before mutation.
Phase disagreement after cleanup retains cleanup and alarm records. Exhaustion
during re-entry returns RS_RESOURCE_ALARM and retains the exact Executive alarm
and partially admitted jobs, including any already reserved VAC. No rollback is
claimed. Six FINDVAC demands can exhaust the five VACs and this is tested.
A 1204 alarm takes POODOO1 rather than BAILOUT1 and is rejected by this composed
recovery. POODOO1 alarm recording is verified separately, before its phase and
servicer changes.

Every raw stage requires caller interrupt exclusion. Before composed recovery,
stop custom IRQ/clock producers, finish/abandon all callback frames, and handle
pending publication queues under their existing ownership contracts. Reinitialize
the clock/task/job runtimes and request buffers deliberately after successful
reconstruction, bind their mappings/guards, then restart the intended sources.
The recovery API does not silently discard queued IRQ requests or source faults.
The C64 recovery diagnostic installs no custom clock or raster producer.

Cleaning scheduler state does not clear a stale `exec_native.fault`. Explicit
`exec_native_init` is required. Host and C64 demonstrations verify that a 1202
runtime cannot execute before or after raw cleanup, then reinitialize it and
execute all six reconstructed native jobs to completion, freeing the owned VAC
and preserving alarm 1202. Restoring guards preserve both initially enabled and
initially masked I states. This is a native integration check, not a claim that
Apollo can resume a suspended C stack.

## Oracle and exclusions

190 input cases produce 226 checkpoints:

- 96 alarm cases: three entry paths, four register occupancy patterns, lamp
  preservation, third-register overflow and eight alarm codes.
- 16 cleanup cases with retained payloads and varied device/flag inputs.
- 26 phase-agreement cases covering both signed zeros and corruption in each group.
- 48 variable-job cases: all six groups, FINDVAC/NOVAC, varied phases, free-slot
  positions, and retained 1201/1202 failures.
- Four ten-step sequences: a real FINDVAC reservation followed by 1202, alarm
  recording, cleanup, valid protected phases and six descending job admissions.

Each checkpoint compares status and all 448 modeled state words: 446 original
AGC/device words plus two explicit native alarm-boundary markers. Protected
records are read back from original erasable memory. Native and reference
snapshot SHA256 is
`81c884b026fbdd4ae8050cddc75f2d07a91a168ccf1d3cf324a8f60cc39631a8`.

The host-only GPL adapter links Virtual AGC's engine and input constructors,
never native arithmetic, Executive, WAITLIST or restart code. It executes real
original instructions and freezes scaler progress so counter changes are explicit.
Historical bank/erasable mappings remain in the oracle. All eight selected
scheduler/alarm/restart source files are byte-identical to this repository.
The assembled rope hash remains unchanged. Frozen C64 fingerprints are generated
only after exact independent snapshot equality.

The stages are exercised sequentially, with explicit register/bank entry inputs
at their boundaries. WHIMPER/RESUME, ENEMA's complete glue, LIGHTSET keys,
MMDSPLAY and display-job admission, GOPROG flag changes, SWCALL/SWRETURN routing,
ENDRSTRT/idle behavior, hardware GOJAM and fresh start remain untranslated.
The composed native driver bypasses that glue; its integration behavior is an
explicit adaptation, not a continuous original-assembly ENEMA execution.
RESTART_TABLES and PHASE_TABLE_MAINTENANCE were traced and included unchanged in
the fixture, but table/even/odd/Type B/display/timed/longcall routes and phase
updates remain future work. No guidance or mission replay is claimed.

## Reproduction

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
python3 tools/freeze_restart.py
make c64-restart MOS_CC=/path/to/mos-c64-clang
python3 tools/restart_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```

Leak scanning is disabled because this executor disallows LeakSanitizer's process
inspection; AddressSanitizer and UndefinedBehaviorSanitizer remain enabled.
The fifth PRG verifies all 226 fingerprints and recovered native jobs. Mailbox
$c080..$c08c is `a50000bee206823c2486038202`. Size/timing/stack samples are in
PERFORMANCE.md and retained with the PRG, map, snapshots and logs in `evidence/`.
These are bounded correctness and observed performance results, not flight
real-time guarantees or physical C64 verification.
