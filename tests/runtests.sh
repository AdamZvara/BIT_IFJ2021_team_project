#!/bin/bash

SCANNER_INPUT="scanner-input"
SCANNER_OUTPUT="scanner-helper.out"
SCANNER_EXPECTED="scanner-helper.expected"

./scanner-helper < $SCANNER_INPUT > $SCANNER_OUTPUT
diff $SCANNER_OUTPUT $SCANNER_EXPECTED
if [ $? -eq 0 ]; then
    echo "PASS"
else
    echo "FAIL"
fi
