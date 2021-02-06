import sys

emitHeaders = ("--headers" in sys.argv)

def emit(ruleSet, buf):
    buf.append(0xFF)
    if emitHeaders:
        print ""
        print "#define %sLen %d" % (ruleSet, len(buf))
        print "extern unsigned char %s[%sLen];" % (ruleSet, ruleSet)
        return

    print ""
    print "unsigned char %s[%sLen] = {" % (ruleSet, ruleSet)
    print "   ",
    count = 0
    for v in buf:
        count += 1
        if (count == len(buf)):
            print "0x%02X" % v
            print "};"
            return
        print "0x%02X," % v,
        if (count % 16)==0:
            print ""
            print "   ",

if emitHeaders:
    print "#ifndef __ruleset_h__"
    print "#define __ruleset_h__"
else:
    print '#include "ruleset.h"'

lines=open("typetalk.asm").readlines()
lines = [x.strip() for x in lines]
ruleSet = None
buf = []
for line in lines:
    if line.startswith("ruleSet"):
        if ruleSet is not None:
            emit(ruleSet, buf)
            buf=[]
        ruleSet = line.rstrip(":")
    if line.startswith("txt2grTable"):
        emit(ruleSet, buf)
        break
    if ("db" in line):
        line = line.split(";")[0].strip()  # remove any comments
        stuff = line.split("db", 1)[1]
        stuff = stuff.strip()
        for part in stuff.split(","):
            part = part.strip()
            buf.append(int(part.lstrip("$"), 16))

if emitHeaders:
    print "#endif"