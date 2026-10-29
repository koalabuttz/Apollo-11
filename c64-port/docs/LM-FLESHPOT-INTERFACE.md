# Luminary CALCSMSC and FLESHPOT-1 return composition

The original bank10 interpreted CALCSMSC source executes EXIT, actual BANKCALL and FLESHPOT-1 default ECADR selection, constructs XNB, executes actual SWRETURN using BUF2, reenters INTPRET and performs all-bank fixed RVQ. The canonical source state exposes eight explicit stage checkpoints, including the body and pending EXIT stages. Saved fixed basic addresses02000..07777 and coherent bank words are the source return domain. FLESHPOT-1 is also a standalone default-entry API. No numerical stage return invents a scheduling check.

The fixture overlays byte-original LM POWERED, INTERPRETER, RTB_OP_CODES and SINGLE_PRECISION sources with zero edits. It preserves original declaration/data rope `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`. The containing file SHA256 is `379505fe1429c8b0d6ef20d85bad9d8fb991083fd7202dc89c46e584f0ba92b1`.

All 65,536 complete 2048-word-plus-IRQ checkpoints from 8,192 inputs match original assembly, strict native and whole ASAN/UBSAN execution. Every unowned physical image word is compared. Decoded SHA256 is `4daa190db35fe7737ae2c0d9abc957ccc69e7911984155bc9fa0788fee1e3696`. One lossless XZ object represents the three identical logical streams. Source-only frozen fixtures reconstruct all expected target images; expected flight values never enter the native numerical routines.

8 actual C64 PRGs pass 256 complete frozen states. Maximum loaded end is $4250; measured hardware/software stack maxima are 25/176 bytes, with compiler-reserved zero-page0. These are standalone native diagnostics. Resident16KB overlay allocation, application Executive composition and sensor/actuator integration remain separate acceptance work.

Reproduce after `python tools/lm_powered_prepare_reference.py` and the original hardware-DV oracle prerequisites with `make -f tools/lm_fleshpot_interface.mk test sanitize check-host`. `python tools/freeze_lm_fleshpot_interface.py` reconstructs the frozen fixtures. C64/VICE targets accept SELECTOR0..7 (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. Native code follows explicitly compiled source operations; it does not decode AGC opcodes or use host floating point. External control retains producer exclusion and canonical scratch ownership across internal stages.

The standalone bank-return API is a FLESHPOT_FINISH-output continuation: its A/L registers must already be normalized as that original body produces. Arbitrary raw-register overflow inputs at this return boundary are not part of the qualification.
