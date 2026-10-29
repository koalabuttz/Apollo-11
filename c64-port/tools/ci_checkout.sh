#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Lean checkout for CI and fresh cloud sessions: every file except legacy
# evidence blobs over 1 MB, plus the commit graph (no old trees or blobs).
# Other large tracked files (generated vector headers) are fetched normally.
#
# Bootstrap from an empty directory (CI does exactly this):
#   git init -q . && git remote add origin https://github.com/OWNER/REPO
#   git fetch -q --filter=blob:limit=1m --depth=1 origin REF
#   git show FETCH_HEAD:c64-port/tools/ci_checkout.sh | bash -s FETCH_HEAD
#
# Afterwards build with SKIP_EVIDENCE_RESTORE=1, and fetch new-style release
# evidence with tools/evidence.py fetch. Older blobs are fetched on demand.
set -euo pipefail
export LC_ALL=C
commit=$(git rev-parse "${1:-FETCH_HEAD}^{commit}")
git config remote.origin.promisor true
git config remote.origin.partialclonefilter blob:limit=1m
git config core.sparseCheckout true
git config core.sparseCheckoutCone false
missing=$(mktemp)
git rev-list --objects --no-walk --missing=print "$commit" | sed -n 's/^?//p' | sort > "$missing"
{
  echo '/*'
  git ls-tree -r --format='%(objectname) %(path)' "$commit" \
    | sort | join - "$missing" | cut -d' ' -f2- | grep '^c64-port/evidence/' | sed 's|^|!/|'
} > "$(git rev-parse --git-path info/sparse-checkout)"
echo "skipping legacy evidence blobs over 1 MB"
rm -f "$missing"
git checkout -q --detach "$commit"
# Commit graph only, so merge-base and diffs against the port branch work.
git fetch -q --filter=tree:0 --unshallow origin \
  '+refs/heads/main:refs/remotes/origin/main' 2>/dev/null \
  || git fetch -q --filter=tree:0 origin '+refs/heads/main:refs/remotes/origin/main'
echo "checked out $(git log -1 --format='%h %s' "$commit")"
