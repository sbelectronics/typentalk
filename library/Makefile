all: ruleset.cpp ruleset.h

ruleset.cpp: ../asm/typetalk.asm convert_rules.py
	python ./convert_rules.py ../asm/typetalk.asm > ruleset.cpp

ruleset.h: ../asm/typetalk.asm convert_rules.py
	python ./convert_rules.py ../asm/typetalk.asm --headers > ruleset.h

clean:
	rm ruleset.cpp ruleset.h
