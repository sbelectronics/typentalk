all: typetalk

typetalk: ruleset.cpp ruleset.h typetalk.cpp convert.cpp main.cpp Makefile
	g++ ruleset.cpp typetalk.cpp convert.cpp main.cpp -g -o typetalk

ruleset.cpp: typetalk.asm convert_rules.py
	python ./convert_rules.py > ruleset.cpp

ruleset.h: typetalk.asm convert_rules.py
	python ./convert_rules.py --headers > ruleset.h

test: ruleset.cpp ruleset.h typetalk.cpp convert.cpp test.cpp testcases.inc  Makefile
	g++ ruleset.cpp typetalk.cpp convert.cpp test.cpp -g -o typetalk-test
	./typetalk-test

testcases.inc: translate_translations.py translations.txt
	python ./translate_translations.py < translations.txt > testcases.inc

clean:
	rm ruleset.cpp ruleset.h
