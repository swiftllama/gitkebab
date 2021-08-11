echo "--- Checking Cmocka 1.1.5 build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/cmocka-1.1.5/linux/debug/lib/libcmocka-static.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


