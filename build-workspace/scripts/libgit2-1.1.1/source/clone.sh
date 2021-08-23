set -e
echo "--- Cloning Libgit2 1.1.1 Source ---"

set -x
rm -rf source/libgit2-1.1.1
mkdir -p source
git clone --branch v1.1.1 --depth 1 git@github.com:libgit2/libgit2.git source/libgit2-1.1.1

set +x
echo "--- DONE ---"
