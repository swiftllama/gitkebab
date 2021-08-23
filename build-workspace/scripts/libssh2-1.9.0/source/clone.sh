set -e
echo "--- Cloning Libssh2 1.9.0 Source ---"

set -x
rm -rf source/libssh2-1.9.0
mkdir -p source
git clone --branch libssh2-1.9.0 --depth 1 git@github.com:libssh2/libssh2.git source/libssh2-1.9.0

set +x
echo "--- DONE ---"
