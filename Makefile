CC = gcc
CFLAGS = -std=c99 -g -Wall -Wextra

TESTS_DIR = tests/

#files for scanner
SCANNER = scanner.c scanner.h str.c str.h error.h
SCANNER_T = $(TESTS_DIR)scanner-helper.c
PARSER = *.c *.h

.PHONY: doc test run pack compiler

compiler: $(PARSER)
	@$(CC) $(CFLAGS) $^ -o ifj2021_compiler

#run all tests
test: scanner-test parser-test
#	@cd $(TESTS_DIR); ./scanner_tests.sh
	@cd $(TESTS_DIR); ./parser_tests.sh

#scanner tests
scanner-test: $(SCANNER_T) $(SCANNER)
	@$(CC) $(CFLAGS) $^ -o $(TESTS_DIR)scanner-helper

#parser tests
parser-test: $(PARSER)
	@$(CC) $(CFLAGS) $^ -o $(TESTS_DIR)parser

#parser
parser: $(PARSER)
	@$(CC) $(CFLAGS) $^ -o parser

#generate doxygen documentation
doc: Doxyfile
	doxygen
	mv warning_doxygen.txt doc/

pdf: doc/dokumentace.tex
	pdflatex -output-directory doc/ $<
	pdflatex -output-directory doc/ $<

run: parser
	@./parser < prog.tl > test.code
	@./interpret/ic21int test.code

pack:
	zip xeichl01.zip *.c *.h Makefile dokumentace.pdf rozdeleni

clean:
	$(RM) doc/{*.log,*.aux,*.pdf}
