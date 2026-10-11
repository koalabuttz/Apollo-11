# Native horizontal velocity

Eight explicit source phases implement VHORCOMP from VLOAD24 through surface DELVS correction, PDVL R, UNIT/VXSC HDOT, VSL1/BVSU, ABVAL, STOREVHORIZ and the original numeric EXIT before DISPEXIT atPC03531. Complete raw-bank effects are preserved: ABVAL ends with FB0/BB7; the bank4 VHORIZ store leaves FB064000, EB02000 and BB064004 before DANZIG restores BANKSET064007.

All8,192 inputs match eight complete120-word checkpoints,65,536 comparisons, including767 UNIT overflow continuations. Snapshot SHA256 `8fe57d51b7b58b6cb99cdb585c7791eaced695dc41aa169d41da823c721bd037`. All-state ASan/UBSan output matches; invalid MODE, VAC and pointer domains reject unchanged.

All256 complete frozen C64 states pass four64-state images, mailbox `a50000400003f07828`. Text26,500bytes, maximum image/state end$ad80, additional zero page0, hardware/software stacks34/265bytes. Inputs independently match original stage entry; this proof does not claim the full flight prefix runs on the target. Sampled complete horizontal timings106,524–205,452PALcycles (about108–209ms) are finite observations. Display/service completion and full resident application deadlines remain separate requirements.
