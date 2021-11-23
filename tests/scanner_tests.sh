#!/bin/bash

SCANNER_INPUT_FILES=$(ls scanner-tests | grep .input)
SCANNER_TEST_DIR=scanner-tests/

for f in $SCANNER_INPUT_FILES; do
    TEST_NAME=$(echo $f | cut -d'.' -f1)
    INPUT=$SCANNER_TEST_DIR$f
    OUTPUT=$SCANNER_TEST_DIR$(echo $f | cut -d'.' -f1).output
    EXPECTED=$SCANNER_TEST_DIR$(echo $f | cut -d'.' -f1).expected
    DIFF=$SCANNER_TEST_DIR$(echo $f | cut -d'.' -f1).diff

    # Run tests
    ./scanner-helper < $SCANNER_TEST_DIR$f > $OUTPUT
    
    # Compare results if no error occurred
    if [ $? -eq 0 ]; then
        # Check output
        echo "Testing: $TEST_NAME"
        diff $OUTPUT $EXPECTED > $DIFF
        if [ $? -eq 0 ]; then
            echo "PASS"
        else
            echo "FAIL - CHECK .diff FILE"
        fi
    fi
done
