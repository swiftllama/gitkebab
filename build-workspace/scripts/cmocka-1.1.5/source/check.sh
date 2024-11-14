echo "--- Checking Cmocka 1.1.5 Source ---"

SENTINEL="./source/cmocka-1.1.5/src/cmocka.c"
echo "Working directory: ${PWD}"
echo "Sentinel: ${SENTINEL}"

if [ -f $SENTINEL ] 
then
    echo "Source code seems to exist"
    exit 0
fi

echo "Source code does not exist"
exit 1


