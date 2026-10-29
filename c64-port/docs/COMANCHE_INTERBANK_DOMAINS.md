# Updated Comanche interbank source domains

The original whole-line inventory remains immutable in
`evidence/cm-interbank-coverage-audit.json`. Its historical span-only reader
limitations are closed by later source-qualified passive original-word and
actual-caller packets. The new `cm-interbank-domain-audit.json` records that
change explicitly, retaining the original audit identity.

Actual DSPFMEM and COMADRS entries through SUPDACAL/read/source return match
163,840 complete original checkpoints from 81,920 caller cases. All 40,960
physical addresses include 36,864 assembled original words and 4,096 hardware
zero holes. Stock C64 passive storage independently compares all 81,920 word
returns individually for both incoming I states, and actual caller target
paths separately compare complete source images. The original rope identity
is `6e3150ae...`; its six passive payloads require 74,802 loaded bytes.
Original words serve only as data. They never supply executable dispatch or
an instruction decoder, and preservation earns no algorithm coverage.

The broader native interbank packet also matches 901,120 original complete
states with raw 16-bit register corners. Its 16,384 invalid negative-overflow
BANKJUMP inputs preserve state and return unsupported: the original TS FBANK
skip bypasses MASK and makes INDEX Q change TCF10000 into CCS1777,
which can fall through the neighboring MAKECADR outside a valid compiled CADR
transfer.

Complete-file qualification remains false. Raw/high Q and saved-return
caller closure, unretained state cross products, arbitrary interrupt timing
and full algorithms after DSPFMEM/COMADRS return still require their own
proofs. The retained actual reader continuation is distinct from the entire
display or ROPECHK algorithm. Run `python3 tools/cm_interbank_domain_audit.py`
to reproduce the updated provenance inventory.
