echo "--- Checking GitKebab HEAD Source ---"

echo "Working directory: ${PWD}"
set -x
if [ -f "./source/gitkebab-head/src/lib/gitkebab.h" ] 
then
    echo "Source code seems to exist"
    exit 0
fi

set +x
echo "Source code does not exist"
exit 1


