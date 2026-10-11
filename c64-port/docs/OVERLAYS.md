# Native C64 disk overlays

The resident loader now executes two separately linked native C payloads from a
D64 image, with normal C calls and returns. This resolves the basic storage and
ABI mechanism needed when the combined native LM/CM code exceeds available RAM.
The payloads are test bodies. This milestone does not claim a complete mission
application or translate additional historical guidance routines.

## RAM and persistent source state

| C64 addresses | Ownership | Capacity |
| --- | --- | --- |
| `$0801..$6fff` | Resident scheduler, loader, adapters and native globals | 26,623 bytes |
| `$7000..$7fff` | Canonical 2,048-word AGC erasable image | 4,096 bytes |
| `$8000..$bfff` | One independently linked native overlay | 16,384 bytes |
| `$c000..$c7ff` | Reserved transient platform/application scratch | 2,048 bytes |
| `$c800..$cfff` | Downward LLVM-MOS software stack | 2,048 bytes |
| `$d000..$dfff` | C64 I/O; unavailable to native code/data | 4,096 bytes |
| `$e000..$ffff` | Stock KERNAL ROM mapping | 8,192 bytes |

`apollo_overlay_state.words[address]` directly indexes the absolute source
erasable address, across all eight AGC erasable banks. It retains the full 16-bit
A/L register representations; it does not normalize signed zero or overflow
bits. Modules receive a pointer to this same image. A numerical context or
Executive structure can be a transient gather/commit view at an excluded call
boundary, but cannot be a second persistent copy of source memory. Original
aliases must continue to name the same word. Native scheduling metadata lives
in resident storage; it is not silently substituted for an AGC source word.

The proof resident links and actually uses the existing Executive admission and
native dispatcher. Its diagnostic Executive object is test state, separate from
the demonstrator's raw image; it is not a finished mission-wide alias adapter.
The mission driver must gather/commit its Executive/VAC views at the same owned
boundaries as numerical views. This distinction prevents the storage proof from
claiming an application integration it has not established.

## Native module ABI and file layout

An overlay implements
`uint16_t apollo_overlay_main(apollo_overlay_state *, uint16_t selector)`.
Selectors identify explicit translated native continuation bodies; they are
not AGC instructions. A three-byte jump at `$8010` transfers to the independently
linked C body, using the same LLVM-MOS calling convention and 32 imaginary
registers as the resident. The body returns to the resident caller normally.
Loading is rejected while any overlay callback is live. To call another module,
the current body commits source state, returns, and lets the resident select and
load the next continuation. No suspended C frame or pointer into replaced
overlay code/data may survive a swap.

The PRG starts with the ordinary two-byte little-endian load address `$8000`.
The payload then has this sixteen-byte header:

| Payload offset | Bytes | Meaning |
| --- | --- | --- |
| 0 | 4 | ASCII `NATV` |
| 4 | 1 | Native ABI version, currently 1 |
| 5 | 1 | Immutable module identity, 1..255 |
| 6 | 1 | Header size, 16 |
| 7 | 1 | Reserved, must be zero |
| 8 | 2 | Entry address, `$8010` |
| 10 | 2 | BSS address |
| 12 | 2 | BSS byte count |
| 14 | 2 | Loaded payload byte count, excluding the PRG load address |

All two-byte fields are explicitly decoded little-endian. BSS must follow the
loaded image and end at or before `$c000`. Initialized `.data` is included in
the file and integrity check. Successful loads reset BSS. Cached calls retain
transient overlay storage; swapping out and back reloads `.data` and zeros BSS.
Persistent mission state always belongs in the canonical source image.

The immutable resident manifest contains module identity, exact filename,
payload length and CRC16-CCITT (initial value `$ffff`, polynomial `$1021`). Host
metadata also records the PRG SHA256 and native source paths. The loader checks
header identity, ABI, entry, bounds and manifest length before copying code,
then checks the complete payload CRC and exact EOF before invocation. CRC is
an accidental-corruption check, not hostile-media authentication.

## Bounded disk loader and platform ownership

The backend opens a PRG as a sequential KERNAL stream on device 8, logical file
2, secondary address 2. It reads the load address/header separately and writes
at most the manifest's bounded payload length into the slot. It reads one final
byte to distinguish exact EOF from an oversized file. Short streams, extra
bytes, KERNAL errors, malformed headers and failed checksums never execute the
partial image and leave all persistent source words and the caller's result
unchanged. A failed/partial swap invalidates the loaded-module cache.
`KERNAL LOAD` is not used because its destination write is unbounded.

