TTLIBSRC = ../library/ruleset.cpp ../library/typetalk.cpp ../library/convert.cpp ../library/convert_stdio.cpp ../library/convert_stdio_string.cpp ../library/typetalk_stdio.cpp

all: typetalk

typetalk: ${TTLIBSRC} main.cpp rawmode.cpp Makefile
	g++ ${TTLIBSRC} main.cpp rawmode.cpp  -g -o typetalk

test: ${TTLIBSRC} test.cpp testcases.inc  Makefile
	g++ ${TTLIBSRC} test.cpp -g -o typetalk-test
	./typetalk-test

testcases.inc: translate_translations.py translations.txt
	python ./translate_translations.py < translations.txt > testcases.inc

