echo "--- Checking Cmocka 1.1.5 Source ---"

echo "Working directory: ${PWD}"
set -x
if [ -f "./source/cmocka-1.1.5/src/cmocka.c" ] 
then
    echo "Source code seems to exist"
    exit 0
fi

set +x
echo "Source code does not exist"
exit 1


