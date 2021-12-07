#!/bin/bash

RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'
ORANGE='\033[0;33m'
PARSER_DIR="runtime"
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
        if [ $PARSER_TEST_DIR == "parser-tests/runtime" ]; then
            # Run tests
            ../src/parser < $PARSER_TEST_DIR/$f > $OUTPUT
            ./../interpret/ic21int $OUTPUT
            if [ "$?" != $RETURN ]; then
                echo -e "${RED}FAIL${NC}"
            fi
        fi

    done
done
