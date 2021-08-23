echo "--- Checking Pcre 8.45 build (Linux/Debug) ---"

SENTINEL="./build/pcre-8.45/linux/debug/lib/libpcre.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


