set -e
echo "--- Cloning Libssh2 1.11.1 Source ---"

set -x
rm -rf source/libssh2-1.11.1-legacy-openssl
mkdir -p source
git clone --branch libssh2-1.11.1 --depth 1 git@github.com:libssh2/libssh2.git source/libssh2-1.11.1-legacy-openssl

echo "Applying patches"
cd source/libssh2-1.11.1-legacy-openssl
git apply ../../patches/libssh2-1.11.1-legacy-openssl/retry-on-eintr.diff
git apply ../../patches/libssh2-1.11.1-legacy-openssl/include-errno-on-ios.diff
cd ../../

set +x
echo "--- DONE ---"
