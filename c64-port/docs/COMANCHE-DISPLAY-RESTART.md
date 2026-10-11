# Comanche display restart and canonical priority change

`cm_basic_priority_change` implements actual Comanche055 PRIOCHNG5210 through
seven-core EJSCAN, context exchange, ENDPRCHG and basic DTCB. It composes the
already source-qualified CM Executive view with the one canonical physical
flight image. Gathered Executive state is transient; commit precedes return.
Exact stores include NEWPRIO, source core/VAC state, scanner BUF/Q debris,
raw A/L, physical banks, SUPERBNK and IRQ. Comanche EXECBANK2626 produces
L02006 at DTCB; this differs from the Luminary value. Candidate priorities one
raw unit above the changed priority do not displace it, as in original CCS
comparison. Positive-zero candidate priorities take the untranslated CCSHOLE
path and are explicitly outside this service's contract. Selected interpretive
contexts remain an explicit unsupported boundary; no native callback is faked.

`cm_init_display` implements actual INITDSP10,3261, restoring saved EBANKTEM1071
and RESTREG0366 SUPERBNK, then making the actual priority change. If another
basic job wins, the original saved core retains INITDSP+6 location3267 and its
source bank. The selected job is returned to the caller for real dispatch.
`cm_init_display_return` translates that source3267 continuation through
CS THREE/AD TEMPFLSH0374/BANKJUMP4704 and returns the actual caller CADR pair.
If the display job remains selected, the initial entry performs this return
immediately. The API does not execute the selected user's program; the actual
application mapping owns that continuation. Valid saved priorities01000..037000
and TEMPFLSH4..077777 are supported, with ordinary source banks and core0 active.
Hardware normalization of raw L is preserved; Q remains a raw16-bit register.
Caller exclusion and one canonical image/device metadata record are required.

Independent host oracle executes the unchanged original Comanche Executive,
Display Interface and Inter-bank source. Original bytes and hashes are retained
in a private fixture; its complete assembled rope must equal the separately
assembled pinned rope. Only indentation normalization needed by modern yaYUL
is permitted and is counted in `build/cm-display-source-audit.json`. The oracle
links no native kernels. Three4096-case corpora cover PRIOCHNG, complete INITDSP
or actual preempting basic dispatch, and the display return continuation. They
include all32 physical FBANKs, all8 EBANKs/SUPER values, all seven cores, priority
ties and threshold edges, dormant/free(-0) cores, both signed zeros and raw
register overflow corners. Every checkpoint compares all2048 physical words
plus SUPERBNK/IRQ. All12288 checkpoints match; full sanitizer output is identical.
Snapshot SHA256:
`6f22da4fc6ac14cd25cba3047aea85b162d1d35a892a436166b46f42dbea57f1`.

Frozen64 full original states pass strict host, ASan/UBSan and native C64 tests.
The selected priority inputs cover all32 physical FBANKs, while the additional
INITDSP/return inputs exercise source bank restoration and raw register corners.
VICE PAL mailbox `a500004002`; PRG20261 bytes (loaded20259); measured hardware/
software stack23/730 bytes. Section/PRG bounds below C000 and zero additional
compiler zero-page storage are checked. Frozen fixtures preserve all2050 state
fields as lossless patches from independent original inputs. Evidence uses
`build/cm-display-*` and `build/cm_display-*`; full streams are retained compressed.
Standalone `tools/cm_display.mk` provides portable host-only `all`, `test` and
`sanitize`, plus explicit `c64-cm-display` and `differential-cm-display` targets.

This completes the bounded original display restart/priority source service,
not the full CM display application. Its selected continuation must be genuinely
registered in the resident CM loop. INITDSP does not itself render a noun or
consume a hardware keyboard event; those separate original Pinball/control
bodies require CM-specific proof and real device composition.
