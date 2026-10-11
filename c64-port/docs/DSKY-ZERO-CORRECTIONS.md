# DSKY signed-zero and NOUT increment corrections

The immutable original `2BLANK` uses `BZMF` after exchanging the old relay
word. Both +0 and negative words skip `INCR NOUT`. The previous native blank
branches in generic Pinball and private DSPMMJB blanking treated +0 as positive.
Both now require positive **nonzero** old relay contents.

Original `INCR NOUT` sign-extends the 15-bit memory word, adds one in the
16-bit accumulator format, and overflow-corrects the stored word. Ordinary
15-bit one's-complement addition gives a different result at maximum positive
NOUT: original `037777 + 1` stores `000000`, rather than `040000`. All NOUT
increments in these two modules now use the existing native register addition
and overflow-correction primitives. The arithmetic is source composition;
no instruction decoder or emulator is linked into the native target.

Published earlier proof identities are preserved. Pinball's previous
`65097a2bedb65f7a5ebf81f42b101107b2e51ced4ae5aed81fd510e05bef0180`
and mode display's previous
`592125e52006dad64ee9a48725010dbef4d330cb7fc3c99456207db621a3096f`
remain byte-identical after the correction. Their constructors did not generate
clean +0 blank targets or positive overflow of NOUT. The prior proof therefore
established the generated corpus, and did not establish those claimed edges.
No historical proof result or immutable artifact is replaced by a new claim.

Fresh independent original-assembly corpora cover both signed zeros, positive
and negative relay words, completely clean +0 relay tables, NOUT +0/-0/+1,
maximum positive and maximum negative, and actual DSPIN/11DSPIN increments.
Each checkpoint compares all 2048 physical erasable words and SUPERBNK/IRQ.
The unchanged historical source is assembled in a private fixture and its entire
rope must equal the pinned fixture. Input construction links no native kernel
into the oracle.

| Corpus | Complete checkpoints | Snapshot SHA256 |
| --- | ---: | --- |
| Pinball zero/increment | 4096 | `ed0303c54b0c1dfc3cefe865042175538e150031190a9135ca288d036412faea` |
| DSPMMJB zero/increment | 1024 | `be2d6a15565c27dd66d0dd1726c36ad543f9e8b1e57ec24673c6d5d17e11268e` |

Both full sanitizer snapshot streams equal the original assembly byte for byte.
Strict host tests and sanitizer tests also check 64 frozen complete original
images. Native C64 diagnostics execute the same frozen images, including clean
+0, blank overflow, digit overflow, and sign overflow. Both VICE PAL mailboxes
are `a500004002`, with zero failures. Pinball PRG is 10657 bytes, hardware/software
stack sentinel extents 16/14; DSPMMJB PRG is 11456 bytes, extents 13/32. Both have
zero extra compiler zero-page allocations and fit below the reserved stack RAM.

Proof logs and snapshots have new `build/dsky-pinball-zero-*` and
`build/p66-mode-zero-*` names. Frozen deltas, test constructors and standalone
Makefile fragments use the corresponding underscore prefixes. Full native,
reference and sanitizer snapshots are also retained compressed with `.gz`.

The dependent resident P66 basic diagnostic was rebuilt after the correction.
Its prior artifacts were copied to `build/pre-zero-*`. Current strict and
sanitizer tests pass, and C64 again matches all eight complete original display
images, performs real ENDOFJOB dispatch and queued 100cs RODTASK admission, and
writes mode 66 to actual screen/color RAM through original DSPOUTSB stores.
Mailbox: `a500000802`; PRG: 42386 bytes; hardware/software stack extents: 22/139.
Current artifact SHA256:

- PRG: `71d674fcd1285d0c8065a487598875fe1d6bdd750f74dbfaa32d8b8fb7a41958`
- map: `9579fde42ca793cb8694fb14889c280104553f83af69e8a59c1567255b3e67da`
- mailbox: `a8c578965eca5befe617cca64aa80f005e9605bc2b8e9ad7ebd62f7b349fe9fc`

This correction does not claim full Pinball keyboard translation or a complete
LM/CM application. CHARIN key bodies and actual hardware input integration are
separate ongoing milestones.
