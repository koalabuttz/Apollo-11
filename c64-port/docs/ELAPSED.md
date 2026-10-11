# P66 DSU PIPTIME

`p66_elapsed_time` translates the DP subtraction immediately after PDDL
THISTPIP. The loaded timestamp in MPAC is reduced by the stable PIPTIME pair;
the six saved vector words and the complete VAC payload remain intact. Native
code contains integer arithmetic only, with no instruction decoder, heap or
floating point. The operation stops at DANZIG (6061), before `STORE 30D` at
bank 32,3361. LUNAR_LANDING_GUIDANCE_EQUATIONS:1004..1006 places DSU at 32,3357
and its PIPTIME operand at 32,3360.

## Ownership and raw-word contract

Call after `p66_push_timestamp`, with MODE=DP (+0). The caller owns distinct
preparation, scaling and elapsed objects and excludes their producers for the
call. All stored words are ordinary 15-bit one's-complement words. Both zeros,
independent high/low signs and arbitrary raw word pairs are supported; no SI
conversion or clock calibration is implied. A/L remain 16-bit AGC registers.

`p66_elapsed_state` owns the two stable PIPTIME input words and BUF scratch:
three words / six target bytes. MPAC/MODE/A/L remain in `p66_preparation`;
OVFIND remains in `p66_scale_scratch`. The existing owned 43-word VAC and raw
FIXLOC/PUSHLOC belong to `p66_pushdown`; DSU does not modify them. Total test
state is 89 words / 178 bytes. PIPTIME, MPTEMP, MPAC +2..+7, all earlier
preparation state and all 43 VAC words are retained.

## Subtraction and overflow correction

INTERPRETER:812..815 complements each PIPTIME word with DCS and enters ENDVXV
at 7000. DAS adds that pair to MPAC, preserving low-word carry and independent
word signs. Without initial high overflow, A/L end at +0 and BUF/OVFIND retain
their previous contents.

An initial high overflow transfers through SETOVF at 7003 to OVERFLOW at 7017
(INTERPRETER:825..839). INDEX A / CS LIMITS chooses the same-sign maximum into
BUF, and AUG forces a register overflow. The original ADS / TS 7 sequence
adjusts the low word, conditionally skips CAF ZERO and adjusts the high word.
Only a final high TS 7 overflow transfers to SETOVF2 and stores +/-1 in OVFIND.
An initial overflow can resolve during this realignment: BUF changes, OVFIND
stays unchanged and raw A retains its nonoverflow final value. L stays +0.
This exact path must not be replaced with generic saturation or unconditional
word-sign normalization.

For example, timestamp `{1,0}` minus PIPTIME `{0,037777}` produces raw MPAC
`{1,040000}`, representing +1 across the low-word boundary. Its mixed word
signs are retained. The retained four-step histories exercise this transition.

The function returns three observation bits: bit 0 records initial DAS high
overflow, bit 1 the low TS 7 skip, bit 2 the high TS 7 skip. This mask is a
native test observation, not historical memory, an alarm or a scheduling status.
For the tested domain the observed masks are 0, 1, 5 and 7.

## Independent original-assembly verification

`tools/elapsed_reference.c` links input-only constructors and genuine pinned
Virtual AGC objects, without native numerical code. Direct entries execute
DSU at 7005. Integrated entries execute unchanged RODCOMP at 32,3275 through
VXSC and PDDL, capture the boundary before DSU, then execute the actual DSU.
Each final boundary is DANZIG before STORE 30D. Hardware counter progress is
held and watchdog alarms suppressed during these bounded oracle calls.

PIPTIME is physical erasable 01234/+1. Its decoded bank-window operand is
ADDRWD=01634 with EBANK=01000 (E2), FBANK=064000 and final BBANK=064002. The
integrated oracle asserts LOC=03360, BANKSET=064007, DP MODE, operand/bank
selection, RELINT permission and PUSHLOC advance. It observes actual next-PC
TS 7 skips at 7026/7033 and reads all 89 modeled words from physical memory.
Nine adjacent memory guards, VACUSE and the word after the VAC are preserved;
ZERO register 7 stays zero. BUF is 0130; its following words are guarded.

