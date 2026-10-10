# Basic job sleep, wake and priority change

The fourth milestone translates JOBSLEEP/JOBSLP1/2, JOBWAKE's dormant-context
search and PRIOCHNG/PRIOCH2. It adds explicit native callback actions for sleep
and reprioritization, with the same guarded foreground admission safe points.
Eighty-one fixtures produce 115 full-state/transfer checkpoints matching actual
original assembly. Five sequences execute mapped native callbacks on host and
PAL C64. An additional target check admits a waking job from a real raster IRQ.

## Sleep retains the job

`exec_sleep_basic(state, wake_cadr, current_bank, next)` accepts the original
one-word fixed CADR and the current complete BBCON. JOBSLEEP stores that CADR in
LOC. JOBSLP1 complements PRIORITY and saves current EBANK/SUPERBNK flags in
BANKSET, excluding FBANK. EJSCAN selects the first largest waiting positive
packed priority. A selected basic job exchanges through CHANJOB; otherwise the
bounded DUMMYJOB NEWJOB=-0 store returns `EXEC_IDLE`.

Sleeping retains its core, VAC, working words and priority low bits. It does not
release or clear resources. With a selected job, the normal exchange encodes
nonzero overflow by complementing the sleeping PUSHLOC. When everything sleeps,
core zero remains dormant with its live FIXLOC/OVFIND/PUSHLOC unchanged. Idle
therefore need not mean that the Executive has free resources. Unsupported
interpreter selection is rejected before sleep mutates the running context.

## Wake finds the first dormant CADR

`exec_wake_basic(state, wake_cadr)` scans all eight contexts in ascending order.
Only negative nonzero PRIORITY is dormant; active contexts and free -0 contexts
are skipped. The first matching LOC wakes. Repeated calls can wake duplicate
CADRs, one at a time. No match returns nonfatal `EXEC_NOT_FOUND`, stores the
requested CADR in NEWLOC and sets LOCCTR to raw -1 (077776). Other state stays
unchanged. Both-zero equality follows the original CS/AD/BZF behavior, even for
the synthetic +0/-0 CADR boundary fixtures.

On a match it recomplements PRIORITY, sets NEWPRIO and LOCCTR, and reconstructs
the two-word address:

```
location = (wake_cadr & 01777) + 02000
bank     = (wake_cadr & 076000) + saved_BANKSET
```

The bank addition uses one's-complement word arithmetic. SETLOC/SPECTEST update
LOC/BANKSET and NEWJOB with the same packed-priority comparison used for
admission. A core-zero wake from idle writes NEWJOB=+0 and retains FIXLOC/OVFIND.
The native adaptation supplies caller L scratch = +0 for the original DXCH
debris. The oracle initializes L accordingly and compares every saved word.
Waking performs no allocation, release, exchange or callback dispatch.

## Priority change and exact scan behavior

`exec_priority_basic(state, upper_priority, resume, next)` preserves the current
priority's low nine VAC/FAKEPRET bits and replaces only its upper priority field.
The public PRIOCHNG entry saves current BBANK without SUPERBNK in BANKSET and
the basic caller's Q return in LOC. PRIOCH2 scans all waiting active contexts.
Dormant/free contexts do not participate. Resources and work words stay owned.

Its initial BUF+1 threshold is the complement of the new full priority, while
EJ1 receives CCS's candidate priority minus one. A candidate must be at least
two raw units higher to replace the running job. Exact ties and a candidate
one unit higher retain the current job. After the first replacement, the scan
selects the largest complete packed word, retaining the first exact tie. The
adjacent-low-bit tests are synthetic routine-level boundaries; normal admitted
VAC/FAKEPRET bases are farther apart. They expose the source's actual CCS bias,
not a new scheduling rule.

If current wins, ENDPRCHG resumes the caller with NEWJOB=+0 and leaves +0 LOC /
BANKSET debris; FIXLOC/OVFIND/PUSHLOC remain unchanged. The original physical
channel-7 SUPERBNK persists even though the stored bank word excluded it.
`next` retains that complete bank in native metadata so full-pair lookup resumes
the correct callback. The oracle independently checks actual BBANK/SUPERBNK.
If another job wins, CHANJOB saves the current resume and complete bank into its
waiting context, performs the normal exchange, and returns the selected pair.

## Native callback API and exclusion

Callbacks return through the normal C ABI before any context exchange. No C
call frame is suspended or reused for a sleeping job. Persistent data belongs
in MPAC/VAC. The immutable full `{location, bank}` mapping table still identifies
native continuations, and raw historical words never hold C pointers.

