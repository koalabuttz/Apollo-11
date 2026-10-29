# Native Luminary keyboard services

Four bounded source services now execute native C rather than AGC instructions.
They preserve every physical erasable word, accumulator/register debris,
SUPERBNK, IRQ state, and actual channel-11 events within their stated boundaries.

| Native module | Original entry and stop | Complete checkpoints | Snapshot SHA256 |
| --- | --- | ---: | --- |
| `dsky_nv_key` | bank40 VERB2354 / NOUN2370 through ENDOFJOB5155 | 4096 | `d5afe6f4954a46f3de2108f7222fa5b8127edc4a34d4b1924603fb42a1a3dd87` |
| `dsky_charin_nv` | bank40 CHARIN2077, keys021/037, through ENDOFJOB5155 | 4096 | `1da3e57b4e13f039c3276c5c15f2de18e174d6f7a9acc03fa8a1607f3f400714` |
| `dsky_keycom` | bank04 KEYCOM3303 through RESUME5270 or BAILOUT5716 | 4096 | `d8270091330ef32a00ccb1eccff54ad8e76b685a0edcd96a81efa9cd824b6e4e` |
| `dsky_keyrupt` | bank04 KEYRUPT1/3274 through RESUME5270 or BAILOUT5716 | 4096 | `5d1f8f6c0f8d208f2029f42c2860687cb398e44e1a2f9c8b8de3d97a33b3a491` |

VERB and NOUN clear their input register, blank the actual two-character relay
word through independently verified generic 2BLANK, select input digit count,
and store decimal/input continuations. Both signed-zero relay words are included.

CHARIN first records the previous display lock in 21/22REG, locks the display,
uses the original CADRSTOR test (clean +0 skips; -0 does not), emits the real
channel011 KEY RELEASE lamp event when appropriate, and dispatches actual
VERB or NOUN before ENDOFJOB. Output events are ordinary hardware metadata and
can feed the native DSKY peripheral latches. No native output placeholder stands
in for the source path. Remaining CHARIN key bodies are not covered by this API.

KEYCOM stores the channel15 keycode, sets DSKYFBIT, and performs actual PRIO30
NOVAC admission of CHARIN at historical 2CADR `02077/060101`. The EB1/SUPER100
pair is the literal source pair; the keyboard interrupt caller may retain any
EB0..7 and SUPER0/20/40/60/100/120/140/160. The keycode is stored into MPAC of the
actual admitted core, including core0. Source core exhaustion stops at BAILOUT
with alarm01202; it does not silently discard the key or claim job completion.

KEYRUPT1 adds source overflow-corrected interrupted-bank/Q stores, complete
LODSAMPT double-word TIME2/TIME1 exchange into SAMPTIME, and actual channel015
low-five-bit read. The corpus includes raw accumulator/Q overflow: original TS
skips XCH Q or the time-sampling call where specified. It composes verified
KEYCOM and retains the source RESUME entry as an explicit stop boundary.
Interrupt lead-in, actual RESUME, native job execution/completion, hardware
keyboard polling, other key bodies, and complete Pinball interaction remain
separate integration milestones.

Independent oracles execute only the unchanged historical original assembly.
They link native-independent input constructors and the host-only pinned AGC
reference engine, and never link native kernels. Where historical fixture
filenames/comments differ, the original file is copied without editing to a
private fixture include name, assembled, and the whole rope must be byte-equal
to the pinned rope before comparison. Native target modules contain no opcode
fetch, decoder, or interpreter. Each corpus compares 2048 physical erasable
words plus SUPERBNK/IRQ; CHARIN additionally compares channel011 and whether an
output event actually occurred. Full sanitizer streams equal the assembly
reference byte for byte.

Strict host and ASan/UBSan tests compare 64 frozen complete source images per
service and reject unsupported input without mutation. Native C64 diagnostics
check these images with real LLVM-MOS builds and restoring platform producer
exclusion supplied by callers. VICE PAL mailbox is `a500004002` for each service;
all report zero failures. Captured logs/snapshots use `build/dsky-nv-key-*`,
`build/dsky-charin-nv-*`, `build/dsky-keycom-*`, and `build/dsky-keyrupt-*`.
Full native/reference/sanitizer snapshots are also retained as `.gz`.

| C64 service diagnostic | PRG bytes | Hardware/software stack sentinel extent |
| --- | ---: | --- |
| VERB/NOUN | 9123 | 10 / 3 |
| CHARIN VERB/NOUN | 11394 | 10 / 22 |
| KEYCOM | 14093 | 13 / 676 |
| KEYRUPT1 | 15167 | 15 / 679 |

All builds use `-fno-static-stack -mlto-zp=0`, allocate no extra compiler zero
page, fit below the reserved software-stack RAM, and do not link the oracle.
KEYCOM/KEYRUPT's temporary Executive view accounts for the larger stack use;
there is one canonical flight-state image and this view is imported/exported
within the excluded native service, rather than kept as authoritative state.

Standalone integration fragments are `tools/dsky_nv_key.mk`,
`tools/dsky_charin_nv.mk`, `tools/dsky_keycom.mk`, and `tools/dsky_keyrupt.mk`.
Each provides `test`, `sanitize`, `c64-*`, and `differential-*` targets; full
native source/reference verification precedes freezing target vectors.
