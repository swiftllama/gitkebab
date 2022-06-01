set -e
echo "--- Cloning Libssh2 1.10.0 Source ---"

set -x
rm -rf source/libssh2-1.10.0
mkdir -p source
git clone --branch libssh2-1.10.0 --depth 1 git@github.com:libssh2/libssh2.git source/libssh2-1.10.0

echo "Applying patch"
cd source/libssh2-1.10.0
git apply ../../patches/libssh2-1.10.0/retry-on-eintr.diff
git apply ../../patches/libssh2-1.10.0/include-errno-on-ios.diff

set +x
echo "--- DONE ---"
