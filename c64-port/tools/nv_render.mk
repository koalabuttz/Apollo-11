# SPDX-License-Identifier: MIT
NVRE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
NVRE_CORE = src/p66_nv_render.c src/dsky_decimal.c src/dsky_scale.c src/guidance_product.c src/p66_nv_rows.c src/p66_nv_words.c src/p66_nv_entry.c src/dsky_two_digit.c src/dsky_pinball.c src/agc_word.c src/agc_multiword.c tests/throttle_bridge_cases.c tests/nv_entry_cases.c tests/nv_words_cases.c tests/nv_rows_cases.c tests/nv_render_cases.c
NVRE_TEST = tests/test_nv_render.c tests/nv_render_check.c $(NVRE_CORE)
.PHONY: nv_render-host nv_render-sanitize nv_render-c64 nv_render-differential
nv_render-host:
	$(CC) -O2 -Wconversion -pedantic $(NVRE_FLAGS) $(NVRE_TEST) -o build/test-nv-render
	build/test-nv-render
nv_render-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVRE_FLAGS) $(NVRE_TEST) -o build/test-nv-render-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-nv-render-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVRE_FLAGS) tools/nv_render_full_snapshot.c $(NVRE_CORE) -o build/nv-render-full-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/nv-render-full-native-sanitize > build/nv-render-full-sanitize.snapshots
	cmp build/nv-render-full-sanitize.snapshots build/nv-render-full-reference.snapshots
nv_render-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DNV_RENDER_SELECTED=$(if $(PHASE),$(PHASE),0) $(NVRE_FLAGS) tools/nv_render_diagnostic.c tests/nv_render_check.c $(NVRE_CORE) -Wl,-Map,build/nv_render$(if $(PHASE),$(PHASE),0).map -o build/nv_render$(if $(PHASE),$(PHASE),0).prg
nv_render-differential:
	python3 tools/nv_render_differential.py
	python3 tools/nv_render_full_differential.py
all: build/nv-render-full-native build/test-nv-render
build/nv-render-full-native: tools/nv_render_full_snapshot.c $(NVRE_CORE) | build
	$(CC) -O2 $(NVRE_FLAGS) tools/nv_render_full_snapshot.c $(NVRE_CORE) -o $@
build/test-nv-render: $(NVRE_TEST) tests/nv_render_vectors.h | build
	$(CC) -O2 -Wconversion -pedantic $(NVRE_FLAGS) $(NVRE_TEST) -o $@
test: nv_render-host
sanitize: nv_render-sanitize