The twelfth byte-identical original-source overlay supplies LIMITS constants:
repository `FIXED_FIXED_CONSTANT_POOL.agc` is copied to the pinned fixture's
hyphenated include name `FIXED-FIXED_CONSTANT_POOL.agc`. Its SHA256 is
`7f139c2b553ce60f8c0ab8cf6242abef390c6a28858e45a4f1d80f0ff2c646b7`.
The adapter asserts assembled POSMAX/NEGMAX at fixed addresses 04733/04735.
All twelve overlays preserve their source bytes; the rope remains
`1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`.

| Cases | Coverage |
|---|---|
| 0..15 | All 16 four-word signed-zero configurations |
| 16..31 | Carry, mixed signs and both signs of overflow paths 1/5/7 |
| 32..159 | All 128 integrated preparation skip inputs in both sign families |
| 160..162 | Three four-step histories retaining saved vectors and VAC state |
| 163..418 | All 256 corners of {+0,-0,+max,-max} across four words |
| 419..2466 | 2048 direct deterministic random inputs |
| 2467..3490 | 1024 full-prefix deterministic random inputs |

3491 cases produce 4664 checkpoints: 2336 direct final states and 1164
integrated calls with both post-PDDL and final DSU boundaries. Every row
compares all 89 words and the observed path. Positive initial overflow paths
1/5/7 occur 1/207/208 times; negative paths occur 1/199/213 times. All agree.
Native and reference snapshot SHA256:
`20786b15499894dfc73baa4c23ef84ac801ff235c404affa293f958d133373b5`.

All ten host and ASan/UBSan suites pass (managed-environment leak scanning
disabled). Frozen expectations contain 172 full final 89-word states and
172 path bytes, including rare resolved overflows and the retained histories.
The tenth standalone C64 PRG checks every expectation, all integrated skip
inputs and restoring I for initially enabled and already masked callers.
Mailbox `a50000ac0003b25924` reports zero failures. All 157 earlier evidence
identities and all historical files remain unchanged; the manifest now holds
176 evidence identities.

LOC/BANKSET/ADDRWD/Q and physical interpreter dispatch/bank control remain
outside the native operation's contract. The original control transfer is
replaced by an explicit C call with a restoring platform guard. Live PIPTIME
publication, interpreter resume and flight scheduling remain future work.

## C64 measurements and continuation

The PRG is 44887 file bytes / 44885 loaded bytes at $0801..$b755, SYS $080d.
Text is 13748 bytes, read-only data 31125, BSS 178 and noinit one byte.
Expected words use 30616 bytes, and expected path masks another 172. Image plus
BSS/noinit is 45064 bytes, ending at $b808; this standalone diagnostic fits
stock C64 RAM. It uses 32 compiler imaginary ZP registers and no extra ZP.

| Sample | PAL cycles | At 985248 Hz |
|---|---:|---:|
| DSU without overflow | 861 | 0.874 ms |
| Resolved positive overflow | 1251 | 1.270 ms |
| Positive final-only skip | 1381 | 1.402 ms |
| Negative two-skip overflow | 1304 | 1.324 ms |
| Complete RODCOMP through DSU, history step 0 | 21427 | 21.748 ms |

Setup and full-state checks are outside these guarded intervals; marker/call
and VIC bus costs are included. The full prefix exceeds one PAL frame while
masked in this diagnostic. These finite samples do not establish WCET, a live
source-rate bound or a flight deadline. Hardware-stack sentinel extent is 38
bytes below entry, 52 including the 14-byte ROM frame. Software-stack extent is
313 bytes, including the complete checker. Evidence is under `elapsed-*`.

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
python3 tools/freeze_elapsed.py
make c64-elapsed MOS_CC=/path/to/mos-c64-clang
python3 tools/elapsed_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```

`STORE 30D` is now implemented and verified in STORE.md, including old-word
register effects and overlapping VAC saves. This DSU API keeps its pre-STORE
boundary; DDV is now verified in DIVIDE.md using the hardware model. PDVL is next.
