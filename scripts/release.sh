#!/usr/bin/env bash
# Make a release, step by step (decision D017).
#
#   ./scripts/release.sh                 guided: works out the version, walks you through
#                                        smoke testing, patching and publishing
#   ./scripts/release.sh 0.2.0           same, with the version given
#   ./scripts/release.sh 0.2.0 --base PATH --no-publish
#                                        build and verify the patch only (no prompts to publish)
#
# Your Emerald dump stays on this Mac (.local/base/emerald.gba, git-ignored).
# Only the .bps patch is published.
source "$(dirname "$0")/lib/common.sh"
cd "$ROOT"

version=""; base="${ASCENSION_BASE_ROM:-$LOCAL_DIR/base/emerald.gba}"; publish=1; guided=1
while [ $# -gt 0 ]; do
  case "$1" in
    --base) base="$2"; shift 2 ;;
    --no-publish) publish=0; shift ;;
    -h|--help) sed -n '2,12p' "$0"; exit 0 ;;
    [0-9]*) version="$1"; shift ;;
    *) die "Unknown option: $1" ;;
  esac
done
[ -t 0 ] || guided=0                     # no terminal (CI, pipes): never prompt
step() { printf '\n%s== Step %s: %s ==%s\n' "$C_OK" "$1" "$2" "$C_OFF"; }
pause() { [ "$guided" = 1 ] || return 0; read -r -p "$1 [press Enter] " _; }
confirm() {  # confirm "question": guided -> asks; otherwise yes only with ASSUME_YES=1
  [ "$guided" = 1 ] || { [ "${ASSUME_YES:-0}" = 1 ]; return; }
  local r; read -r -p "$1 [y/N] " r; [[ "$r" =~ ^[Yy]$ ]]
}
sha1() { shasum -a 1 "$1" 2>/dev/null | awk '{print $1}' || sha1sum "$1" | awk '{print $1}'; }

# ---------------------------------------------------------------- 1. Version
step 1 "Version"
last_tag="$(git describe --tags --abbrev=0 --match 'v[0-9]*' 2>/dev/null || true)"
if [ -z "$version" ]; then
  if [ -z "$last_tag" ]; then
    suggested="0.1.0"; info "No releases yet, so this is the first: 0.1.0 (Foundation)."
  else
    IFS=. read -r ma mi pa <<<"${last_tag#v}"
    suggested="$ma.$((mi + 1)).0"
    info "Last release: $last_tag. Milestone releases bump the middle number (e.g. $suggested); a fix-only release bumps the last ($ma.$mi.$((pa + 1)))."
  fi
  if [ "$guided" = 1 ]; then
    read -r -p "Version to release [$suggested]: " version
  fi
  version="${version:-$suggested}"
