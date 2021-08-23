echo "--- Checking Libiconv 1.16 Source ---"

echo "Working directory: ${PWD}"
set -x
if [ -f "./source/libiconv-1.16/src/iconv.c" ] 
then
    echo "Source code seems to exist"
    exit 0
fi

set +x
echo "Source code does not exist"
exit 1


