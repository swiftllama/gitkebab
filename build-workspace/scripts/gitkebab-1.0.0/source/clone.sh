set -e
echo "--- Copying latest GitKebab HEAD Source ---"

set -x
rm -rf source/gitkebab-1.0.0
mkdir -p source
git clone --branch v1.0.0-released --depth 1 git@github.com:projectjudo/gitkebab.git source/gitkebab-1.0.0
 
set +x
echo "--- DONE ---"
