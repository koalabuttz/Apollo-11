# Apollo 11 Executive — native Commodore 64 C port

This is a small native translation of actual Luminary 099 Executive paths.
The C64 binary contains no AGC instruction decoder or AGC emulator. Virtual AGC
is used only on the host as a verification oracle.

**Implemented:** NOVAC/FINDVAC admission and allocation; ENDJOB1/EJSCAN;
basic CHANG1/CHANJOB context exchange and completion through basic dispatch;
explicit historical-2CADR mappings to native C continuations; platform interrupt
exclusion and bounded IRQ admission requests; basic JOBSLEEP/JOBWAKE and
PRIOCHNG. Native demonstration jobs run, sleep, wake, change priority, resume
and complete. **Deferred:** Apollo guidance jobs, interpreter dispatch, WAITLIST
and restart.

26 original admission/completion scenarios and 53 dispatch cases (109 checkpoints,
including eight sequential two-job executions) agree word-for-word with original
assembly. The native C64 diagnostic runs the same jobs and checks all 135
original-derived state fingerprints in VICE. A separate real raster-IRQ diagnostic
checks interrupted C, register preservation, bounded delivery and overflow.
A third diagnostic checks 81 lifecycle cases / 115 checkpoints, five native
sequences and an IRQ-admitted waker. These are bounded behavioral checks,
not a complete flight Executive or real-time equivalence claim.

## Repository

Upstream: https://github.com/chrislgarry/Apollo-11

Pinned base: `911e5c0283c629c50cb97666f34065e8c07d71a5`.
Development branch: `port/c64-native-c`. All changes are under `c64-port/`.
The full upstream history and historical `.agc` files are retained unchanged.
Fork: https://github.com/koalabuttz/Apollo-11
The development branch is published on that fork; master retains the pinned base.
See `docs/HANDOFF.md` and `docs/LIFECYCLE.md` for the current milestone.

## Build and run

From `c64-port/`, with GCC/Clang, Make, Python 3, and LLVM-MOS installed:

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
make c64 MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-irq MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-lifecycle MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
```

The deliverables are `build/executive.prg`, `build/irq.prg` and
`build/lifecycle.prg`
(ordinary C64 PRGs with BASIC SYS stubs).
Load a PRG on a C64 and RUN, or use VICE. The console diagnostic prints a test summary,
not an interactive DSKY. Target compiler: LLVM-MOS SDK **v23.3.0**.
Download the pinned release from the URL in `tools/versions.json`, verify its
SHA256, and extract with `tar --no-same-owner -xJf llvm-mos-linux.tar.xz`.
Toolchains and C64 ROMs are not bundled in this repository.

### Execute the original assembly oracle

```sh
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
```

This builds pinned yaYUL/yaAGC and assembles a full reference fixture using
Virtual AGC's corrected supporting transcription, with the requested upstream
`EXECUTIVE.agc` copied **byte-for-byte**. The selected repository's full MAIN
cannot currently assemble unmodified; details are in `docs/VERIFICATION.md`.
No native port implementation is linked into the Executive reference runner.

The test checks full saved state and selected basic dispatch addresses, including
actual BBANK/SUPERBNK transfers, rather than just the small manual assertions. Arithmetic
is also compared against Virtual AGC's numerical helper API. Regenerate frozen
C64 fingerprints only after a successful differential test:

```sh
python3 tools/freeze_vectors.py
python3 tools/freeze_lifecycle.py
make test c64 c64-irq c64-lifecycle
```

### VICE verification

```sh
python3 tools/vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/irq_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/lifecycle_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
```

Provide standard ROM files `kernal-901227-03.bin`, `basic-901226-01.bin`, and
`chargen-901225-01.bin`. The helper uses the console monitor, loads the PRG after
BASIC boot, runs native code, and checks a RAM completion mailbox. A cycle limit
or missing completion record fails the test. The helpers record timing and stack
sentinel observations; the IRQ and lifecycle helpers write structured timing
reports and reject unpreserved compiler ZP allocations. No display server is required.

## Engineering record

- `docs/DISPATCH.md`: native callback API, context swaps, sequential reference boundaries.
- `docs/IRQ.md`: exclusion, queue ownership, fault boundaries and C64 IRQ ABI.
- `docs/LIFECYCLE.md`: sleep/wake, priority scans, callback actions and oracle boundaries.
- `docs/ARCHITECTURE.md`: original structures, calling conventions, dependencies.
- `docs/SOURCE_MAP.md`: original labels mapped to C and reference stop boundaries.
- `docs/FIDELITY.md`: classifications, adaptations, and unsupported behavior.
- `docs/VERIFICATION.md`: executed commands, results, reference provenance.
- `docs/PERFORMANCE.md`: measured native size, RAM, stack, and timing limits.
- `docs/ROADMAP.md`: next bounded milestone.
- `evidence/`: retained snapshots, logs, and native binary.
- `tools/versions.json`: exact source/tool/ROM identities and hashes.

The historical source remains public domain with its original attribution.
New native port code is MIT licensed. The host-only Virtual AGC adapter is
GPL-2.0-or-later; it is never linked into the C64 PRG. See `LICENSE`.
