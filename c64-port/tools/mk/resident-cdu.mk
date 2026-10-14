# The flight resident's IMU CDU drive (src/resident_cdu.c rk_cdu_drive)
# against yaAGC's BurstOutput over one timeline of 10 ms periods
# (tests/resident_cdu_vectors.h, frozen by tools/resident_cdu_vectors.py).
RC_SRC = src/resident_cdu.c tests/resident_cdu_cases.c tests/test_resident_cdu.c
RC_HDR = include/resident_cdu.h tests/resident_cdu_cases.h tests/resident_cdu_vectors.h
.PHONY: test-resident-cdu sanitize-resident-cdu
all: build/test-resident-cdu
test: test-resident-cdu
sanitize: sanitize-resident-cdu
build/test-resident-cdu: $(RC_SRC) $(RC_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RC_SRC) -o $@
test-resident-cdu: build/test-resident-cdu
	./build/test-resident-cdu
sanitize-resident-cdu: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-sanitize-recover=all $(RC_SRC) -o build/test-resident-cdu-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-resident-cdu-sanitize
