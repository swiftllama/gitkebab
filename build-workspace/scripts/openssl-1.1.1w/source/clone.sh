set -e
echo "--- Cloning Openssl 1.1.1w Source ---"

set -x
rm -rf source/openssl-1.1.1w
mkdir -p source
git clone --branch OpenSSL_1_1_1w --depth 1 git@github.com:openssl/openssl.git source/openssl-1.1.1w

set +x
echo "--- DONE ---"
