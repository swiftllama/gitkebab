echo "--- Checking Openssl 1.1.1k build (android/arm64/Debug) ---"

SENTINEL="./build/openssl-1.1.1k/android/arm64-debug/lib/libssl.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