fi
[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "Versions look like 0.1.0 (got '$version')."
tag="v$version"
ok "Releasing $tag"

# ---------------------------------------------------------------- 2. Repository
step 2 "Repository"
if [ "$publish" = 1 ]; then
  if [ "$(git rev-parse --abbrev-ref HEAD)" != "main" ]; then
    confirm "You're on '$(git rev-parse --abbrev-ref HEAD)'. Switch to main?" || die "Releases are made from main."
    git checkout -q main
  fi
  [ -z "$(git status --porcelain)" ] || die "There are uncommitted changes. Commit or stash them, then run this again."
  git fetch -q origin main
  if [ "$(git rev-parse HEAD)" != "$(git rev-parse origin/main)" ]; then
    if git merge-base --is-ancestor HEAD origin/main; then
      confirm "Your main is behind GitHub. Pull the latest now?" || die "Run: git pull"
      git pull -q --ff-only origin main
    fi
    # Releases must be exactly what is on GitHub: stop on local-only commits.
    [ "$(git rev-parse HEAD)" = "$(git rev-parse origin/main)" ] \
      || die "Your main has commits that aren't on GitHub. Push them through a PR first, or reset to origin/main."
  fi
  ! git rev-parse -q --verify "refs/tags/$tag" >/dev/null || die "$tag already exists. Pick another version."
fi
commit="$(git rev-parse --short HEAD)"
ok "main @ $commit"

# ---------------------------------------------------------------- 3. Base ROM
step 3 "Your Pokémon Emerald dump"
expected="$(awk '{print $1}' rom.sha1)"
if [ ! -f "$base" ] && [ "$guided" = 1 ]; then
  info "Needed to make the patch. It stays on this Mac and is never uploaded."
  read -r -p "Path to your Emerald .gba (you can drag the file into this window): " given
  given="${given%\'}"; given="${given#\'}"; given="${given%\"}"; given="${given#\"}"; given="${given% }"
  # Terminal escapes dragged paths (e.g. Pokemon\ Emerald.gba). Undo backslash escapes as
  # plain text; nothing typed here is ever run as a command.
  [ -f "$given" ] || given="$(printf '%s' "$given" | sed 's/\\\(.\)/\1/g')"
  [ -f "$given" ] || die "No file at: $given"
  [ "$(sha1 "$given")" = "$expected" ] || die "That file's SHA-1 is $(sha1 "$given"); a clean Pokémon Emerald (USA/Europe) is $expected."
  mkdir -p "$(dirname "$base")" && cp "$given" "$base"
  info "Saved a copy to .local/base/emerald.gba for next time."
fi
[ -f "$base" ] || die "Base ROM not found at $base. Put your own Emerald dump there or pass --base."
[ "$(sha1 "$base")" = "$expected" ] || die "Base ROM SHA-1 is $(sha1 "$base"); expected $expected (Pokémon Emerald, USA/Europe)."
ok "Base ROM verified"

# ---------------------------------------------------------------- 4. Build and smoke test
step 4 "Build the release ROM and smoke test it"
"$ROOT/scripts/build.sh" --release
mkdir -p "$ROOT/dist"
rom="$ROOT/dist/pokemon-ascension-$tag.gba"      # local only (dist/ is git-ignored)
patch="$ROOT/dist/pokemon-ascension-$tag.bps"
cp "$ROM_COPY" "$rom"
if [ "$guided" = 1 ]; then
  if is_macos && { [ -d /Applications/mGBA.app ] || [ -d "$HOME/Applications/mGBA.app" ]; }; then
    open -a mGBA "$rom"; ok "Opened the release build in mGBA"
  else
    warn "Open dist/pokemon-ascension-$tag.gba in your emulator."
  fi
  cat <<EOF

  Smoke test (about 5 minutes):
    1. New Game, through to your first wild battle on Route 101 (win or run).
    2. Save in-game.
    3. Close mGBA, reopen dist/pokemon-ascension-$tag.gba, choose Continue: you are where you saved.
  Release builds have no debug menu. To check things deeper in the game (PC boxes,
  story flags), use a dev build: ./scripts/play.sh, then R+Start in the overworld.

EOF
  confirm "Did the smoke test pass?" || die "Stopped. Tell Claude what went wrong; nothing was tagged or published."
  ok "Smoke test passed"
fi

# ---------------------------------------------------------------- 5. Patch
step 5 "Create and verify the patch"
info "Creating patch (about 20 seconds)"
python3 -I "$ROOT/tools/ascension/bps.py" create "$base" "$rom" "$patch" \
  --metadata "{\"name\":\"Pokémon Ascension\",\"version\":\"$version\",\"commit\":\"$commit\"}"
python3 -I "$ROOT/tools/ascension/bps.py" verify "$base" "$rom" "$patch"
ok "Patch: dist/pokemon-ascension-$tag.bps ($(du -h "$patch" | awk '{print $1}')); patched ROM SHA-1 $(sha1 "$rom")"
if [ "$guided" = 1 ]; then
  cat <<EOF

  Check the patch the way players will use it:
    1. Rom Patcher JS opens in your browser. Choose your Emerald dump, then the patch
       (Finder opens on it), then Apply patch.
    2. Open the patched ROM in mGBA: it should reach the title screen.
EOF
  pause "Ready?"
  if is_macos; then open "https://www.marcrobledo.com/RomPatcher.js/"; open -R "$patch"; fi
  confirm "Did the patched ROM from Rom Patcher JS boot?" || die "Stopped. Tell Claude; nothing was tagged or published."
  ok "Patch checked in Rom Patcher JS"
fi

[ "$publish" = 1 ] || { ok "Done. Not published (--no-publish)."; exit 0; }

# ---------------------------------------------------------------- 6. Publish
step 6 "Tag and publish $tag"
notes="$ROOT/dist/release-notes-$tag.md"
{
  echo "Pokémon Ascension $version (commit $commit)."
  echo
  echo "Apply \`pokemon-ascension-$tag.bps\` to your own Pokémon Emerald (USA/Europe) ROM, SHA-1 \`$expected\`, with Rom Patcher JS: https://www.marcrobledo.com/RomPatcher.js/ . See the README for full steps."
  echo
  awk '/^## /{n++} n==1' CHANGELOG.md
} > "$notes"
confirm "Tag $tag at $commit and publish the patch on GitHub?" || { ok "Stopped before publishing. The patch is in dist/."; exit 0; }
git tag -a "$tag" -m "Pokémon Ascension $version"
info "Pushing the tag to GitHub"
git push -q origin "$tag"
ok "Tagged and pushed $tag"
if have gh && gh auth status >/dev/null 2>&1; then
  info "Uploading the $(du -h "$patch" | awk '{print $1}') patch to GitHub. This shows no progress bar and can take a few minutes on home broadband."
  gh release create "$tag" "$patch" --title "Pokémon Ascension $version" --notes-file "$notes"
  ok "Published: $(gh release view "$tag" --json url -q .url)"
else
  warn "GitHub CLI (gh) isn't set up, so finish on the website:"
  info "1. Open https://github.com/amber-falcon-games/pokemon-ascension/releases/new?tag=$tag"
  info "2. Attach dist/pokemon-ascension-$tag.bps"
  info "3. Paste dist/release-notes-$tag.md as the description, then Publish release."
  is_macos && { open "https://github.com/amber-falcon-games/pokemon-ascension/releases/new?tag=$tag"; open -R "$patch"; }
fi
printf '\n'; ok "Release $tag done. Tell Claude it's out so the dashboard and docs can be updated."
