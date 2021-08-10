echo "--- Checking Zlib 1.2.11 build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/zlib-1.2.11/linux/debug/lib/libz.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


