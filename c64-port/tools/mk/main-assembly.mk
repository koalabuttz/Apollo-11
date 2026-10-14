# SPDX-License-Identifier: MIT
# Milestone main-assembly (lane A): MAIN.agc of both missions, the include
# manifest (src/main_manifest.c, tools/main_assembly.py), checked against the
# assembled ropes on the host and, from the REU, on the C64.
MAN_SRC = src/main_manifest.c tests/main_manifest_check.c
MAN_HDR = include/main_manifest.h include/rope_image.h
MAN_ROPES = build/fixed-data/Luminary099/MAIN.agc.bin build/fixed-data/Comanche055/MAIN.agc.bin
.PHONY: test-main-assembly sanitize-main-assembly c64-main-assembly vice-main-assembly main-assembly-differential
all: build/test-main-assembly
test: test-main-assembly
sanitize: sanitize-main-assembly
build/test-main-assembly: $(MAN_SRC) src/rope_image_host.c tests/test_main_manifest.c $(MAN_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MAN_SRC) src/rope_image_host.c tests/test_main_manifest.c -o $@
test-main-assembly: build/test-main-assembly $(MAN_ROPES)
	./build/test-main-assembly $(MAN_ROPES)
sanitize-main-assembly: $(MAN_ROPES) | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(MAN_SRC) src/rope_image_host.c tests/test_main_manifest.c -o build/test-main-assembly-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-main-assembly-sanitize $(MAN_ROPES)
# Host-only: the manifest regenerated from yaYUL's listings, and the yaAGC oracle.
main-assembly-differential:
	python3 tools/main_assembly.py --check --proof
c64-main-assembly: build/main-assembly.prg
build/main-assembly.prg: $(MAN_SRC) src/overlay_reu.c platform/c64/overlay_reu_platform.c platform/c64/rope_image_reu.c platform/c64/main_manifest_diagnostic.c $(MAN_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/main-assembly.map $(MAN_SRC) src/overlay_reu.c platform/c64/overlay_reu_platform.c platform/c64/rope_image_reu.c platform/c64/main_manifest_diagnostic.c -o $@
vice-main-assembly: build/main-assembly.prg build/self-check.reu
	python3 tools/vice_run.py build/main-assembly.prg --reu build/self-check.reu --cycles 8000000000 --timeout 2400
