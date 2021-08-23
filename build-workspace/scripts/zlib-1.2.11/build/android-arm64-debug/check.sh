echo "--- Checking Zlib 1.2.11 build (Linux/Debug) ---"

SENTINEL="./build/zlib-1.2.11/android/arm64-debug/lib/libz.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


