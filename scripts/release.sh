#!/usr/bin/env bash
# Make a release: build the optimised ROM, create a .bps patch against your own
# Pokémon Emerald dump, verify it, tag the version and publish it (decision D017).
# The base ROM never leaves your machine; only the patch is published.
#
# Usage: ./scripts/release.sh X.Y.Z [--base path/to/emerald.gba] [--no-publish]
#   --base        your Emerald dump (default: $ASCENSION_BASE_ROM or .local/base/emerald.gba)
#   --no-publish  build and verify the patch only; no tag, no GitHub release
source "$(dirname "$0")/lib/common.sh"
version="${1:-}"; shift || true
base="${ASCENSION_BASE_ROM:-$LOCAL_DIR/base/emerald.gba}"; publish=1
while [ $# -gt 0 ]; do
  case "$1" in
    --base) base="$2"; shift 2 ;;
    --no-publish) publish=0; shift ;;
    *) die "Unknown option: $1" ;;
  esac
done
[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "Give a version like 0.1.0 (got '${version}')."
tag="v$version"
cd "$ROOT"

# 1. Preconditions
[ -f "$base" ] || die "Base ROM not found at $base. Put your own Emerald dump there or pass --base."
expected="$(awk '{print $1}' rom.sha1)"
actual="$(shasum -a 1 "$base" 2>/dev/null | awk '{print $1}' || sha1sum "$base" | awk '{print $1}')"
[ "$actual" = "$expected" ] || die "Base ROM SHA-1 is $actual; expected $expected (Pokémon Emerald, USA/Europe)."
ok "Base ROM verified (Emerald, SHA-1 $expected)"
if [ "$publish" = 1 ]; then
  [ "$(git rev-parse --abbrev-ref HEAD)" = "main" ] || die "Releases are made from main. Run: git checkout main && git pull"
  [ -z "$(git status --porcelain)" ] || die "Uncommitted changes. Commit or stash them first."
  git fetch -q origin main && [ "$(git rev-parse HEAD)" = "$(git rev-parse origin/main)" ] || die "Local main differs from GitHub. Run: git pull"
  ! git rev-parse -q --verify "refs/tags/$tag" >/dev/null || die "Tag $tag already exists."
fi
commit="$(git rev-parse --short HEAD)"

# 2. Build the release ROM
"$ROOT/scripts/build.sh" --release
mkdir -p "$ROOT/dist"
rom="$ROOT/dist/pokemon-ascension-$tag.gba"          # stays local (dist/ is git-ignored)
patch="$ROOT/dist/pokemon-ascension-$tag.bps"
cp "$ROM_COPY" "$rom"

# 3. Create and verify the patch
info "Creating patch (about 20 seconds)"
python3 -I "$ROOT/tools/ascension/bps.py" create "$base" "$rom" "$patch" \
  --metadata "{\"name\":\"Pokémon Ascension\",\"version\":\"$version\",\"commit\":\"$commit\"}"
python3 -I "$ROOT/tools/ascension/bps.py" verify "$base" "$rom" "$patch"
ok "Patch: dist/pokemon-ascension-$tag.bps ($(du -h "$patch" | awk '{print $1}'))"
info "Patched ROM SHA-1: $(shasum -a 1 "$rom" 2>/dev/null | awk '{print $1}' || sha1sum "$rom" | awk '{print $1}')"

[ "$publish" = 1 ] || { ok "Done (not published)."; exit 0; }

# 4. Tag and publish
notes="$ROOT/dist/release-notes-$tag.md"
{
  echo "Pokémon Ascension $version (commit $commit)."
  echo
  echo "Apply \`pokemon-ascension-$tag.bps\` to your own Pokémon Emerald (USA/Europe) ROM, SHA-1 \`$expected\`, with Rom Patcher JS: https://www.marcrobledo.com/RomPatcher.js/ . See the README for full steps."
  echo
  awk '/^## /{n++} n==1' CHANGELOG.md
} > "$notes"
ask_yes "Tag $tag at $commit and publish the patch on GitHub?" || { ok "Stopped before publishing. Patch is in dist/."; exit 0; }
git tag -a "$tag" -m "Pokémon Ascension $version"
git push origin "$tag"
if have gh && gh auth status >/dev/null 2>&1; then
  gh release create "$tag" "$patch" --title "Pokémon Ascension $version" --notes-file "$notes"
  ok "Published: $(gh release view "$tag" --json url -q .url)"
else
  warn "GitHub CLI (gh) not set up, so the release page was not created."
  info "Open https://github.com/amber-falcon-games/pokemon-ascension/releases/new?tag=$tag"
  info "Attach dist/pokemon-ascension-$tag.bps and paste dist/release-notes-$tag.md as the description."
fi