- Return `EXEC_SLEEP` with the mapped wake pair in `resume`. Its location must
  be in the switched window 02000..03777 and its EBANK/SUPERBNK must match the
  current job: original JOBSLEEP saves those current flags. The trampoline
  derives the fixed CADR from the pair and sleeps after the callback returns.
- Call `exec_native_request_priority(runtime, priority)` inside the current
  callback, then return `EXEC_REPRIORITIZE` with its mapped resume pair. The
  helper stages metadata only. It does not alter PRIORITY or dispatch while
  that C frame is live. Staged metadata is cleared at each callback entry;
  omitting the matching action or returning it without a staged priority faults.
- Call `exec_native_wake(runtime, fixed_cadr)` from foreground code or a callback.
  It drains earlier IRQ admissions and performs wake under the bound platform
  guard. NOT_FOUND is nonfatal. A woken idle context starts on the next start /
  step; a running callback changes only on its returned action.

All new mutations restore previous IRQ permission, including already-masked
callers. The IRQ producer continues to publish admission requests only. It must
not invoke wake, priority or dispatch APIs; a foreground job can handle the
event, as the target integration test demonstrates. Native step re-entry from
inside a callback is rejected. Sleep/resume mappings and bank flags are checked
before action mutation. A wake can commit original saved-state changes before
an unsupported mapping is detected; its fault is fail-stop, not rollback.
Existing overflow and exact retained 1201/1202 boundaries still apply (IRQ.md).

## Independent reference and target checks

`tools/lifecycle_reference.c` links the pinned Virtual AGC engine, shared input
constructors and original assembly. `DISPATCH_INPUT_ONLY` and
`LIFECYCLE_INPUT_ONLY` omit native execution implementations from the fixture
units. It links no native Executive, dispatcher, arithmetic or callback code.
Each sequential case keeps one machine instance and its scratch/stale state.
Only supplied payload writes and routine-entry registers change between calls.
Signed CADRs are sign-extended into A, as required by the real accumulator ABI.

Transfers are recognized by address **and physical bank**. Some demonstration
addresses overlap Executive instructions in bank 01 (HIGH_ENTRY overlaps
JOBSLP2+1); a PC match alone is insufficient. Wake/admission stop at ENDFIND,
dispatch stops at the actual selected target with BBANK/SUPERBNK checked, and
idle stops after NEWJOB=-0 before RELINT/self-check. No idle polling, activity
lamp, ENDFIND hardware-return or interpreter continuation is translated.

| Cases | Coverage |
|---|---|
| 0..21 | Sleep through every waiting slot; overflow, idle, dormant competitors, ties, low-bit/max-priority winners and PUSHLOC zeros |
| 22..50 | Wake positions 0..7, first duplicate, active match ignored, pending candidates, no match, SB/EB flags, negative CADR and both-zero boundaries |
| 51..75 | Lower/raised priority through every slot, ties/adjacent words, stale NEWJOB, no waiting work, dormant competitors, PUSHLOC/OVFIND zeros |
| 76..77 | Two-job sleep, foreground wake, priority reduction, resumed payloads and completion; NOVAC/VAC variants |
| 78..79 | Sole job sleeps to idle, external wake, direct startup, raised priority and completion; NOVAC/VAC variants |
| 80 | Sleep to idle retaining a VAC; admit/start another job while core zero is dormant; wake, reprioritize and complete both |

All 115 rows compare 96 core words, 220 VAC words, seven globals, alarm marker,
status and the next full native pair. Snapshot SHA256:
`6e5c7afa4a31a2d7fde7b981d89f9ccecb1a6896866ef505432927ab3623c00b`.
Only successful full-state equality permits `tools/freeze_lifecycle.py` to
produce C64 fingerprints. Hashes are compact target regressions, not substitutes
for the host's exact comparison.

The target checks all 115 fingerprints and five actual native sequences under
C64 guards. Its additional IRQ bridge uses a fresh Executive: a VAC-owning job
sleeps, a real raster IRQ publishes a waker admission without touching it, and
foreground dispatch admits/starts the waker. The waker wakes the sleeper and
lowers its own priority. Restored MPAC/OVFIND/VAC payloads are checked inside the
resumed callbacks; only final completion releases the sleeper's VAC. The bridge
is an integration/payload check, not a separate full-state assembly comparison
of C64 interrupt hardware. Seventy-six guarded primitive intervals and stack
observations are retained in `evidence/lifecycle-timing.json` (PERFORMANCE.md).

WAITLIST/CIA task timers, interpreter sleep/resume, restart protection, mission
replay and physical hardware are still outside this bounded milestone.
