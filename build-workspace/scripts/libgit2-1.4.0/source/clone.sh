set -e
echo "--- Cloning Libgit2 1.4.0 Source ---"

set -x
rm -rf source/libgit2-1.4.0
mkdir -p source
git clone --branch v1.4.0 --depth 1 git@github.com:libgit2/libgit2.git source/libgit2-1.4.0

echo "Applying patches"
cd source/libgit2-1.4.0
#patch -p0 < ../../patches/libgit2-1.4.0/fix-userauth-publikey-frommemory-detection.diff
git apply ../../patches/libgit2-1.4.0/fix-macos-eintr-handling.diff
#git apply ../../patches/libgit2-1.4.0/fix-allow-injecting-libs-to-libgi2_clar-linking.diff

set +x
echo "--- DONE ---"

