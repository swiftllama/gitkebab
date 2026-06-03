set -e
echo "--- Cloning Libssh2 1.11.1 Source ---"

set -x
rm -rf source/libssh2-nextgen-1.11.1
mkdir -p source
git clone --branch libssh2-1.11.1 --depth 1 git@github.com:libssh2/libssh2.git source/libssh2-nextgen-1.11.1

echo "Applying patches"
cd source/libssh2-nextgen-1.11.1
git apply ../../patches/libssh2-1.11.1/retry-on-eintr.diff
git apply ../../patches/libssh2-1.11.1/include-errno-on-ios.diff
cd ../../

set +x
echo "--- DONE ---"
