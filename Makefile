CC = gcc
CFLAGS = -std=c99 -g -Wall -Wextra 

TESTS_DIR = tests/

#files for scanner
SCANNER = src/scanner.c src/scanner.h src/str.c src/str.h src/error.h
SCANNER_T = $(TESTS_DIR)scanner-helper.c
PARSER = src/scanner.c src/scanner.h src/str.c src/str.h src/error.h src/parser.c src/parser.h src/symtable.c src/symtable.h

.PHONY: doc test

#run all tests
test: scanner-test
	cd $(TESTS_DIR); ./runtests.sh

#scanner tests
scanner-test: $(SCANNER_T) $(SCANNER) 
	$(CC) $(CFLAGS) $^ -o $(TESTS_DIR)scanner-helper

#parser
parser: $(PARSER)
	$(CC) $(CFLAGS) $^ -o src/parser

#generate doxygen documentation
doc: Doxyfile
	doxygen
	mv warning_doxygen.txt doc/

