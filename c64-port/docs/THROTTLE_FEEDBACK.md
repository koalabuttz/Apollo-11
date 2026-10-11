# Native force feedback and clamps

The native service follows LOC03433 through LOC03507 before EXIT. It performs DELVROD vector work, cross product/addition/magnitude, interval scaling, FWEIGHT/MASS/SCALEFAC and the minimum, maximum and normal force branches, retaining MPAC, AFC, LASTTPIP and VAC aliases. It stops before the basic THROTTLE service bridge.

All16,384 complete124-word states agree with unchanged original assembly (SHA256 `a21344afc9e88cbe1adeec50b2c0b7645458b45b963ac0712d583bdff4d26d57`). Branch counts are10,083 minimum,3,544 maximum and2,757 normal. Strict frozen-state tests, invalid MODE/VAC guards and entire-corpus sanitizers pass.

All64 complete frozen states pass on the C64, covering39/13/12 minimum/maximum/normal branches, mailbox `a500004000`. PRG42,742bytes ends$afef with no additional zero page; hardware/software stack extents30/328bytes. The target proof deliberately bounds fixture count to fit RAM; it retains all branch classes. Application integration and deadline certification remain separate.
