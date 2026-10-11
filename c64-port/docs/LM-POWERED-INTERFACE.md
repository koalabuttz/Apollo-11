# Luminary Eight permanent interfaces

Selectors0..7 implement CDUTRIG, CD*TR*G, CDU*SMNB, TRG*SMNB, CDU*NBSM, TRG*NBSM, *SMNB*, and *NBSM*. Explicit EXIT, basic call, source axis, INTPRET reentry, VLOAD/pendingRVQ and fixed RVQ stages preserve all canonical words and bank controls. All ordinary saved FCADR banks are accepted. Basic internal returns retain ownership, and VLOAD returns EDOP0161: an Executive handoff there would be an invented source check.

The fixture overlays byte-original LM POWERED, INTERPRETER, RTB_OP_CODES and SINGLE_PRECISION sources with zero edits. It preserves original declaration/data rope `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`. The containing file SHA256 is `379505fe1429c8b0d6ef20d85bad9d8fb991083fd7202dc89c46e584f0ba92b1`.

All 868,352 complete 2048-word-plus-IRQ checkpoints from 65,536 inputs match original assembly, strict native and whole ASAN/UBSAN execution. Every unowned physical image word is compared. Decoded SHA256 is `729b6924bba78120a4e6b0d4e4fddee58cd71e8355bc5c2f0f359adfa882f623`. One lossless XZ object represents the three identical logical streams. Source-only frozen fixtures reconstruct all expected target images; expected flight values never enter the native numerical routines.

8 actual C64 PRGs pass 256 complete frozen states. Maximum loaded end is $63c8; measured hardware/software stack maxima are 35/342 bytes, with compiler-reserved zero-page0. These are standalone native diagnostics. Resident16KB overlay allocation, application Executive composition and sensor/actuator integration remain separate acceptance work.

Reproduce after `python tools/lm_powered_prepare_reference.py` and the original hardware-DV oracle prerequisites with `make -f tools/lm_powered_interface.mk test sanitize check-host`. `python tools/freeze_lm_powered_interface.py` reconstructs the frozen fixtures. C64/VICE targets accept SELECTOR0..7 (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. Native code follows explicitly compiled source operations; it does not decode AGC opcodes or use host floating point. External control retains producer exclusion and canonical scratch ownership across internal stages.
