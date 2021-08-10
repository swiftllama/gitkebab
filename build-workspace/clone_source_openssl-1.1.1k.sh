set -e
echo "--- Cloning Openssl 1.1.1k Source ---"

set -x
rm -rf source/openssl-1.1.1k
mkdir -p source
git clone --branch OpenSSL_1_1_1k --depth 1 git@github.com:openssl/openssl.git source/openssl-1.1.1k

set +x
echo "--- DONE ---"
