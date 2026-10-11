The native `cm_interbank_complete` module maps all 76 emitted words and both assembler
directives in byte-original Comanche055 `INTER-BANK_COMMUNICATION.agc`. It uses
the canonical 2,048-word erasable image and explicit C entry selectors. The
source file remains unchanged. The original-layout reference rope is
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.

| Original entry | Logical source PC | Native behavior |
| --- | --- | --- |
| BANKCALL | 04662 | Save A/L, reread the inline CADR after stores, advance caller Q, switch FB, retain actual SWRETURN |
| SWCALL | 04666 | Normal and both raw-A overflow paths through TS/LXCH, BUF exchange, and indexed call |
| SWRETURN | 04675 | Restore arbitrary saved FB bits with source masking and execute the valid saved TC descriptor |
| POSTJUMP | 04701 | Preserve incoming A, reread aliased inline data after XCH Q, switch bank, and jump |
| BANKJUMP | 04704 | Ordinary transfer, positive overflow, and all four negative-overflow successor branches |
| Q+10000 | 04707 | Source indexed transfer for an ordinary relative Q |
| PRIO12 | 04710 | Original TCF 10000 entry and immutable data literal 012000 |
| MAKECADR | 04711 | Source LOW10 mask, raw AD carry/overflow, and actual TC Q return |
| SUPDACAL | 04715 | Read caller-supplied passive original fixed word, preserve source MPTEMP/L/bank debris, restore channel 7, and RELINT |
| IBNKCALL | 04740 | Interrupt-context BANKCALL using the original RUPTREG3/4 physical aliases |
| ISWCALL | 04744 | SWCALL using RUPTREG3/4 |
| ISWRETRN | 04753 | Interrupt-context bank return through the actual saved descriptor |
| USPRCADR | 04757 | Source LOC/BANKSET/EDOP stores, inline alias order, bank selection, and interpreted-code handoff |
| SUPERSW | 04773 | Source channel-7 mask/write and actual caller return |

The ordinary compiled caller Q occupies 0100 through 07775. An erasable inline
program word must occupy ordinary physical storage at or above 0100; CPU,
editing, and hardware-counter registers are excluded as inline code storage.
Fixed inline data is supplied by the immutable original-word reader. A and L
retain raw 16-bit register values; other canonical words are ordinary 15-bit
words. EB, FB, and BB must be coherent, ZERO must contain zero, channel 7 must
use its real 0160 mask, and the interrupt latch must be zero or one. Saved
return words must be valid basic TC caller descriptors 0100 through 07777;
CPU-register destinations and special TC 2/3/4/6 words are excluded as saved
compiled caller returns. Native
caller bodies remain explicitly owned by their compiled modules.

Negative-overflow BANKJUMP receives full source-body qualification. TS FBANK
skips MASK and produces -1 in A; XCH Q leaves -1 in Q, and INDEX Q modifies the
one fixed original TCF word into CCS 1777. The native code implements that
specific source branch directly. Positive, positive-zero, negative, and
negative-zero CCS outcomes enter MAKECADR at CAF, MASK, AD, or TC respectively.
The final TC Q reaches physical PC 2 with raw Q 0177776. That register word is
an invalid caller descriptor, so the module exposes this exact external
continuation before any register word is fetched and executed. It does not
turn this fault path into a successful return or decode the arbitrary word.
Normal compiled caller returns are independently checked at the actual
returned PC and bank state, including the instruction held in saved TC data.

The source audit records all 12 original BANKJUMP caller sites and their
preceding producers. Most load an ordinary stored or fixed CADR. The display
paths additionally use MAKECADR plus a saved offset, subtract three from a
saved return, or add three to a saved immediate return; PREBJUMP passes the
fixed SETUPDSP CADR across LXCH BBANK. The proof does not rely on excluding
overflow through those producers: all 65,536 raw A values are qualified for
every native entry, including the unusual body above. Valid original CADR
ownership and caller placement still belong to the compiled caller, and this
module does not certify the bodies of those 12 containing files.

BUF2/BUF2+1, RUPTREG3/4, LOC, BANKSET, EDOP, and MPTEMP use the original
physical image aliases. Erasable inline reads happen after the source stores,
including aliased BUF/LOC cases. No second authoritative RAM image is created.
The diagnostic's second image is an expected-state comparison object only.
Scratch, inline data, and operand producers remain excluded until this source
body returns or hands off its explicit continuation. No extra Executive poll
is inserted. IRQ latches are source state; the reference and C64 diagnostic
disable asynchronous external producers during each comparison.

SUPDACAL uses `cm_interbank_complete_data_address` and the unchanged passive original
CM storage reader. The host source oracle independently fetches from its own
original rope and verifies the complete resulting state, including overflow
restoration and superbank selection. Passive storage is already independently
qualified across all 40,960 physical addresses and both IRQ states on C64;
this packet depends on that immutable data proof. The C64 control diagnostic
supplies the actual source-fetched word at the explicit native data-call
boundary. It does not add a new disk resolver, perform an AGC instruction
fetch, or certify a unified application loader.

Strict and full ASAN/UBSAN runs match 921,088 complete original-source states.
The first 917,504 cases cover all raw A and raw L values across all 14 entries,
varied EB/FB/superbank/IRQ and erasable callers. A further 3,584 cases cover
fixed callers, raw corners, varied saved return descriptors, and every signed
CCS outcome on the negative-overflow path. This is exhaustive raw-register
coverage per entry, with sampled context cross-products rather than every
possible global image. The ordered source stream's decoded SHA256 is
`ea316c318b96d980e7d95dc875e1955ad78c7c5bd93174d01ac49bd21beb26ad`.

On stock-ROM PAL C64, 448 native calls compare every physical word and both
external latches exactly against independently executed source states. Sparse
fixtures encode source-derived changed words while retaining every unchanged
constructor word; they are lossless, and each source/constructor difference
was independently checked before freezing. No state hash replaces comparison.
The original PRG, map, mailbox, VICE logs, host logs, and full lossless source
streams are retained with hashes in the evidence ledger. `cm_interbank_complete_literal`
also qualifies PRIO12 as the original immutable 012000 word; all 65,536 accessor
arguments and unsupported-output preservation are checked on the host.

Reproduce with `make -f tools/cm_interbank_complete_port.mk test sanitize`,
`make -f tools/cm_interbank_complete_port.mk check-host`, and
`make -f tools/cm_interbank_complete_port.mk vice-test` with the configured compiler,
VICE, ROM, and data directory variables. The source oracle links GPL Virtual
AGC only on the host. Native C and the target contain no copied GPL engine,
instruction decoder, or runtime AGC emulator. Source-body qualification does
not assert that either complete flight application is finished or scheduled
by this diagnostic.

This additive module leaves the historical frozen `cm_interbank` and
`cm_interbank_extended` helpers and existing INITDSP callers unchanged. Their
previous bounded source contracts remain distinct. Callers must explicitly bind
the new compiled selector API; this packet does not silently replace old APIs.
All eleven other `cm_native_io` fields are preserved. Every host call checks
that preservation, and each target call compares the complete unowned metadata
record against its input. The diagnostic uses two comparison images and is a
standalone PRG; it makes no claim of production resident/overlay RAM allocation.
