echo "--- Checking Libiconv 1.16 Source ---"

SENTINEL="./source/libiconv-1.16/src/iconv.c"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ] 
then
    echo "Source code seems to exist"
    exit 0
fi

echo "Source code does not exist"
exit 1


