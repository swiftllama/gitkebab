echo "--- Checking Libiconv 1.16 build (Linux/Debug) ---"

SENTINEL="./build/libiconv-1.16/linux/debug/lib/libiconv.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


