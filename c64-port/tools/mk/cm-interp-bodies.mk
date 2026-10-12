# SPDX-License-Identifier: MIT
# Milestone cm-interp-bodies (lane A): the compiled interpretive order bodies
# (src/lm_interp_orders.c, lm_interp_steps.c) built for Comanche055 with its
# fixed addresses (src/cm_interp_*.c, include/cm_interp.h), against the
# original Comanche055 program in yaAGC.
CM_INTERP_SRC = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_multiword.c src/agc_word.c
CM_INTERP_RUN = -include src/cm_interp_names.h tests/interp_body_run.c
CM_INTERP_HDR = include/cm_interp.h include/interp_mission.h src/cm_interp_names.h src/lm_interp_orders.c src/lm_interp_steps.c tests/interp_body_run.h tests/interp_vector_check.h tests/cm_interp_vectors.h
.PHONY: test-cm-interp-bodies sanitize-cm-interp-bodies c64-cm-interp-bodies vice-cm-interp-bodies cm-interp-bodies-differential
all: build/test-cm-interp-bodies
test: test-cm-interp-bodies
sanitize: sanitize-cm-interp-bodies
build/test-cm-interp-bodies: $(CM_INTERP_SRC) tests/interp_body_run.c tests/test_cm_interp_bodies.c $(CM_INTERP_HDR) | build
	$(CC) $(CPPFLAGS) -Isrc $(CFLAGS) $(CM_INTERP_SRC) $(CM_INTERP_RUN) tests/test_cm_interp_bodies.c -o $@
test-cm-interp-bodies: build/test-cm-interp-bodies
	python3 tools/cm_interp_generate.py --check
	./build/test-cm-interp-bodies
sanitize-cm-interp-bodies: | build
	$(CC) $(CPPFLAGS) -Isrc -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_INTERP_SRC) $(CM_INTERP_RUN) tests/test_cm_interp_bodies.c -o build/test-cm-interp-bodies-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-interp-bodies-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both mission listings).
cm-interp-bodies-differential: | build
	python3 tools/lmi_body_differential.py --mission cm --cases 3000 --proof build/cm-interp-bodies-proof.json --vectors build/cm_interp_vectors.h --vector-cases 4
	cmp build/cm_interp_vectors.h tests/cm_interp_vectors.h
# The C64 diagnostic is three programs, bodies 0-19, 20-39 and 40-70, so each
# fits below the $c000 mailbox.
CM_INTERP_PART_1 = -DINTERP_BODY_LO=0u -DINTERP_BODY_HI=20u
CM_INTERP_PART_2 = -DINTERP_BODY_LO=20u -DINTERP_BODY_HI=40u
CM_INTERP_PART_3 = -DINTERP_BODY_LO=40u -DINTERP_BODY_HI=71u
c64-cm-interp-bodies: build/cm-interp-bodies-1.prg build/cm-interp-bodies-2.prg build/cm-interp-bodies-3.prg
build/cm-interp-bodies-%.prg: $(CM_INTERP_SRC) tests/interp_body_run.c platform/c64/cm_interp_bodies_diagnostic.c $(CM_INTERP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -Isrc -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -Wl,--gc-sections $(CM_INTERP_PART_$*) -Wl,-Map,build/cm-interp-bodies-$*.map $(CM_INTERP_SRC) $(CM_INTERP_RUN) platform/c64/cm_interp_bodies_diagnostic.c -o $@
vice-cm-interp-bodies: build/cm-interp-bodies-1.prg build/cm-interp-bodies-2.prg build/cm-interp-bodies-3.prg
	python3 tools/vice_run.py build/cm-interp-bodies-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/cm-interp-bodies-2.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/cm-interp-bodies-3.prg --cycles 3000000000 --timeout 1200
