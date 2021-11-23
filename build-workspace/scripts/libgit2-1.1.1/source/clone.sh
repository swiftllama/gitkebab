set -e
echo "--- Cloning Libgit2 1.1.1 Source ---"

set -x
rm -rf source/libgit2-1.1.1
mkdir -p source
git clone --branch v1.1.1 --depth 1 git@github.com:libgit2/libgit2.git source/libgit2-1.1.1

echo "Applying patches"
cd source/libgit2-1.1.1
patch -p0 < ../../patches/libgit2-1.1.1/fix-userauth-publikey-frommemory-detection.diff
git apply ../../patches/libgit2-1.1.1/fix-macos-eintr-handling.diff
git apply ../../patches/libgit2-1.1.1/fix-allow-injecting-libs-to-libgi2_clar-linking.diff

set +x
echo "--- DONE ---"

