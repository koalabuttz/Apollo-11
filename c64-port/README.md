# Apollo 11 Executive — native Commodore 64 C port

This is a small native translation of actual Luminary 099 Executive paths.
The C64 binary contains no AGC instruction decoder or AGC emulator. Virtual AGC
is used only on the host as a verification oracle.

**Implemented:** NOVAC/FINDVAC admission through ENDFIND; VAC/core allocation;
packed-priority comparison; ENDJOB1 resource release and EJSCAN selection up to
dispatch entry. **Not implemented:** context dispatch, running admitted Apollo
jobs, sleep/wake, WAITLIST, interrupt adaptation, restart/recovery, guidance.

26 original-assembly scenarios agree word-for-word with the host translation.
The native C64 diagnostic passes all 26 scenarios and checks their complete-state
fingerprints against assembly-derived vectors in VICE. This is bounded behavioral
evidence, not a claim of a complete Executive or real-time equivalence.

## Repository

Upstream: https://github.com/chrislgarry/Apollo-11

Pinned base: `911e5c0283c629c50cb97666f34065e8c07d71a5`.
Development branch: `port/c64-native-c`. All changes are under `c64-port/`.
The full upstream history and historical `.agc` files are retained unchanged.
No fork was created: the authenticated connector did not expose repository
creation/forking, and no CLI credential was available. See `docs/HANDOFF.md`.

## Build and run

From `c64-port/`, with GCC/Clang, Make, Python 3, and LLVM-MOS installed:

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
make c64 MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
```

The deliverable is `build/executive.prg` (ordinary C64 PRG, BASIC SYS stub).
Load it on a C64 and RUN, or use VICE. This diagnostic prints a test summary,
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

The test checks full saved state, not just the small manual assertions. Arithmetic
is also compared against Virtual AGC's numerical helper API. Regenerate frozen
C64 fingerprints only after a successful differential test:

```sh
python3 tools/freeze_vectors.py
make test c64
```

### VICE verification

```sh
python3 tools/vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
```

Provide standard ROM files `kernal-901227-03.bin`, `basic-901226-01.bin`, and
`chargen-901225-01.bin`. The helper uses the console monitor, loads the PRG after
BASIC boot, runs native code, and checks a RAM completion mailbox. A cycle limit
or missing completion record fails the test. It also records one admission timing
and stack sentinel observations. No display server is required.

## Engineering record

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
