#ifndef __CONVERTER_H__
#define __CONVERTER_H__

#include <stdarg.h>
#include "tt_types.h"

#define STATE_INITIAL 0
#define STATE_SPACE 1
#define STATE_PHONEME 2
#define STATE_LETTER 3
#define STATE_NONLETTER 4
#define STATE_ESCAPE 5

#define MAX_GRAPHENEBUF 32

class Converter {
	public:
	    Converter(void);

		virtual void emitPhoneme(uint8_t phoneme);
        virtual void DebugPrintf(const char *format, ...);
        virtual void DebugRule(const char *txt, const uint8_t *myRulePtr, uint8_t *graPtr);

		void convert(uint8_t *, uint8_t);
		void convertString(char *word);

	protected:
		uint8_t grapheneBuf[MAX_GRAPHENEBUF+1];

		uint8_t *graPtr;
		uint8_t graCount;
		uint8_t graLen;
		uint8_t graPhoneme;
		uint8_t *graMatchPtr;
		const uint8_t *rulePtr;
		const uint8_t *rightRulePtr;
		uint8_t *rightGraphemePtr;
		uint8_t *nextRightGraphemePtr;
		const uint8_t *matchPtr;
		uint8_t context;

		bool debug;

	    uint8_t wordToGraphenes(char *word, uint8_t *graphenes);

        uint8_t readRuleByte(const uint8_t *p);
		const uint8_t *getRuleSet(uint8_t grapheme);

		void LocateRightContext(const uint8_t *);
		const uint8_t *skipRule(const uint8_t *myRulePtr);
		bool matchRuleSet();
		bool matchRule(uint8_t);
		
		bool matchSpecial(uint8_t);
		bool matchMultiVowel();
		bool matchFrontVowel();
		bool matchMultiCons();
		bool matchConsonant();
		bool matchNonLetterStuff();
		bool matchSuffix();
		bool matchSibilant();
		bool matchZeroCons();
		bool matchVoicedCons();
		bool matchLongUCons();
		bool matchNonLetterS();
		bool matchNumeric();
};

extern const char *phonemes[64];

#endif
