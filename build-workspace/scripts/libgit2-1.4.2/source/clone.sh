set -e
echo "--- Cloning Libgit2 1.4.2 Source ---"

set -x
rm -rf source/libgit2-1.4.2
mkdir -p source
git clone --branch v1.4.2 --depth 1 git@github.com:libgit2/libgit2.git source/libgit2-1.4.2

echo "Applying patches"
cd source/libgit2-1.4.2
git apply < ../../patches/libgit2-1.4.2/fix-userauth-publikey-frommemory-detection.diff
git apply ../../patches/libgit2-1.4.2/fix-macos-eintr-handling.diff
git apply ../../patches/libgit2-1.4.2/fix-allow-injecting-libs-to-libgit2-tests-linking.diff
git apply ../../patches/libgit2-1.4.2/fix-android-avoid-getloadavg.diff
git apply ../../patches/libgit2-1.4.2/fix-std-c99.diff
git apply ../../patches/libgit2-1.4.2/disable-gitentropy-ios.diff

set +x
echo "--- DONE ---"

