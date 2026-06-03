set -e
echo "--- Cloning Libgit2 1.8.4 Source ---"

set -x
rm -rf source/libgit2-1.8.4
mkdir -p source
git clone --branch v1.8.4 --depth 1 git@github.com:libgit2/libgit2.git source/libgit2-1.8.4

echo "Applying patches"
cd source/libgit2-1.8.4
# Obsolete in 1.8.4 (kept here as notes):
#   fix-macos-eintr-handling: upstream's connect_with_timeout supersedes it
#   fix-android-avoid-getloadavg: upstream now gates getloadavg via check_symbol_exists
#   fix-allow-injecting-libs-to-libgit2-tests-linking: not needed (we don't build tests)
git apply ../../patches/libgit2-1.8.4/fix-userauth-publikey-frommemory-detection.diff
git apply ../../patches/libgit2-1.8.4/fix-std-c99.diff
git apply ../../patches/libgit2-1.8.4/disable-gitentropy-ios.diff
cd ../../

set +x
echo "--- DONE ---"

