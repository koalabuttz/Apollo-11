# SPDX-License-Identifier: MIT
"""Native sources of the lm-p32 milestone (shared by its tools)."""
SRC=['src/lm_p32_routines.c','src/agc_transfer.c','src/lm_interp_orders.c','src/lm_interp_steps.c','src/matrix_native.c','src/guidance_product.c','src/guidance_norm.c','src/inverse_trig.c','src/rtb_agreement.c','src/agc_image_ops.c','src/agc_hardware_divide.c','src/fixed_data.c','src/agc_word.c','src/agc_multiword.c']
CORE=SRC+['tests/lm_p32_cases.c']
CASES=['tests/lm_p32_cases.c']
