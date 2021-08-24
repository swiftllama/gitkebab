echo "--- Checking GitKebab HEAD build (Android/arm64/Debug) ---"

SENTINEL="./build/gitkebab-head/android/arm64-debug/lib/libgitkebab.so"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


