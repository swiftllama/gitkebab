set -e
echo "--- Cloning Zlib 1.2.11 Source ---"

set -x
rm -rf source/zlib-1.2.11
mkdir -p source
git clone --branch v1.2.11 --depth 1 git@github.com:madler/zlib.git source/zlib-1.2.11

set +x
echo "--- DONE ---"
