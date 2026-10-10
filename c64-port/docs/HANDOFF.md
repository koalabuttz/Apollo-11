# Bounded interrupt delivery milestone handoff

## Repository

Fork: https://github.com/koalabuttz/Apollo-11
Development branch: **port/c64-native-c**. Upstream and master remain based on
`911e5c0283c629c50cb97666f34065e8c07d71a5`. Historical files are unchanged; all
port changes live under `c64-port/`. Full upstream history is preserved.
The first archived milestone ended at `a9edd173fdd269262f886c19ae983276bf4e7e6d`.
Use `git rev-parse port/c64-native-c` to identify this milestone's current tip.

```sh
git clone --branch port/c64-native-c https://github.com/koalabuttz/Apollo-11.git
cd Apollo-11
git remote add upstream https://github.com/chrislgarry/Apollo-11.git
cd c64-port
make test
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
make c64 MOS_CC=/path/to/mos-c64-clang
make c64-irq MOS_CC=/path/to/mos-c64-clang
python3 tools/vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/irq_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```

## Completed work

The third milestone adds restoring C64 exclusion guards, a four-entry IRQ
admission-request buffer and foreground delivery safe points. Actual raster IRQs
preserve LLVM-MOS registers and interrupted C frames, while dispatch remains
cooperative. Queue overflow and resource-fault behavior are documented in IRQ.md.

The original bounded admission and release/selection paths remain verified.
The second milestone implements basic CHANG1/CHANJOB context exchange and
completion through basic dispatch. Historical two-word addresses map to native
C continuations, with the original ascending packed-priority scans retained.
All MPAC/MODE, LOC/BANKSET debris, PUSHLOC/PRIORITY, FIXLOC and OVFIND transitions
are compared with unchanged original assembly. Interpreter dispatch is rejected
before release or exchange; it has not been approximated with native callbacks.

Two native demonstration jobs actually execute, yield, resume and complete.
Eight sequences cover higher/equal/lower priorities, VAC/FAKEPRET low-bit ties,
minimum/maximum priority and retained stale state. They check restored payloads
inside the native callbacks. The 53 dispatch cases yield 109 full-state/target
checkpoints. Original target Z/BBANK/SUPERBNK transfers are checked independently.

Validation passes:

- Host numerical tests and the original 26 admission/completion scenarios.
- Host dispatch/fault tests and all eight native two-job executions.
- 26 admission/completion plus 109 dispatch checkpoints, word-for-word against
  original assembly, and the separate Virtual AGC numerical helper comparisons.
- AddressSanitizer and UndefinedBehaviorSanitizer (leak scanning disabled).
- PAL C64 VICE: zero failures, all 135 original-derived fingerprints, eight
  actual native two-job sequences and three repeated timed-path checks.
- Host IRQ publication/FIFO tests over 600 starting-ticket cases, guards, idle
  delivery, overflow, exact resource faults and callback-local fail-stop.
- Separate PAL C64 diagnostic: nine actual raster IRQs, register/decimal canary,
  isolated ISR locals, interrupted native C arithmetic, eight original-derived
  checkpoints, overflow, masked delivery, KERNAL timer advancement and removal.
- 74 guarded operation intervals plus IRQ and hash-free latency observations.

Retained executable, snapshots, logs, target mailbox/stack dumps, map and hashes
are in `evidence/`; reproducible output is in `build/`. DISPATCH.md documents the
callback ABI, startup scratch convention, reference boundaries and fail-stop
behavior. PERFORMANCE.md records the current image/RAM/stack and timing samples.
IRQ.md defines queue ownership, safe points, fail-stop behavior and the C64 ABI.
The Executive state remains 648 bytes; the target request buffer is 32 bytes.

## Limits and next milestone

This is a cooperative basic-job translation with native demonstration routines,
not a complete flight OS or translated guidance workload. Callbacks return to
a trampoline; persistent values belong in MPAC/VAC, not suspended C stacks.
An unmapped address latches a runtime fault. No rollback or Apollo recovery is
claimed. Idle dispatch uses a bounded A=2/L=0 convention, not Apollo self-check.

The IRQ handler publishes through `exec_irq_post`; it must never mutate Executive
state or invoke native dispatch. Foreground wrappers restore IRQ state and admit
at declared safe points. The C64 IRQ build requires `-fno-static-stack -mlto-zp=0`;
the legacy console build is unbound and uses the usual compiler defaults.
Source rate, callbacks and masked intervals still need application bounds;
finite raster testing does not establish an unconditional delivery deadline.

Next: sleep/wake and priority change. Follow with WAITLIST/timers, restart and selected guidance
work. Interpreter dispatch, flight deadlines and physical hardware verification
remain open. See ROADMAP.md.

Acceptance for basic native dispatch and bounded interrupt delivery is met.
No master/upstream merge is needed.
No second-agent/human review was performed. Native C is MIT/public-domain
translation; the separate GPL-2.0-or-later original-assembly adapters are host
verification tools and are never linked into the C64 executable.
