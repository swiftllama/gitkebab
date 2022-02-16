echo "--- Checking GitKebab HEAD Source ---"

SENTINEL="./source/gitkebab-1.0.0/src/lib/gitkebab.h"

echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ] 
then
    echo "Source code seems to exist"
    exit 0
fi

echo "Source code does not exist"
exit 1


