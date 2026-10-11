# Comanche interbank register and fixed-read qualification

The extended canonical API translates the same fourteen original interbank
control entries with raw 16-bit A/L inputs, source TS overflow branches and
passive original-layout fixed reads. Inline CADR words are explicit compiled
caller metadata. Fixed words are fetched only as data after a control frame
has returned. No instruction decoder or program dispatch from fixed words is
introduced.

The complete host corpus contains 917,504 inputs: all 65,536 raw A encodings
for each entry, with bijective raw-L variation and bank/latch variation. Of
these, 901,120 execute original source instructions to the genuine control
boundary and match all 2,048 physical words plus SUPERBNK and IRQ. Another
16,384 verify an unchanged-image rejection of invalid negative-overflow
BANKJUMP inputs. The full lossless stream SHA256 is
`90cf6b53444bfceb559c27492594066289fa5fa8930e6e661f22454e868d2661`.
The entire stream passes strict compilation and ASAN/UBSAN.

Negative overflow in BANKJUMP is a material typed-caller exclusion. TS FBANK
skips MASK, places negative one in Q, and INDEX Q changes the original TCF
10000 word into CCS 1777. That no longer represents an ordinary CADR transfer;
it can fall into neighboring instructions and eventually use an invalid
negative return address. The native preflight rejects this input before any
image or I/O mutation. All other raw-A domains, including positive overflow
BANKJUMP and both overflow signs in SWCALL/ISWCALL/USPRCADR/SUPDACAL, are
source-compared. Physical L is normalized as the original hardware register
requires, including entries which otherwise preserve its incoming value.

The host oracle uses original-layout rope SHA256
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The source reference now stops RETURN/TC Q only after the actual indirect
transfer, rather than its intermediate physical PC=2. This boundary correction
is recorded in the source adapter and does not alter historical instructions.
The source covers banks `0..39`, including unpopulated zero words, through the
committed passive storage dependency. Actual DSPFMEM/COMADRS callers separately
close every physical read address in `COMANCHE_FIXED_CALLERS.md`.

Stock PAL C64 execution loads actual compiled control module `207` and the
same six byte-pinned passive data modules `200..205`. Fifty-six representative
contexts exercise four raw-A corners for all fourteen groups, alternating
incoming C64 interrupt states. All complete-state fingerprints match, two
complete 2,048-word+SUPER/IRQ samples are compared directly, and the final
whole image equals the original source. The explicit invalid BANKJUMP guard
is included. Mailbox `a500003800`, software stack 65 bytes. Control occupies
3,990 loaded bytes; source-input and checker proof modules are diagnostic only.

This remains a bounded primitive qualification, not a complete-file caller
certificate. Q is an ordinary compiled address at most `07775`, saved return
PC is at most `07777`, bank/register consistency is required, and the caller
owns producer exclusion. The proof varies individual bank fields but does
not enumerate every A/L/Q/bank cross-product. Actual application caller
translation and high/raw-Q closure remain separately reviewable. The older
whole-file line audit and its open domains are preserved.

Run `make -f Makefile -f tools/cm_interbank_extended.mk
test-cm-interbank-extended` and `sanitize-cm-interbank-extended`.
`cm_interbank_extended_build.py` reproduces the native disk payloads and
`cm_interbank_extended_vice_test.py` checks their actual C64 states.
