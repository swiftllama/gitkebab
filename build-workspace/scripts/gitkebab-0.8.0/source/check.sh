echo "--- Checking GitKebab 0.8.0 Source ---"

SENTINEL="./source/gitkebab-0.8.0/src/lib/gitkebab.h"

echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ] 
then
    echo "Source code seems to exist"
    exit 0
fi

echo "Source code does not exist"
exit 1