The backend distinguishes a valid final byte with READST bit 6 from the next
EOF. Other READST bits fail the read. Logical file 2 and the input channel are
owned by the loader; callers must keep that file closed. Cleanup restores the
standard keyboard/screen channels. The target temporarily unmaps BASIC and
exposes KERNAL/I/O for disk operations and overlay execution, then restores the
incoming `$01` value and processor I/D bits. Caller decimal mode must be clear.
Every native IRQ producer that might execute C or touch the source image/slot
must be quiesced. Stock ROM IRQs may run. No existing IRQ gate was modified.
This is a foreground disk operation, with no real-time guidance deadline claim.

## Source-driven allocation inventory

`tools/overlay_source_manifest.py` inventories all 90 Luminary099 and 85
Comanche055 `.agc` files with SHA256, BANK declaration lines and conservative
native allocation groups. Each mission has 34 explicit BANK groups and the
architectural fixed-fixed banks 02/03, giving 36 groups. The fixed-fixed groups
require the complete mission's assembler listing to resolve source membership;
SETLOC symbols and inherited placement are not guessed. Native continuation
mappings remain explicitly unassigned. This is storage planning, not coverage
certification; `tools/port_coverage.py` remains the completion inventory.

An AGC bank is not automatically one native overlay: translated C can expand
or shrink, a source routine may cross historical bank boundaries, and resident
services may be shared. Future reviewed mappings select complete native call
bodies, preserve their source addresses and boundaries, and partition compiled
modules so each loaded image plus BSS fits the 16KB slot. Modules link their own
arithmetic helpers, never a rope image or instruction interpreter.

A standard 35-track D64 has 174,848 image bytes and 664 free data blocks after
formatting; each data block holds 254 file bytes. A 16KB payload plus PRG header
uses 65 blocks. Therefore 36 maximal overlays need more than one D64. An actual
complete application must partition disk sets or select a larger supported
drive image and a verified disk/device-selection adapter. The demonstrator
contains one resident and two proof modules; no full-mission disk packing or
media-change runtime is claimed.

## Verification and reproduction

```sh
cc -Iinclude -std=c99 -O2 -g -Wall -Wextra -Werror -Wconversion -pedantic \
  src/overlay_runtime.c tests/test_overlay.c -o build/test-overlay
./build/test-overlay
cc -Iinclude -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic \
  -fsanitize=address,undefined src/overlay_runtime.c tests/test_overlay.c \
  -o build/test-overlay-sanitize
ASAN_OPTIONS=detect_leaks=0 ./build/test-overlay-sanitize
python3 tools/overlay_source_manifest.py
python3 tools/overlay_build.py --cc /path/to/mos-c64-clang --c1541 /path/to/c1541
python3 tools/overlay_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/vice
```

Host and ASan/UBSan tests pass: the standard CRC known answer, real cache swaps,
BSS initialization, nested-call rejection, malformed load/header/manifest
cases, every truncation/error position of a small valid file, excess bytes,
checksum corruption, all 2,048 unchanged source words on failure and byte guards
around the entire 16KB destination slot.

The LLVM-MOS 23.3.0 SDK proof builds the bodies at `$a000`, underneath BASIC ROM,
while retaining the jump entry at `$8010`. It therefore exercises the mapping
required by larger overlays. On stock PAL C64 ROMs in VICE 3.7.1, the resident
Executive callback loads A, reuses cached A, loads B, reloads A, then calls cached
A and loads B with BASIC mapped and I masked. All six calls return correctly;
BSS resets after swaps. The real native dispatcher then finishes the job and
returns idle. All 2,048 raw source words are read back and compared individually.
Mailbox `a50000064e010000000000` confirms zero failures and final source values.
The measured software-stack sentinel extent is 71 bytes within its 2KB region.

The resident PRG is 21,169 file bytes, with 21,013 text bytes, 136 read-only
bytes, six initialized bytes, 699 BSS bytes and one no-init byte. Image/state end
at `$5d6b`, below the `$7000` shared image. Overlay A/B are 8,326/8,336 file bytes,
including the intentionally padded gap before `$a000`; both have two BSS bytes.
They allocate no private zero-page storage. The default production linker
origin is `$8013`, so the proof padding is not required by the ABI.

VICE uses D64-backed virtual IEC device emulation because this environment has
no 1541 drive ROM. Actual disk contents pass through target KERNAL open/read
calls; payloads are not injected by the monitor. This verifies C64 disk loading,
mapping and native execution, while leaving physical-1541 timing and real-media
error behavior for hardware qualification. The emulator has a finite cycle
limit and a separate host timeout. Raw files/maps/D64, mailbox, shared-image and
stack captures are in `build/overlay-*` for the parent milestone's evidence set.
