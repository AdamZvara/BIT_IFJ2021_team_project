CC = gcc
CFLAGS = -std=c99 -g -Wall -Wextra 

TESTS_DIR = tests/

#files for scanner
SCANNER = scanner.c scanner.h str.c str.h error.h
SCANNER_T = $(TESTS_DIR)scanner-helper.c

.PHONY: doc test

#generate doxygen documentation
doc: Doxyfile
	doxygen
	mv warning_doxygen.txt doc/

#scanner tests
scanner-test: $(SCANNER_T) $(SCANNER) 
	$(CC) $(CFLAGS) $^ -o $(TESTS_DIR)scanner-helper

#run all tests
test: scanner-test
	cd $(TESTS_DIR); ./runtests.sh
