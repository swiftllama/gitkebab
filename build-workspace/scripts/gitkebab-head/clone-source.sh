set -e
echo "--- Copying latest GitKebab HEAD Source ---"

set -x
rm -rf source/gitkebab-head
mkdir -p source/gitkebab-head
cp -PR ../src source/gitkebab-head/
cp ../CMakeLists.txt source/gitkebab-head/

set +x
echo "--- DONE ---"
