#!/bin/bash

PARSER_DIR="error normal simple"
for f in $PARSER_DIR; do
    PARSER_TEST_DIR=parser-tests/$f
    PARSER_INPUT_FILES=$(ls $PARSER_TEST_DIR | grep .input)

    for f in $PARSER_INPUT_FILES; do
        TEST_NAME=$(echo $f | cut -d'.' -f1)
        RETURN="${TEST_NAME: -1}"
        INPUT=$PARSER_TEST_DIR/$f
        OUTPUT=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).output
        RESULT=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).result
        EXPECTED=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).expected
        DIFF=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).diff
         
        echo "Testing: $TEST_NAME"
        if [ $PARSER_TEST_DIR == "parser-tests/error" ]; then
            # Run tests
            ../src/parser < $PARSER_TEST_DIR/$f >>/dev/null 2>&1
            if [ "$?" == $RETURN ]; then
                echo "PASS"
            else
                echo "FAIL"
            fi
        else
            # Run tests
            ../src/parser < $PARSER_TEST_DIR/$f > $OUTPUT
            ./../interpret/ic21int $OUTPUT > $RESULT
            diff $RESULT $EXPECTED > $DIFF
            if [ $? -eq 0 ]; then
                echo "PASS"
            else
                echo "FAIL - CHECK .diff FILE"
            fi
        fi

    done
done
