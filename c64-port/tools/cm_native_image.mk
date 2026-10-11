CM_IMAGE_CORE = src/agc_word.c src/cm_executive.c src/cm_dispatch.c src/cm_native_dispatch.c src/cm_restart.c src/cm_restart_native.c src/cm_native_image.c
CM_IMAGE_TEST = tests/cm_restart_cases.c tests/cm_restart_native_cases.c tests/cm_native_image_check.c
CM_IMAGE_HEADERS = include/cm_native_image.h include/cm_restart_native.h include/cm_restart.h include/cm_executive.h include/cm_waitlist.h include/cm_native_dispatch.h include/cm_dispatch.h include/cm_phase.h include/cm_enema.h include/overlay_runtime.h tests/cm_restart_cases.h tests/cm_restart_native_cases.h tests/cm_restart_native_vectors.h tests/cm_native_image_check.h tests/cm_restart_addresses.h
all: build/test-cm-native-image
build/test-cm-native-image: $(CM_IMAGE_CORE) $(CM_IMAGE_TEST) tests/test_cm_native_image.c $(CM_IMAGE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IMAGE_CORE) $(CM_IMAGE_TEST) tests/test_cm_native_image.c -o $@
.PHONY: test-cm-native-image sanitize-cm-native-image c64-cm-native-image
test: test-cm-native-image
test-cm-native-image: build/test-cm-native-image
	./build/test-cm-native-image
sanitize: sanitize-cm-native-image
sanitize-cm-native-image: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_IMAGE_CORE) $(CM_IMAGE_TEST) tests/test_cm_native_image.c -o build/test-cm-native-image-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-native-image-sanitize
c64-cm-native-image: build/cm-native-image.prg
build/cm-native-image.prg: $(CM_IMAGE_CORE) $(CM_IMAGE_TEST) platform/c64/cm_native_image_diagnostic.c $(CM_IMAGE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_IMAGE_NO_FULL -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-native-image.map $(CM_IMAGE_CORE) $(CM_IMAGE_TEST) platform/c64/cm_native_image_diagnostic.c -o $@
