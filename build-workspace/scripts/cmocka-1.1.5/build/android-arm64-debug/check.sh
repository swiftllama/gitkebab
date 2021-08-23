echo "--- Checking Cmocka 1.1.5 build (Linux/Debug) ---"

SENTINEL="./build/cmocka-1.1.5/linux/debug/lib/libcmocka-static.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


