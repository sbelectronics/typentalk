import sys

phonemes = [
	"EH3",  # 00
	"EH2",  # 01
	"EH1",  # 02
	"PA0",  # 03
	"DT",   # 04
	"A2",   # 05
	"A1",   # 06
	"ZH",   # 07
	"AH2",  # 08
	"I3",   # 09
	"I2",   # 0A
	"I1",   # 0B
	"M",    # 0C
	"N",    # 0D
	"B",    # 0E
	"V",    # 0F
	"CH",   # 10
	"SH",   # 11
	"Z",    # 12
	"AW1",  # 13
	"NG",   # 14
	"AH1",  # 15
	"OO1",  # 16
	"OO",   # 17
	"L",    # 18
	"K",    # 19
	"J",    # 1A
	"H",    # 1B
	"G",    # 1C
	"F",    # 1D
	"D",    # 1E
	"S",    # 1F
	"A",    # 20
	"AY",   # 21
	"Y1",   # 22
	"UH3",  # 23
	"AH",   # 24
	"P",    # 25
	"O",    # 26
	"I",    # 27
	"U",    # 28
	"Y",    # 29
	"T",    # 2A
	"R",    # 2B
	"E",    # 2C
	"W",    # 2D
	"AE",   # 2E
	"AE1",  # 2F
	"AW2",  # 30
    "UH2",  # 31
	"UH1",  # 32
	"UH",   # 33
	"O2",   # 34
	"O1",   # 35
	"IU",   # 36
	"U1",   # 37
	"THV",  # 38
	"TH",   # 39
	"ER",   # 3A
	"EH",   # 3B
	"E1",   # 3C
	"AW",
	"PA1",
	"STOP"
];

print "void execute_test_cases() {"

for line in sys.stdin.readlines():
    line = line.strip()
    if " " not in line:
        continue
    (english, encoded_phonemes) = line.split(" ",1)
    english = english.strip()
    if encoded_phonemes.startswith("C"):
        encoded_phonemes = encoded_phonemes[1:]
    encoded_phonemes = encoded_phonemes.strip()
    decoded_phonemes = [phonemes[ord(x)&0x3F] for x in encoded_phonemes]
    print '  test("%s", "%s");' % (english, " ".join(decoded_phonemes))

print "}"

