# Native Comanche combined Sun and Moon positions

The native LSPOS composition computes the Sun, stores its vector in VAC+2..7,
restores the original time, evaluates the ninth-degree Moon polynomial and returns
the Moon vector in MPAC to either actual source caller: Q022557/LOC02557 or
Q030231/LOC02231. It preserves real bank and interrupt controls, signed-zero
arithmetic, TIX scratch and all2048 canonical words.

65,536 uninterrupted original/strict native/full ASAN/UBSAN 2053-word states match
SHA1f97e8255b85c24af27ea9686a96f6e1ca52d12a6df15a20c2872c6b8851bfb3.
The source oracle executes LSPOS through solar and lunar bodies without replacing
any intermediate state. Two stock PAL C64 diagnostic cohorts check64 complete
original RESA-to-return states. The separately qualified five solar phases supply
the earlier computation. Incoming target states are captured from the actual
continuous original run and are losslessly frozen, rather than supplied by native
code. Explicit compiled stage returns retain only canonical persistent state.

The ten RESA/REM words overlap nine previously qualified LUNPOS polynomial words;
only the one RESA store is newly covered in containing-file word counts. Four
unsupported whole LSPOS caller frames and two null arguments are rejected without
mutating canonical state or external metadata. The public continuation requires
its exact original RESA frame and coherent bank/image/VAC/IO contracts.

This covers local combined-position computation. Mission caller bodies, original
restart responsibilities and complete application runtime remain separate.
