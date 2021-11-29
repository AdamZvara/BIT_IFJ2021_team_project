CC = gcc
CFLAGS = -std=c99 -g -Wall -Wextra

TESTS_DIR = tests/

#files for scanner
SCANNER = src/scanner.c src/scanner.h src/str.c src/str.h src/error.h
SCANNER_T = $(TESTS_DIR)scanner-helper.c
PARSER = src/*.c src/*.h

.PHONY: doc test run

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
	@$(CC) $(CFLAGS) $^ -o src/parser

#generate doxygen documentation
doc: Doxyfile
	doxygen
	mv warning_doxygen.txt doc/

pdf: doc/dokumentace.tex
	pdflatex -output-directory doc/ $<
	pdflatex -output-directory doc/ $<

run: parser
	@./src/parser < prog.tl > test.code
	@./interpret/ic21int test.code

clean:
	$(RM) doc/{*.log,*.aux,*.pdf}
