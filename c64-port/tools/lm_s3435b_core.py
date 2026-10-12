# SPDX-License-Identifier: MIT
"""Native sources of the lm-s3435b milestone (shared by its tools)."""
SRC=['src/lm_s3435b_routines.c','src/lm_s3435_routines.c','src/agc_transfer.c','src/lm_interp_orders.c','src/lm_interp_steps.c','src/matrix_native.c','src/guidance_product.c','src/guidance_norm.c','src/agc_image_ops.c','src/agc_hardware_divide.c','src/fixed_data.c','src/agc_word.c','src/agc_multiword.c']
CORE=SRC+['tests/lm_s3435b_cases.c']
CASES=['tests/lm_s3435b_cases.c']
