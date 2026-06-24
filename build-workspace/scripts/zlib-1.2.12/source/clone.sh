set -e
echo "--- Cloning Zlib 1.2.12 Source ---"

set -x
rm -rf source/zlib-1.2.12
mkdir -p source
git clone --branch v1.2.12 --depth 1 git@github.com:madler/zlib.git source/zlib-1.2.12

echo "Applying patches"
cd source/zlib-1.2.12
git apply ../../patches/zlib-1.2.12/fix-macos-fdopen-clash.diff
cd ../../

set +x
echo "--- DONE ---"
