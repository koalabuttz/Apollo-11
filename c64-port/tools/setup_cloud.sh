#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Install the pinned port toolchain into a cloud session or CI runner.
#
#   tools/setup_cloud.sh            # host tools, llvm-mos, VICE + ROMs, reference fixture
#   tools/setup_cloud.sh --no-fixture
#
# Everything lands under c64-port/deps/ (ignored by git) except the VICE apt
# package. Versions and hashes come from tools/versions.json. Afterwards:
#   source deps/env.sh   # exports MOS_CC, VICE, VICE_ROM_DIR, VICE_DATA_DIR
set -euo pipefail
cd "$(dirname "$0")/.."
FIXTURE=1
for arg in "$@"; do
  case "$arg" in
    --no-fixture) FIXTURE=0 ;;
    *) echo "unknown option: $arg" >&2; exit 2 ;;
  esac
done

pin() { python3 -c "import json,sys;d=json.load(open('tools/versions.json'));print(eval('d'+sys.argv[1]))" "$1"; }
sudo_() { if [ "$(id -u)" = 0 ]; then "$@"; else sudo "$@"; fi; }
fetch() { curl -fsSL --retry 4 --retry-delay 2 -o "$2" "$1"; }
check() { echo "$1  $2" | sha256sum -c --quiet - || { echo "SHA256 mismatch: $2" >&2; exit 1; }; }

mkdir -p deps

# Host packages. VICE comes from the distribution; its ROMs are fetched below.
need=()
for tool in make gcc clang python3 git curl xz; do command -v "$tool" >/dev/null || need+=("$tool"); done
command -v x64sc >/dev/null || need+=(vice)
if [ ${#need[@]} -gt 0 ]; then
  pkgs=(build-essential clang python3 git curl xz-utils)
  case " ${need[*]} " in *" vice "*) pkgs+=(vice) ;; esac
  # GitHub's runners point apt at azure.archive.ubuntu.com (through the
  # mirror list /etc/apt/apt-mirrors.txt), which has gone unresolvable for
  # hours at a time. If that name does not resolve, or apt fails, point apt
  # at the main Ubuntu archive for the rest of the job.
  apt_() { sudo_ env DEBIAN_FRONTEND=noninteractive apt-get -qq -o Acquire::Retries=2 -o Acquire::http::Timeout=30 "$@"; }
  apt_main_archive() {
    echo "apt: switching to archive.ubuntu.com" >&2
    for f in /etc/apt/apt-mirrors.txt /etc/apt/sources.list /etc/apt/sources.list.d/*.list /etc/apt/sources.list.d/*.sources; do
      if [ -f "$f" ]; then sudo_ sed -i 's#//azure\.archive\.ubuntu\.com#//archive.ubuntu.com#g' "$f"; fi
    done
  }
  if grep -qs azure.archive.ubuntu.com /etc/apt/apt-mirrors.txt /etc/apt/sources.list /etc/apt/sources.list.d/* &&
     ! getent hosts azure.archive.ubuntu.com >/dev/null; then apt_main_archive; fi
  apt_ok=0
  for try in 1 2; do
    if apt_ update && apt_ install -y --no-install-recommends "${pkgs[@]}"; then apt_ok=1; break; fi
    echo "apt failed (try $try)" >&2
    if [ "$try" = 1 ]; then apt_main_archive; fi
  done
  [ "$apt_ok" = 1 ] || { echo "apt install failed" >&2; exit 100; }
fi
echo "VICE $(x64sc -version 2>/dev/null | head -1 || true) (pinned $(pin "['vice']['version']"))"

# LLVM-MOS SDK.
if [ ! -x deps/llvm-mos/bin/mos-c64-clang ]; then
  tarball=deps/llvm-mos-linux.tar.xz
  [ -f "$tarball" ] || fetch "$(pin "['llvm_mos_sdk']['url']")" "$tarball"
  check "$(pin "['llvm_mos_sdk']['sha256']")" "$tarball"
  tar --no-same-owner -xJf "$tarball" -C deps
  rm -f "$tarball"
fi
deps/llvm-mos/bin/mos-c64-clang --version | head -1

# C64 ROMs from the pinned VICE source mirror, hash-checked.
rom_commit=$(pin "['vice']['rom_commit']")
mkdir -p deps/vice-data/C64
for rom in kernal-901227-03.bin basic-901226-01.bin chargen-901225-01.bin; do
  dst=deps/vice-data/C64/$rom
  [ -f "$dst" ] || fetch "https://raw.githubusercontent.com/VICE-Team/svn-mirror/$rom_commit/vice/data/C64/$rom" "$dst"
  check "$(pin "['vice']['rom_sha256']['$rom']")" "$dst"
done

# Reference fixtures: pinned Virtual AGC (yaYUL/yaAGC) plus the original
# sources, in dependency order. Each script verifies its own pinned hashes.
FIXTURES=(prepare_reference cm_prepare_reference cm_services_prepare_reference
  cm_waitlist_prepare_reference fixed_data_ropes
  cm_powered_prepare_reference lm_powered_prepare_reference
  cm_entry_tasks_prepare_reference cm_finish_prepare_reference cm_interbank_prepare_reference
  cm_numeric_prepare_reference cm_orbital_prepare_reference cm_restart_types_prepare_reference
  cm_trig_prepare_reference cm_rcs_matrix_prepare_reference lm_rcs_monitor_prepare_reference
  lm_single_trig_prepare_reference lm_sps_rcs_prepare_reference cm_mysubs_prepare_reference
  cm_mission_prepare_reference cm_clock_mission_prepare_reference rtb_angles_prepare_reference
  imu_pulse_prepare_reference)
if [ "$FIXTURE" = 1 ]; then
  mkdir -p deps/.done build
  for script in "${FIXTURES[@]}"; do
    if [ "$script" = fixed_data_ropes ]; then
      # Fresh host assemblies of both original ropes under build/fixed-data,
      # which the powered-flight fixtures copy. Not cached: build/ is scratch.
      python3 -c "import sys,pathlib;sys.path.insert(0,'tools');from fixed_data_generate import assemble
for m in ('Luminary099','Comanche055'): assemble(m, pathlib.Path('build/fixed-data'))" > deps/.done/fixed_data_ropes.log
      continue
    fi
    stamp=deps/.done/$script-$(sha256sum "tools/$script.py" | cut -c1-12)
    [ -f "$stamp" ] && continue
    echo "fixture: $script"
    python3 "tools/$script.py" > "deps/.done/$script.log" 2>&1 || { tail -20 "deps/.done/$script.log"; exit 1; }
    touch "$stamp"
  done
fi

cat > deps/env.sh <<EOF
export MOS_CC="$PWD/deps/llvm-mos/bin/mos-c64-clang"
export VICE="$(command -v x64sc)"
export VICE_ROM_DIR="$PWD/deps/vice-data/C64"
export VICE_DATA_DIR="$PWD/deps/vice-data"
# Names the legacy tools/*.mk recipes use for the same things.
export ROM_DIR="$PWD/deps/vice-data/C64"
export C64_ROM_DIR="$PWD/deps/vice-data/C64"
export C1541="$(command -v c1541)"
EOF
echo "READY: source deps/env.sh"
