phonemes = []

for l in open("phonemelist.txt").readlines():
    l = l.strip()
    l = l.split(",", 1)[0]
    l = l.strip('"')
    phonemes.append(l)

for phoneme in phonemes:
    print "const char phon_%s[] PHONEMEMEM = \"%s\";" % (phoneme, phoneme)

print ""

phonemes = ["    phon_%s," % x for x in phonemes]

phonemes_with_comments = []
index = 0
for phoneme in phonemes:
    phonemes_with_comments.append("%-13s // %02X" % (phoneme, index))
    index+=1

phonemes_with_comments[-1] = phonemes_with_comments[-1].replace(",","")

print "const char *const phoneme_table[64] PHONEMEMEM = {"
print "\n".join(phonemes_with_comments)
print "};"

