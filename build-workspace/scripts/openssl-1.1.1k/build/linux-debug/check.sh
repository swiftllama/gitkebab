echo "--- Checking Openssl 1.1.1k build (Linux/Debug) ---"

SENTINEL="./build/openssl-1.1.1k/linux/debug/lib/libssl.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


