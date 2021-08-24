set -e
echo "--- Cloning Openssl 1.1.1k Source ---"

set -x
rm -rf source/openssl-1.1.1k
mkdir -p source
git clone --branch OpenSSL_1_1_1k --depth 1 git@github.com:openssl/openssl.git source/openssl-1.1.1k

echo "Applying patch"
cd source/openssl-1.1.1k
git apply ../../patches/openssl-1.1.1k/pr-13694-android-ndk-22-build-fix.diff

set +x
echo "--- DONE ---"
