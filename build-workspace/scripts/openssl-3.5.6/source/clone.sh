set -e
echo "--- Cloning Openssl 3.5.6 Source ---"

set -x
rm -rf source/openssl-3.5.6
mkdir -p source
git clone --branch openssl-3.5.6 --depth 1 git@github.com:openssl/openssl.git source/openssl-3.5.6

set +x
echo "--- DONE ---"
