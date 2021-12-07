#!/bin/bash

RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'
ORANGE='\033[0;33m'
PARSER_DIR="error failed_tests simple need_to_fix"
for f in $PARSER_DIR; do
    PARSER_TEST_DIR=parser-tests/$f
    PARSER_INPUT_FILES=$(ls $PARSER_TEST_DIR | grep .input)
	echo -e "${ORANGE}${f^^}:${NC}"
    for f in $PARSER_INPUT_FILES; do
        TEST_NAME=$(echo $f | cut -d'.' -f1)
        RETURN="${TEST_NAME: -1}"
        INPUT=$PARSER_TEST_DIR/$f
        OUTPUT=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).output
        RESULT=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).result
        EXPECTED=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).expected
        DIFF=$PARSER_TEST_DIR/$(echo $f | cut -d'.' -f1).diff

        echo -e "${BLUE}Testing:${NC} $TEST_NAME"
        if [ $PARSER_TEST_DIR == "parser-tests/error" ]; then
            # Run tests
            ../src/parser < $PARSER_TEST_DIR/$f >>/dev/null 2>&1
            if [ "$?" != $RETURN ]; then
                echo -e "${RED}FAIL${NC}"
            fi
        else
            # Run tests
            ../src/parser < $PARSER_TEST_DIR/$f > $OUTPUT
            ./../interpret/ic21int $OUTPUT > $RESULT 2>/dev/null
            diff $RESULT $EXPECTED > $DIFF
            if [ $? -ne 0 ]; then
                echo -e "${RED}FAIL${NC} - CHECK .diff FILE"
            fi
        fi


    done
done

bash runtime.sh
bash built_in.sh
