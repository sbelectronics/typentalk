TTLIBSRC = ../library/ruleset.cpp ../library/typetalk.cpp ../library/convert.cpp ../library/convert_stdio.cpp ../library/convert_stdio_string.cpp ../library/typetalk_stdio.cpp ../library/phoneme.cpp

all: typetalk

typetalk: ${TTLIBSRC} main.cpp rawmode.cpp Makefile
	g++ ${TTLIBSRC} main.cpp rawmode.cpp  -g -o typetalk

singleword: ${TTLIBSRC} singleword.cpp Makefile
	g++ ${TTLIBSRC} singleword.cpp  -g -o singleword

test: test1 test2

test1: ${TTLIBSRC} test-convert.cpp testcases.inc  Makefile
	g++ ${TTLIBSRC} test-convert.cpp -g -o test-convert
	./test-convert

test2: ${TTLIBSRC} test-typetalk.cpp  Makefile
	g++ ${TTLIBSRC} test-typetalk.cpp -g -o test-typetalk
	./test-typetalk

testcases.inc: translate_translations.py translations.txt
	python ./translate_translations.py < translations.txt > testcases.inc

