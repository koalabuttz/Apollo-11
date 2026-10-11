# Port genuine P66 velocity scheduling boundaries

Three native velocity groups stop at source LOC03366,03370 and03373 with EDOP zero. All13936 complete118-word source checkpoints match SHA977279d21f48ea76643d805554338b2d4fb8fb9b8da9f72a80952c9a2d6c912d; strict/sanitizer checks and96 actualC64 entry/exit states pass. LOC03371 retains pendingSTADR and cannot hand off. C64 images17922/17035/16086bytes use no additional zero page, at most20 hardware and47 software stack bytes. The owning canonical driver supplies scheduling/control state and producer exclusion.

These bounded services do not certify complete program coverage. The original
Luminary and Comanche source trees remain unchanged. Public headers state each
entry domain and producer exclusions; evidence includes original-source states,
host and sanitizer results, target PRGs, maps, mailboxes and stack measurements.

Reproduce the source comparison with `python3 tools/velocity_phased_differential.py deps/fixture`.
