echo "--- Checking Libiconv 1.16 build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/libiconv-1.16/linux/debug/lib/libiconv.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


