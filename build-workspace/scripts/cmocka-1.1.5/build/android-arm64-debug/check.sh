echo "--- Checking Cmocka 1.1.5 build (android/arm64/Debug) ---"

SENTINEL="./build/cmocka-1.1.5/android/arm64-debug/lib/libcmocka-static.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


