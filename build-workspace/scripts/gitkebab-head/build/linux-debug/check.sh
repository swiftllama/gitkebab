echo "--- Checking GitKebab HEAD build (Linux/Debug) ---"

SENTINEL="./build/gitkebab-head/linux/debug/lib/libgitkebab.so"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


