CC = gcc
CFLAGS = -std=c99 -g -Wall -Wextra 

TESTS_DIR = tests/

SCANNER = scanner.c scanner.h str.c str.h error.h
SCANNER_T = $(TESTS_DIR)scanner-helper.c

scanner-test: $(SCANNER_T) $(SCANNER) 
	$(CC) $(CFLAGS) $^ -o $(TESTS_DIR)scanner-helper

test: scanner-test
	cd $(TESTS_DIR); ./runtests.sh
