# SPDX-License-Identifier: MIT
CM_ENEMA_IMAGE_CORE = src/cm_enema_image.c src/cm_enema.c src/cm_restart.c src/cm_waitlist.c src/cm_native_image.c src/agc_word.c tests/cm_enema_image_cases.c tests/cm_enema_cases.c tests/cm_restart_cases.c src/cm_executive.c
build/cm-enema-image-native: $(CM_ENEMA_IMAGE_CORE) platform/host/cm_enema_image_snapshot.c include/cm_enema_image.h | build
	$(CC) $(CPPFLAGS) -DCM_RESTART_INPUT_ONLY -Itools $(CFLAGS) $(CM_ENEMA_IMAGE_CORE) platform/host/cm_enema_image_snapshot.c -o $@
cm-enema-image-differential: build/cm-enema-image-native prepare-cm-original-rope
	python3 tools/cm_enema_image_differential.py
.PHONY: test-cm-enema-image sanitize-cm-enema-image
test: test-cm-enema-image
test-cm-enema-image: cm-enema-image-differential
sanitize: sanitize-cm-enema-image
sanitize-cm-enema-image: | build
	$(CC) $(CPPFLAGS) -DCM_RESTART_INPUT_ONLY -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ENEMA_IMAGE_CORE) platform/host/cm_enema_image_snapshot.c -o build/cm-enema-image-sanitize
	python3 tools/cm_enema_image_sanitize.py
